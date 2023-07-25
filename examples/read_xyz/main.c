#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"

#include "adxl345.h"



/*
    Sensor connected
    CS to Vdd (high) for I2C
    SDO to GND  , for ALT address
    see datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf
*/


static void adxl345_task(void *vParm);
static void adxl345_setDataRate1(void *vParm);
static void adxl345_setDataRate1(void *vParm);
static void adxl345_setgetrange(void *vParm);
static void adxl345_getaxis(void *vParm);

static TaskHandle_t AccelTask_h = NULL;
static TaskHandle_t DataRate1_h = NULL;
static TaskHandle_t SetGetRange1_h = NULL;
static TaskHandle_t Axis_h = NULL;
static const char *TAG = "app_main";

void app_main(void)
{

    if (adxl345_begin()) {
        xTaskCreate(adxl345_task, "adxl345_Task", 4096, NULL, 5, &AccelTask_h);
        xTaskCreate(adxl345_setDataRate1, "DataRate", 4096, NULL, 5, &DataRate1_h);
        xTaskCreate(adxl345_setgetrange, "Ranges", 4096, NULL, 5, &SetGetRange1_h);
        xTaskCreate(adxl345_getaxis, "Axis", 4096, NULL, 5, &Axis_h);
    }
    vTaskDelete(NULL);
}


static void adxl345_task(void *vParm)
{

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Get data rate from sensor: %s", adxl345_get_datarate());
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Get data rate from sensor: %s", adxl345_get_datarate());
    vTaskDelay(pdMS_TO_TICKS(100));

    vTaskDelete(NULL);
}


static void adxl345_setDataRate1(void *vParm)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    adxl345_set_datarate(ADXL345_DATARATE_3200_HZ);

    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "Get data rate from sensor: %s", adxl345_get_datarate());

    vTaskDelete(NULL);

}

static void adxl345_setgetrange(void *vParm)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "Read current range from register: %s", adxl345_get_range());

    adxl345_set_range(16);
    ESP_LOGI(TAG, "Read current range from register: %s.", adxl345_get_range());

    vTaskDelete(NULL);
}

static void adxl345_getaxis(void *vParm)
{
    vTaskDelay(pdMS_TO_TICKS(5000));

    static adxl345_xyz_t  axis_data;
    static adxl345_xyz_iir_t  axis_data_iir;

    adxl345_flush_accel_struct(&axis_data);         // zero the struct

    while (1) {

        adxl345_get_accel(&axis_data);
        adxl345_get_accel_iir(&axis_data_iir, 0.01);        
        ESP_LOGI(__func__, "\tX: %3.4f\tY: %3.4f\tZ: %3.4f\t|\tXf: %3.4f\tYf: %3.4f\tZf: %3.4fm/s", axis_data.x_ms, axis_data.y_ms, axis_data.z_ms, axis_data_iir.x_ms, axis_data_iir.y_ms, axis_data_iir.z_ms);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}