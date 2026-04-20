# Overview
This a simple non-blocking library for the BH1750 sensor, implemented for STM microcontrollers with HAL library.

# Usage
1. Remember to replace #include "stm32u5xx_hal.h" with appropriate HAL header for your MCU - current implementation is for STM U5 series.
2. You can see example DMA, interrupts, timers and I2C configuration in main.c or project files - I recommend using the same or similar configuration.