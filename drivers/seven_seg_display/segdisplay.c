#include "ch32v003fun.h"
#include <stdio.h>
#include "swio_daisy.h"

const uint8_t digitData[] = {
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111,  // 9
  0b01110111,  // A
  0b01111100,  // b
  0b00111001,  // C
  0b01011110,  // d
  0b01111001,  // E
  0b01110001,   // F
  0b00000000    // none  
};

volatile int state = 0;   
volatile uint8_t number[4] = {0, 1, 2, 3};
volatile uint8_t scantime = 0; 
volatile uint8_t signal = 0;

void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
    if(TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update; 
        
        if(number[0] > 15) {
            number[0] = 15;        
        }
        if(number[1] > 15) {
            number[1] = 15;        
        }
        if(number[2] > 15) {
            number[2] = 15;        
        }
        if(number[3] > 15) {
            number[3] = 15;        
        }

        if(scantime == 4) {
            scantime = 0;        
        }

        uint8_t p = 0;
        //uint8_t dot = 0;
        
        switch(scantime) {
            case 0:
                funDigitalWrite(PD5, 0); // com0
                funDigitalWrite(PA2, 1); // com1
                funDigitalWrite(PC0, 1); // com2
                funDigitalWrite(PC3, 1); // com3
                //dot = dot0;
                break;
            case 1:
                funDigitalWrite(PD5, 1);
                funDigitalWrite(PA2, 0);
                funDigitalWrite(PC0, 1);
                funDigitalWrite(PC3, 1);
                break;   
            case 2:
                funDigitalWrite(PD5, 1);
                funDigitalWrite(PA2, 1);
                funDigitalWrite(PC0, 0);
                funDigitalWrite(PC3, 1);
                //dot = dot2;
                break;
            case 3:
                funDigitalWrite(PD5, 1);
                funDigitalWrite(PA2, 1);
                funDigitalWrite(PC0, 1);
                funDigitalWrite(PC3, 0);
                //dot = dot2;
                break;
            default:
                break; 
        } 

        p = digitData[number[scantime]];

        funDigitalWrite(PD6, p & 0b00000001); //a
        funDigitalWrite(PA1, p & 0b00000010); //b
        funDigitalWrite(PC6, p & 0b00000100); //c
        funDigitalWrite(PC1, p & 0b00001000); //d
        funDigitalWrite(PD4, p & 0b00010000); //e
        funDigitalWrite(PD7, p & 0b00100000); //f
        funDigitalWrite(PC4, p & 0b01000000); //g
        //digitalWrite(0xc7, dot);  //dot

        scantime++;
        signal = 1;
    }
}

static void HandleCommand(uint32_t dmdword) { 
    if(((dmdword >> 20) & 0x0f) == 0x0a) { // a is check word.
        number[0] = (dmdword >> 12) & 0x0f;
        number[1] = (dmdword >> 8) & 0x0f;
        number[2] = (dmdword >> 4) & 0x0f;
        number[3] = dmdword & 0x0f;
    }
    
    *DMDATA0 = 0;
}

int main()
{
	SystemInit(); 
	funGpioInitAll();
	
	swio_init(); 
	
	*DMDATA0 = 0;
	
    funPinMode(PC0, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_PP);  
    funPinMode(PC3, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PC4, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PC6, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PC7, GPIO_CFGLR_OUT_10Mhz_PP);  
    funPinMode(PD4, GPIO_CFGLR_OUT_10Mhz_PP); 
    funPinMode(PD5, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PD6, GPIO_CFGLR_OUT_10Mhz_PP);
    funPinMode(PD7, GPIO_CFGLR_OUT_10Mhz_PP);  
    funPinMode(PA1, GPIO_CFGLR_OUT_10Mhz_PP); 
    funPinMode(PA2, GPIO_CFGLR_OUT_10Mhz_PP);  
    
    RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1;
    
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    TIM1->ATRLR = 7000-1; 
    TIM1->PSC = 10-1;
    TIM1->RPTCR = 0;
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

    NVIC_EnableIRQ(TIM1_UP_IRQn);

    TIM1->INTFR = ~TIM_FLAG_Update;
    TIM1->DMAINTENR |= TIM_IT_Update;
    TIM1->CTLR1 |= TIM_CEN;  
	
	//swio_send_header();
	
	while(1) {
	    uint32_t dmdword = *DMDATA0;
	    
	    if (signal == 1) {
	        signal = 0;
	        Delay_Us(150);  // display brightness
	        
	        funDigitalWrite(PD5, 1);
            funDigitalWrite(PA2, 1);
            funDigitalWrite(PC0, 1);
            funDigitalWrite(PC3, 1);
	    }
	    
        // display 0 write 1236
	    // sudo ./minichlink -s 0x04 0x00a41236 
        // display 1 write 2244
        // sudo ./minichlink -s 0x04 0x00a52244 
        if((dmdword & 0xf0000) == 0x40000) {
		    HandleCommand(dmdword);
	    } else if(dmdword != 0x00000000) {
	        uint32_t next_word = (dmdword - 0x00010000);
            swio_send_header();
	        swio_write_reg_32(0x04, next_word);
            *DMDATA0 = 0;
	    } 
	}
}