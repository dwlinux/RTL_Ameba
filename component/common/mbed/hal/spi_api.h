/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_SPI_API_H
#define MBED_SPI_API_H

#include "device.h"

#if DEVICE_SPI

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_DMA_RX_EN           (1<<0)
#define SPI_DMA_TX_EN           (1<<1)

//#define SPI_SCLK_IDLE_LOW       0       // the SCLK is Low when SPI is inactive
//#define SPI_SCLK_IDLE_HIGH      2       // the SCLK is High when SPI is inactive
enum {
    SPI_SCLK_IDLE_LOW=0,        // the SCLK is Low when SPI is inactive
    SPI_SCLK_IDLE_HIGH=2        // the SCLK is High when SPI is inactive
};

//#define SPI_CS_TOGGLE_EVERY_FRAME        0       // the CS toggle every frame
//#define SPI_CS_TOGGLE_START_STOP         1       // the CS toggle at start and stop
enum {
    SPI_CS_TOGGLE_EVERY_FRAME=0,       // the CS toggle every frame
    SPI_CS_TOGGLE_START_STOP=1         // the CS toggle at start and stop
};

typedef enum {
    CS_0 = 0,
    CS_1 = 1,
    CS_2 = 2,
    CS_3 = 3,
    CS_4 = 4,
    CS_5 = 5,
    CS_6 = 6,
    CS_7 = 7
}ChipSelect;

typedef struct spi_s spi_t;

void spi_init         (spi_t *obj, PinName mosi, PinName miso, PinName sclk, PinName ssel);
void spi_free         (spi_t *obj);
void spi_format       (spi_t *obj, int bits, int mode, int slave);
void spi_frequency    (spi_t *obj, int hz);
int  spi_master_write (spi_t *obj, int value);
int  spi_slave_receive(spi_t *obj);
int  spi_slave_read   (spi_t *obj);
void spi_slave_write  (spi_t *obj, int value);
int  spi_busy         (spi_t *obj);

#ifdef __cplusplus
}
#endif

#endif

#endif
