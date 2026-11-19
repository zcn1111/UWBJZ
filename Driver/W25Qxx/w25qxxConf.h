#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H

#include <stdbool.h>
//#include "bsp_spi.h"
#include "Drv_W25Qxx_QSPI.h"

#define _W25QXX_SPI             hqspi
#define _W25QXX_CS_GPIO         GPIOE
#define _W25QXX_CS_PIN          GPIO_PIN_11
#define _W25QXX_USE_FREERTOS    0
#define _W25QXX_DEBUG           0





#endif

