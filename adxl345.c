#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_err.h"

#include "adxl345.h"

/** Resources
 * Inclination sensing: https://www.analog.com/en/app-notes/an-1057.html
 *
*/


/* prototype static functions */

static uint8_t adxl345_read8(uint8_t reg_addr);
static int16_t adxl345_read16(uint8_t reg_addr);
static void adxl345_write(uint8_t reg_addr, uint8_t value);
static double dsp_ema_i32(double in, double average, float alpha );
static char *print_byte(uint8_t byte);
static char *adxl345_return_datarate(uint8_t _data_rate);
static char *adxl345_return_range(uint8_t _range_data);


/**
 * @brief Initialize the sensor and set some basic parameters
 * @param  none
 * @return true is sucess. s
 */
bool adxl345_begin(void)
{
    esp_err_t ret = ESP_OK;
    ret = adxl345_chipid();     // check if the sensor reposds by asking the ChipID.

    if (ret == ESP_OK) {

        adxl345_set_auto_sleep(true, ADXL345_WAKE_1HZ);         // read during sleep freq: ADXL345_WAKE_8HZ. ADXL345_WAKE_4HZ, ADXL345_WAKE_2HZ, ADXL345_WAKE_1HZ
        adxl345_set_fullres_mode(true);                         // enable highest dynamic range
        adxl345_start_measure();                                // start measuring

    } else {
        ESP_LOGE(__func__, "Something on the knikker");
        return false;
    }

    return true;
}

/**
 * @brief Read and compare Device ID
 * @param  n/a
 * @return ESP_OK or ESP_FAIL
 */
esp_err_t adxl345_chipid(void)
{
    static uint8_t rx[1];

    rx[0] = adxl345_read8(ADXL345_REG_DEVID);

    if (ADXL345_REG_RETURN_DEVID == rx[0]) {
        ESP_LOGD(__func__, "ChipID returned: 0x%X", rx[0]);
        return ESP_OK;
    } else {
        ESP_LOGE(__func__, "Invalid ChipID returned: 0x%X", rx[0]);
        return ESP_FAIL;
    }
}

/**
 * @brief Read 1 Byte from the sensor
 * @param reg_addr The register address your want to read
 * @return value read from the register is returned as a uint8_t
 */
static uint8_t adxl345_read8(uint8_t reg_addr)
{
    esp_err_t err;
    static uint8_t rx[1];

    err = i2c_manager_read(I2C_PORT, ADXL345_I2C_ADDRESS, reg_addr, rx, 1);
    if (err != ESP_OK) {
        ESP_LOGE(__func__, "Reading sensor register 0x%x failed, error: %d", reg_addr, err);
        return -1;
    }

    return rx[0];
}

/**
 * @brief Read 2 Bytes from the sensor
 * @param reg_addr The register address your want to read
 * @return value read from the register is returned as a int16_t
 */
static int16_t adxl345_read16(uint8_t reg_addr)
{
    esp_err_t err;
    static uint8_t rx[2];

    err = i2c_manager_read(I2C_PORT, ADXL345_I2C_ADDRESS, reg_addr, rx, 2);
    if (err != ESP_OK) {
        ESP_LOGE(__func__, "Reading sensor register 0x%x failed, error: %d", reg_addr, err);
        return -1;
    }

    return (uint16_t)rx[1] << 8 | (uint16_t)rx[0];
}

/**
 * @brief Write to a register
 * @param reg_addr the register address
 * @param value the value you want to write
 */
static void adxl345_write(uint8_t reg_addr, uint8_t value)
{
    esp_err_t err;
    static  uint8_t tx[1];
    tx[0] = value;

    err = i2c_manager_write(I2C_PORT, ADXL345_I2C_ADDRESS, reg_addr, tx, 1);

    if (err != ESP_OK) {
        ESP_LOGE(__func__, "I2C Write failed to register 0x%X , sendign value 0x%X", ADXL345_I2C_ADDRESS, tx[0]);
    }
}

/**
 * @brief Read Bandwidth rate from sensor
 * @return return data rate
 */
char *adxl345_get_datarate(void)
{
    static uint8_t data_rate_v = 0;
    data_rate_v = (adxl345_read8(ADXL345_REG_BW_RATE) & 0x0F);

    return adxl345_return_datarate(data_rate_v);
}


/**
 * @brief set the data rate
 * @param data_rate the data rate to set
 */
void adxl345_set_datarate(adxl345_datarate_t data_rate)
{
    adxl345_write(ADXL345_REG_BW_RATE, data_rate);
}


/**
 * @brief Get range value in G's
 * @return return range in G, default is 2G (0b00)
 */
char *adxl345_get_range(void)
{
    static uint8_t data_range_v = 0;
    data_range_v = (adxl345_read8(ADXL345_REG_DATA_FORMAT) & 0x03);

    return adxl345_return_range(data_range_v);
}


/**
 * @brief Set the g range ( 2g, 4g, 8g, 16g )
 * @param range G's
 */
void adxl345_set_range(uint8_t range)
{
    // Read the DATA_FORMAT register current values, so we can flip our bits only and leave the rest
    uint8_t data_format_reg = adxl345_read8(ADXL345_REG_DATA_FORMAT);

    switch (range) {
    case 2:
        data_format_reg |= ADXL345_RANGE_2_G;
        break;
    case 4:
        data_format_reg |= ADXL345_RANGE_4_G;
        break;
    case 8:
        data_format_reg |= ADXL345_RANGE_8_G;
        break;
    case 16:
        data_format_reg |= ADXL345_RANGE_16_G;
        break;
    default:
        ESP_LOGE(__func__, "Not a valid range value");
        break;
    }
    //data_format_reg &= ~0x0F;       // select only the bits we're interested in " 0b000 (000)" .
    //data_format_reg |= range;       // flip the bits to set the new range: "0b000(00xx)".

    // and while we're here set FULL_RES mode as well.
    //  data_format_reg |= 0x08;        // 0x08 = 0b1000 => 0b000 (X000);
    ESP_LOGI(__func__, "Set data range:\t0x%X = 0b%s", data_format_reg, print_byte(data_format_reg));
    // write new value to the register
    adxl345_write(ADXL345_REG_DATA_FORMAT, data_format_reg);
}


/**
 * @brief
 * @param onoff
 */
void adxl345_set_fullres_mode(bool onoff)
{
    uint8_t full_res_mode = 0;
    full_res_mode = adxl345_read8(ADXL345_REG_DATA_FORMAT);
    ESP_LOGI(__func__, "Current reg vals: \t0x%X = 0b%s", full_res_mode, print_byte(full_res_mode));

    if (onoff) {
        full_res_mode |= 0x08;          // flip fullres bit on
    } else {
        full_res_mode ^= 0x08;          // flip fullress bit off
    }

    adxl345_write(ADXL345_REG_DATA_FORMAT, full_res_mode);
    //sanitytititiies
    full_res_mode = adxl345_read8(ADXL345_REG_DATA_FORMAT);
    ESP_LOGI(__func__, "After write: \t0x%X = 0b%s", full_res_mode, print_byte(full_res_mode));
}

/**
 * @brief Read x,y,z data from registers
 * @param  void
 * @return bits and bytes and blops
 */
int16_t adxl345_get_x(void)
{
    return adxl345_read16(ADXL345_REG_DATAX0);
}

/**
 * @brief Read x,y,z data from registers
 * @param  void
 * @return bits and bytes and blops
 */
int16_t adxl345_get_y(void)
{
    return adxl345_read16(ADXL345_REG_DATAY0);
}

/**
 * @brief Read x,y,z data from registers
 * @param  void
 * @return bits and bytes and blops
 */
int16_t adxl345_get_z(void)
{
    return adxl345_read16(ADXL345_REG_DATAZ0);
}


/**
 * @brief 3 Accel functions combined and return value tru struct
 * @param accel
 */
void adxl345_get_accel(adxl345_xyz_t *accel)
{
    accel->x = adxl345_get_x();
    accel->y = adxl345_get_y();
    accel->z = adxl345_get_z();
    accel->x_ms = (accel->x * ADXL345_MG2G_MULTIPLIER * GRAVITY);
    accel->y_ms = (accel->y * ADXL345_MG2G_MULTIPLIER * GRAVITY);
    accel->z_ms = (accel->z * ADXL345_MG2G_MULTIPLIER * GRAVITY);
}

/**
 * @brief Zero all accel values in struct
 * @param accel accel struct
 */
void adxl345_flush_accel_struct(adxl345_xyz_t *accel)
{
    accel->x = 0;
    accel->y = 0;
    accel->z = 0;
    accel->x_ms = 0.0F;
    accel->y_ms = 0.0F;
    accel->z_ms = 0.0F;
}

/**
 * @brief Enable/Disable auto_sleep mode & set freq of readings in sleep mode
 *
        Register POWER_CTL 0x2D
        --------------------------------------------------------------------
        | D7  |  D6  |  D5  |      D4     |     D3   |   D2  |  D1  |  D0  |
        --------------------------------------------------------------------
        |  0  |  0   | LINK | AUTO_SLEEP  | MEASURE  | SLEEP |    WAKEUP   |
        --------------------------------------------------------------------
        In wakeup bits D0 and D1 we can specify the frequency of measurment
        during auto_sleep.

        | D1 | D0 |  Hz
        ---------------------
        | 0  |  0 |  8
        ---------------------
        | 0  |  1 |  4
        ---------------------
        | 1  |  0 |  2
        ---------------------
        | 1  |  1 |  1
        ---------------------
 * @param enable auto_Sleep
 */
void adxl345_set_auto_sleep(bool flip, adxl345_autosleep_readhz_t freq)
{

    static uint8_t power_ctl_reg = 0;

    power_ctl_reg = adxl345_read8(ADXL345_REG_POWER_CTL);
    ESP_LOGI(__func__, "Read Reg: ADXL345_REG_POWER_CTL:\t0x%X = 0b%s", power_ctl_reg, print_byte(power_ctl_reg));

    if (flip) {
        power_ctl_reg = 0x10 | freq;                         // 16 = 0001 0000 | 0000 0011
        adxl345_write(ADXL345_REG_POWER_CTL, power_ctl_reg);

        power_ctl_reg = adxl345_read8(ADXL345_REG_POWER_CTL);
        ESP_LOGI(__func__, "After write: ADXL345_REG_POWER_CTL:\t0x%X = 0b%s", power_ctl_reg, print_byte(power_ctl_reg));

    } else {

        adxl345_write(ADXL345_REG_POWER_CTL, 0x0);

        power_ctl_reg = adxl345_read8(ADXL345_REG_POWER_CTL);
        ESP_LOGI(__func__, "After write: ADXL345_REG_POWER_CTL:\t0x%X = 0b%s", power_ctl_reg, print_byte(power_ctl_reg));

    }
}

/**
 * @brief Start measuring
 */
void adxl345_start_measure(void)
{
    static uint8_t power_ctl_reg = 0;

    power_ctl_reg = adxl345_read8(ADXL345_REG_POWER_CTL);
    ESP_LOGI(__func__, "Read Reg: ADXL345_REG_POWER_CTL:\t0x%X = 0b%s", power_ctl_reg, print_byte(power_ctl_reg));

    power_ctl_reg |= 0x8;                            // 0b 0000 (1000)      Start Measuring
    adxl345_write(ADXL345_REG_POWER_CTL, power_ctl_reg);

    power_ctl_reg = adxl345_read8(ADXL345_REG_POWER_CTL);
    ESP_LOGI(__func__, "After write: ADXL345_REG_POWER_CTL:\t0x%X = 0b%s", power_ctl_reg, print_byte(power_ctl_reg));
}



/**
 * @brief Exponential moving averaing filter - IIR - Low pass
 *          https://blog.stratifylabs.co/device/2013-10-04-An-Easy-to-Use-Digital-Filter/
 * @param in    - Input value from sensor
 * @param average - Previous avg value
 * @param alpha   - the smoothing factor
 */
/* int32_t dsp_ema_i32(int32_t in, int32_t average, uint16_t alpha )
{
    if ( alpha == 65535 ) {
        return in;
    }

    int64_t tmp0;
    tmp0 = (int64_t)in * (alpha + 1) + average * (65536 - alpha);
    return (int32_t)((tmp0 + 32768) / 65536);
} */
static double IRAM_ATTR dsp_ema_i32(double in, double average, float alpha )
{
    if (alpha > 1.0f) {
        alpha = 1.0f;
    } else if (alpha < 0.0f) {
        alpha = 0.0f;
    }

    return (double)(in * (alpha) + (average * (1 - alpha)));
}



/**
 * @brief Identical as adxl345_get_accel() but with an IIR low pass filter added
 *        both on raw data and coverted to m/s2 data.
  * @param out - Struct that contains the values
 * @param alpha - Smoothing factor, between 0 and 1.
 */
void adxl345_get_accel_iir(adxl345_xyz_iir_t *out, float alpha)
{
    out->x = dsp_ema_i32(adxl345_get_x(), out->x, (alpha));
    out->y = dsp_ema_i32(adxl345_get_y(), out->y, (alpha));
    out->z = dsp_ema_i32(adxl345_get_z(), out->z, (alpha));

    out->x_ms = (out->x * ADXL345_MG2G_MULTIPLIER * GRAVITY);
    out->y_ms = (out->y * ADXL345_MG2G_MULTIPLIER * GRAVITY);
    out->z_ms = (out->z * ADXL345_MG2G_MULTIPLIER * GRAVITY);
}




/**
 * @brief Acivate selftest mode
 *      Register DATA_FORMAT 0x31
        ---------------------------------------------------------------------------
        |      D7    |  D6  |      D5     | D4 |    D3    |    D2   |  D1  |  D0  |
        ---------------------------------------------------------------------------
        | SELF_TEST  | SPI  |  INT_INVERT | 0  | FULL_RES | JUSTIFY |   RANGE     |
        ---------------------------------------------------------------------------

        Enable SELF_TEST: set D7 to 1

 * @param _selftest  yes/no
 */
void adxl345_start_selftest(bool _selftest)
{
    static uint8_t set_selftest = 0;
    set_selftest = adxl345_read8(ADXL345_REG_DATA_FORMAT);

    if (_selftest) {
        set_selftest |= 0x80;
    } else {
        set_selftest ^= 0x80;
    }

    adxl345_write(ADXL345_REG_DATA_FORMAT, set_selftest);
}

/* <=====================================================================================> */

/*
        TOOLBOX

*/

// binary values lookup table
const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

static char *print_byte(uint8_t byte)
{
    static char binbyte[8];
    snprintf(binbyte, sizeof(binbyte) + 1, "%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
    return binbyte;
}


/**
 * @brief
 * @param _range_data
 * @return
 */
static char *adxl345_return_range(uint8_t _range_data)
{
    static char return_range[20];

    switch (_range_data) {
    case 0:
        snprintf(return_range, sizeof(return_range) + 1, "ADXL345_RANGE_2_G");
        break;
    case 1:
        snprintf(return_range, sizeof(return_range) + 1, "ADXL345_RANGE_4_G");
        break;
    case 2:
        snprintf(return_range, sizeof(return_range) + 1, "ADXL345_RANGE_8_G");
        break;
    case 3:
        snprintf(return_range, sizeof(return_range) + 1, "ADXL345_RANGE_16_G");
        break;
    default:
        snprintf(return_range, sizeof(return_range) + 1, "ADXL345_RANGE_ERROR");
        break;
    }
    return return_range;
}


/**
 * @brief Return data rate in text format
 * @param _data_rate
 * @return
 */
static char *adxl345_return_datarate(uint8_t _data_rate)
{
    static char return_data_rate[25];

    switch (_data_rate) {
    case 0:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_0_10_HZ");
        break;
    case 1:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_0_20_HZ");
        break;
    case 2:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_0_39_HZ");
        break;
    case 3:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_0_78_HZ");
        break;
    case 4:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1,  "ADXL345_DATARATE_1_56_HZ");
        break;
    case 5:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_3_13_HZ");
        break;
    case 6:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1,  "ADXL345_DATARATE_6_25HZ");
        break;
    case 7:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_12_5_HZ");
        break;
    case 8:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_25_HZ");
        break;
    case 9:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_50_HZ");
        break;
    case 10:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1,  "ADXL345_DATARATE_100_HZ");
        break;
    case 11:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_200_HZ");
        break;
    case 12:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_400_HZ");
        break;
    case 13:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_800_HZ");
        break;
    case 14:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_1600_HZ");
        break;
    case 15:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_3200_HZ");
        break;
    default:
        snprintf(return_data_rate, sizeof(return_data_rate) + 1, "ADXL345_DATARATE_ERROR");
        break;
    }

    return return_data_rate;
}






/*
https://www.embedded.com/bitwise-operations-on-device-registers/
https://www.electrosoftcloud.com/en/bit-by-bit-operations-bitwise/
How to convert binary to hex
Convert every 4 binary digits (start from bit 0) to 1 hex digit, with this table:

Binary  Hex
0000    0
0001    1
0010    2
0011    3
0100    4
0101    5
0110    6
0111    7
1000    8
1001    9
1010    A
1011    B
1100    C
1101    D
1110    E
1111    F
0001 000    0x10    16

*/


/* void adxl345_set_fifo(adxl345_fifo_mode_t mode)
{

        Register FIFO_CTL 0x38
        -------------------------------------------------------
        | D7  |  D6  |  D5  |  D4  | D3  |  D2  |  D1  |  D0  |
        -------------------------------------------------------
        | FIFO_MODE  | TRIG |       SAMPLES                   |
        -------------------------------------------------------
        we focus on D7 - D6

static uint8_t fifo_ctl_reg = 0;
fifo_ctl_reg = adxl345_read8(ADXL345_REG_FIFO_CTL);

fifo_ctl_reg &= ~0xF0;      // mask left part 0b (0000) 0000

switch (mode)
{
case bypass:
    fifo_ctl_reg |= bypass;
    break;
case fifo:
    fifo_ctl_reg |= fifo;
    break;
case stream:
    fifo_ctl_reg |= stream;
    break;
case trigger:
    fifo_ctl_reg |= trigger;
    break;
default:
    ESP_LOGE(__func__, "Wrong fifo mode ??");
    break;
}
adxl345_write(ADXL345_REG_FIFO_CTL, fifo_ctl_reg);
} */