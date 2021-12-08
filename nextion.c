#include "nextion.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)
#define LED (GPIO_NUM_23)
#define GPIO_OUTPUT_PIN_SEL (1ULL << LED)

TaskHandle_t myTask1Handle = NULL;

void task1(void* arg)
{

    int8_t n1_val = nextion_get_numeric_data("n1");
    int8_t n2_val = nextion_get_numeric_data("n2");
    printf("n1=%d\n", n1_val);
    printf("n2=%d\n", n2_val);

    while (n1_val || n2_val) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (!n2_val) {

            if (n1_val) {
                n2_val = 59;
                n1_val--;
                nextion_send_number("n1", n1_val);
                nextion_send_number("n2", n2_val);
            }
        } else {
            --n2_val;
            nextion_send_number("n2", n2_val);
        }
    }
    myTask1Handle = NULL;
    vTaskDelete(NULL);
}

void btn0_touch_release(void)
{
    printf("Start button released\n");
    if (!myTask1Handle)
        xTaskCreate(task1, "task1", 15000, NULL, 8, &myTask1Handle);
    printf("Start button released func end\n");
}

void btn1_touch_release(void)
{
    printf("Stop button released\n");
    vTaskDelete(myTask1Handle);
    myTask1Handle = NULL;
    printf("Stop button released func end\n");
}

void btn2_touch_release(void)
{
    printf("Led button released\n");

    static int hide;
    if (hide % 2)
        nextion_hide_component_with_name("t0");
    else
        nextion_show_component_with_name("t0");
    hide++;

    int32_t btn2_val = nextion_get_numeric_data("btn2");
    if (btn2_val == 1)
        gpio_set_level(LED, 1);
    else
        gpio_set_level(LED, 0);
    printf("Led button released func end\n");
}

void btn3_touch_release(void)
{
    printf("Change page button released\n");
    nextion_change_page_with_id(1);
    printf("Change page button released func end\n");
}

void slider_release(void)
{
    printf("Slider released\n");
    int32_t h0_val = nextion_get_numeric_data("h0");
    printf("h0=%d\n", h0_val);
    nextion_send_number("n0", h0_val);
    printf("Slider released func end\n");
}

void gpio_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void app_main(void)
{
    gpio_init();

    if (nextion_init(115200, UART_NUM_1, TXD_PIN, RXD_PIN) != ESP_OK)
        esp_restart();

    if (nextion_start() != ESP_OK)
        esp_restart();

    nextion_add_touch_event(0, 1, NEXTION_TOUCH_RELEASED, btn0_touch_release);
    nextion_add_touch_event(0, 4, NEXTION_TOUCH_RELEASED, btn1_touch_release);
    nextion_add_touch_event(0, 5, NEXTION_TOUCH_RELEASED, btn2_touch_release);
    nextion_add_touch_event(0, 2, NEXTION_TOUCH_RELEASED, slider_release);
    nextion_add_touch_event(0, 10, NEXTION_TOUCH_RELEASED, btn3_touch_release);

    nextion_list_events();

    nextion_set_brightness(50);
}
