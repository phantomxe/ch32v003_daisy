#include "ch32v003fun.h"
#include <stdio.h>

volatile uint16_t adc_buffer[1];

void Adc_Init() {
    RCC->CFGR0 &= ~(0x1F<<11);
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1;
    
    GPIOA->CFGLR &= ~(0xf<<(4*4));	// PC4
    
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
	
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = (2<<(5*0));
	
	ADC1->SAMPTR2 = (7<<(3*2));
	
	ADC1->CTLR2 |= ADC_ADON;
	
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	
	DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
	DMA1_Channel1->MADDR = (uint32_t)adc_buffer;
	DMA1_Channel1->CNTR  = 1;
	DMA1_Channel1->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralSRC;
	 
	DMA1_Channel1->CFGR |= DMA_CFGR1_EN; 
	ADC1->CTLR1 |= ADC_SCAN; 
	ADC1->CTLR2 |= ADC_CONT | ADC_DMA | ADC_EXTSEL; 
	ADC1->CTLR2 |= ADC_SWSTART;
}

int main()
{
	SystemInit(); 
	funGpioInitAll();
	Adc_Init();
	
	//printf("Hello world!!\n");
	 
	*DMDATA0 = 0;

	while(1) {
		//printf("ADC: %u\n", adc_buffer[0]);
		*DMDATA0 = adc_buffer[0];
		
		Delay_Ms(10);
	}
}