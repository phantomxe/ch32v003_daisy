#ifndef __SWIO_DAISY_H
#define __SWIO_DAISY_H

#include "ch32v003fun.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define SWIO_OUT_PIN PC2

#define SWIO_DRV_ONE  funDigitalWrite(SWIO_OUT_PIN, 0); asm volatile( ".rept 12\nc.nop\n.endr" ); funDigitalWrite(SWIO_OUT_PIN, 1); asm volatile( ".rept 8\nc.nop\n.endr" );
#define SWIO_DRV_ZERO funDigitalWrite(SWIO_OUT_PIN, 0); asm volatile( ".rept 28\nc.nop\n.endr" ); funDigitalWrite(SWIO_OUT_PIN, 1);

void swio_init() {
    funPinMode(SWIO_OUT_PIN, GPIO_CFGLR_OUT_10Mhz_OD);
}

static inline uint8_t SWIO_RECV_BIT() {
    funDigitalWrite(SWIO_OUT_PIN, 0);  
    funDigitalWrite(SWIO_OUT_PIN, 1);
    asm volatile( ".rept 12\nc.nop\n.endr" );
    funPinMode(SWIO_OUT_PIN, GPIO_CFGLR_IN_FLOAT);
    asm volatile( ".rept 12\nc.nop\n.endr" );
    uint8_t output = funDigitalRead(SWIO_OUT_PIN);
    funPinMode(SWIO_OUT_PIN, GPIO_CFGLR_OUT_10Mhz_OD);
    return output;
}

void swio_write_reg_32(uint8_t addr, uint32_t val) { 

__disable_irq();
    SWIO_DRV_ONE;

    for (uint8_t i = 0; i < 7; i++) {
        if (addr & 0x40) {
            SWIO_DRV_ONE;
        } else {
            SWIO_DRV_ZERO;
        }

        addr <<= 1;
    }

    SWIO_DRV_ONE;

    for (uint8_t i = 0; i < 32; i++) {
        if (val & 0x80000000) {
            SWIO_DRV_ONE;
        } else {
            SWIO_DRV_ZERO;
        }

        val <<= 1;
    }
__enable_irq();

    Delay_Us(10);
}

uint32_t swio_read_reg_32(uint8_t addr) { 
    uint32_t x = 0;
    
__disable_irq();
    SWIO_DRV_ONE;

    for (uint8_t i = 0; i < 7; i++) {
        if (addr & 0x40) {
            SWIO_DRV_ONE;
        } else {
            SWIO_DRV_ZERO;
        }

        addr <<= 1;
    }

    SWIO_DRV_ZERO;

    for (uint8_t i = 0; i < 32; i++) {
        x <<= 1;

        if (SWIO_RECV_BIT())
            x |= 1;
    }

__enable_irq();

    Delay_Us(10);
    
    return x;
}

void swio_send_header() {
    swio_write_reg_32(0x7E, 0x5AA50400);
    swio_write_reg_32(0x7D, 0x5AA50400); 
    swio_write_reg_32(0x10, 0x00000000); // reset slave debug
    swio_write_reg_32(0x10, 0x00000001); // enable slave debug
}

#endif