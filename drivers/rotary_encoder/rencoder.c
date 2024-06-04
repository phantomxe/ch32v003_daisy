#include "ch32v003fun.h"
#include <stdio.h>
#include "swio_daisy.h"

#define BUTTON_DEBOUNCE_INTERVAL Ticks_from_Ms(250)

volatile int a = 0, b = 0, prev_a = 1, prev_b = 1, count = 0;
volatile int button_pressed = 0;
volatile int flag = 0;
volatile uint32_t button_event_time = 0;

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) { 

    if (EXTI->INTFR & EXTI_Line1) { 
        b = funDigitalRead(PC2); 
		
		if(b != prev_b) {
		    if(b) {
		        if(!a) {
		            flag = 1;
		            count--;
		        }
		    }
		    prev_b = b;
		}
 
		EXTI->INTFR |= EXTI_Line1;
	}
	
	if (EXTI->INTFR & EXTI_Line2) { 
        a = funDigitalRead(PC1);
		
		if(a != prev_a) {
		    if(a) {
		        if(!b) {
		            flag = 1;
		            count++;
		        }
		    }
		    prev_a = a;
		}

		EXTI->INTFR |= EXTI_Line2;
	}
	
	if (EXTI->INTFR & EXTI_Line4) {   
        if( SysTick->CNT - button_event_time > BUTTON_DEBOUNCE_INTERVAL ) { 
            button_event_time = SysTick->CNT;
            button_pressed = 1;
        }
	
	    EXTI->INTFR |= EXTI_Line4;
	}
}

// custom function to interact with swio display
void ch32_display_write(int base, int i) { 
  int first_digit = i % 10;
  int sec_digit = (i / 10) % 10;
  int third_digit = (i / 100) % 10;
  int fourth_digit = (i / 1000) % 10;

  int result = base + (fourth_digit << 12) + (third_digit << 8) + (sec_digit << 4) + first_digit;
  
  swio_write_reg_32(0x04, result); 
}

int main()
{
	SystemInit(); 
	funGpioInitAll();
	
	swio_init();
	
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
 
	funPinMode(PC1, GPIO_CFGLR_IN_FLOAT);
	funPinMode(PC2, GPIO_CFGLR_IN_FLOAT); 
	funPinMode(PC4, GPIO_CFGLR_IN_FLOAT);
	
	asm volatile(
#if __GNUC__ > 10
		".option arch, +zicsr\n"
#endif
 		"addi t1, x0, 3\n"
		"csrrw x0, 0x804, t1\n"
		 : : :  "t1" );
		 
    AFIO->EXTICR = (AFIO_EXTICR_EXTI1_PC | AFIO_EXTICR_EXTI2_PC | AFIO_EXTICR_EXTI4_PC);
	EXTI->INTENR = (EXTI_INTENR_MR1 | EXTI_INTENR_MR2 | EXTI_INTENR_MR4);
	EXTI->FTENR = (EXTI_FTENR_TR1 | EXTI_FTENR_TR2 | EXTI_FTENR_TR4 );  
	EXTI->RTENR = (EXTI_RTENR_TR1 | EXTI_RTENR_TR2 ); 

	button_event_time = SysTick->CNT;

	NVIC_EnableIRQ( EXTI7_0_IRQn );
	
	swio_send_header(); //demo code
	
	while(1) {
	    if(flag) {
		    //printf("Counter: %d %lu\n", count, SysTick->CNT);
		    if(count >= 0 && count <= 9999) {
		        ch32_display_write(0x00a40000, count);  // write counter to swio display
		    }
		    flag = 0;
		}
		if(button_pressed) {
		    //printf("Button pressed, counter = 0\n");
		    button_pressed = 0;
		    count = 0;
		    ch32_display_write(0x00a40000, count); // clear display
		}
	}
}