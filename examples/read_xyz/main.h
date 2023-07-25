#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_manager.h"

#if CONFIG_IDF_TARGET_ESP32S3
// #    define SPI_HOST     SPI2_HOST
// #    define PIN_NUM_MISO 13
// #    define PIN_NUM_MOSI 11
// #    define PIN_NUM_CLK  12
// #    define PIN_NUM_CS   2
#   define PIN_NUM_SDA  CONFIG_I2C_MANAGER_0_SDA
#   define PIN_NUM_SCL  CONFIG_I2C_MANAGER_0_SCL
#   define I2C_PORT     I2C_NUM_0
#endif

#define HIGH    1
#define LOW     0

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define ADXL345_DEFAULT_ADDRESS (0x53) ///< Assumes ALT address pin low
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
#define ADXL345_REG_DEVID (0x00)        ///< Device ID
#define ADXL345_REG_THRESH_TAP (0x1D)   ///< Tap threshold
#define ADXL345_REG_OFSX (0x1E)         ///< X-axis offset
#define ADXL345_REG_OFSY (0x1F)         ///< Y-axis offset
#define ADXL345_REG_OFSZ (0x20)         ///< Z-axis offset
#define ADXL345_REG_DUR (0x21)          ///< Tap duration
#define ADXL345_REG_LATENT (0x22)       ///< Tap latency
#define ADXL345_REG_WINDOW (0x23)       ///< Tap window
#define ADXL345_REG_THRESH_ACT (0x24)   ///< Activity threshold
#define ADXL345_REG_THRESH_INACT (0x25) ///< Inactivity threshold
#define ADXL345_REG_TIME_INACT (0x26)   ///< Inactivity time
#define ADXL345_REG_ACT_INACT_CTL (0x27) ///< Axis enable control for activity and inactivity detection
#define ADXL345_REG_THRESH_FF (0x28) ///< Free-fall threshold
#define ADXL345_REG_TIME_FF (0x29)   ///< Free-fall time
#define ADXL345_REG_TAP_AXES (0x2A)  ///< Axis control for single/double tap
#define ADXL345_REG_ACT_TAP_STATUS (0x2B) ///< Source for single/double tap
#define ADXL345_REG_BW_RATE (0x2C)        ///< Data rate and power mode control
#define ADXL345_REG_POWER_CTL (0x2D)      ///< Power-saving features control
#define ADXL345_REG_INT_ENABLE (0x2E)     ///< Interrupt enable control
#define ADXL345_REG_INT_MAP (0x2F)        ///< Interrupt mapping control
#define ADXL345_REG_INT_SOURCE (0x30)     ///< Source of interrupts
#define ADXL345_REG_DATA_FORMAT (0x31)    ///< Data format control
#define ADXL345_REG_DATAX0 (0x32)         ///< X-axis data 0
#define ADXL345_REG_DATAX1 (0x33)         ///< X-axis data 1
#define ADXL345_REG_DATAY0 (0x34)         ///< Y-axis data 0
#define ADXL345_REG_DATAY1 (0x35)         ///< Y-axis data 1
#define ADXL345_REG_DATAZ0 (0x36)         ///< Z-axis data 0
#define ADXL345_REG_DATAZ1 (0x37)         ///< Z-axis data 1
#define ADXL345_REG_FIFO_CTL (0x38)       ///< FIFO control
#define ADXL345_REG_FIFO_STATUS (0x39)    ///< FIFO status
/*=========================================================================*/
#define ADXL345_REG_RETURN_DEVID (0xE5)        ///< ID returned by device
/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
#define ADXL345_MG2G_MULTIPLIER (0.004) ///< 4mg per lsb
/*=========================================================================*/


#ifdef __cplusplus
}
#endif