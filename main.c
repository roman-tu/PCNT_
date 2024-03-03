// /* Pulse counter module - Example

//    For other examples please check:
//    https://github.com/espressif/esp-idf/tree/master/examples

//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "esp_log.h"


#define GPIO_INPUT_IO_TRIGGER 0
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_TRIGGER)

#define SAMPLE_CNT 1
static const adc1_channel_t adc_channel = ADC_CHANNEL_4;
#define LEDC_GPIO 14
static ledc_channel_config_t ledc_channel;
#define PCNT_H_LIM_VAL      10
#define PCNT_L_LIM_VAL     -10
#define PCNT_THRESH1_VAL    5
#define PCNT_THRESH0_VAL   -5
#define PCNT_INPUT_SIG_IO   4  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO  17  // Control GPIO HIGH=count up, LOW=count down
#define LEDC_OUTPUT_IO      2 // Output GPIO of a sample 1 Hz pulse generator

xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

static const char *TAG = "example";

int16_t pulse_count = 0;

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
    int pcnt_unit = (int)arg;
    pcnt_evt_t evt;
    evt.unit = pcnt_unit;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    pcnt_get_event_status(pcnt_unit, &evt.status);
    xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
}

static void init_hw(void)
{
    adc1_config_width(ADC_WIDTH_BIT_10);
    adc1_config_channel_atten(adc_channel, ADC_ATTEN_DB_11);
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 80,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.duty = 0;
    ledc_channel.gpio_num = LEDC_GPIO;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.hpoint = 0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel_config(&ledc_channel);
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    pulse_count++;
}

static void pcnt_example_init(int unit)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        // .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_DISABLE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_DISABLE,    // Keep the primary counter mode if high
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_INC,   // Keep the counter value on the negative edge
        
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(unit, 80);
    pcnt_filter_enable(unit);

    /* Set threshold 0 and 1 values and enable events to watch */
    pcnt_set_event_value(unit, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_1);
    pcnt_set_event_value(unit, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_0);
    /* Enable events on zero, maximum and minimum limit values */
    pcnt_event_enable(unit, PCNT_EVT_ZERO);
    pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* Install interrupt service and add isr callback handler */
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(unit, pcnt_example_intr_handler, (void *)unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(unit);
}

void app_main(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_INPUT_IO_TRIGGER, pcnt_example_intr_handler, (void *)GPIO_INPUT_IO_TRIGGER);
    init_hw();
    gpio_set_direction(4, GPIO_MODE_INPUT);
    int pcnt_unit = PCNT_UNIT_0;
        /* Initialize PCNT event queue and PCNT functions */
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    pcnt_example_init(pcnt_unit);
    pcnt_evt_t evt;
    portBASE_TYPE res;
    // xTaskCreatePinnedToCore(encoded_output, "encoded_output", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL, APP_CPU_NUM);
    while (1) {
         uint32_t adc_val = 0;
        for (int i = 0; i < SAMPLE_CNT; ++i)
        {
            adc_val += adc1_get_raw(adc_channel);
        }
        adc_val /= SAMPLE_CNT;


        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, adc_val);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 100 / portTICK_PERIOD_MS);
        if (res == pdTRUE) {
            // pcnt_get_counter_value(pcnt_unit, &count);
            // ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);
            // if (evt.status & PCNT_EVT_THRES_1) {
            //     ESP_LOGI(TAG, "THRES1 EVT");
            // }
            // if (evt.status & PCNT_EVT_THRES_0) {
            //     ESP_LOGI(TAG, "THRES0 EVT");
            // }
            // if (evt.status & PCNT_EVT_L_LIM) {
            //     ESP_LOGI(TAG, "L_LIM EVT");
            // }
            // if (evt.status & PCNT_EVT_H_LIM) {
            //     ESP_LOGI(TAG, "H_LIM EVT");
            // }
            // if (evt.status & PCNT_EVT_ZERO) {
            //     ESP_LOGI(TAG, "ZERO EVT");
            // }
        } else {
            pcnt_get_counter_value(pcnt_unit, &pulse_count);
            ESP_LOGI(TAG, "Current counter value :%d", pulse_count);
        }
        printf("Pulse count: %d\n", pulse_count);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "driver/ledc.h"
// #include "driver/pcnt.h"
// #include "esp_attr.h"
// #include "esp_log.h"



// /**
//  * TEST CODE BRIEF
//  *
//  * Use PCNT module to count rising edges generated by LEDC module.
//  *
//  * Functionality of GPIOs used in this example:
//  *   - GPIO18 - output pin of a sample 1 Hz pulse generator,
//  *   - GPIO4 - pulse input pin,
//  *   - GPIO5 - control input pin.
//  *
//  * Load example, open a serial port to view the message printed on your screen.
//  *
//  * To do this test, you should connect GPIO18 with GPIO4.
//  * GPIO5 is the control signal, you can leave it floating with internal pull up,
//  * or connect it to ground. If left floating, the count value will be increasing.
//  * If you connect GPIO5 to GND, the count value will be decreasing.
//  *
//  * An interrupt will be triggered when the counter value:
//  *   - reaches 'thresh1' or 'thresh0' value,
//  *   - reaches 'l_lim' value or 'h_lim' value,
//  *   - will be reset to zero.
//  */
// #define PCNT_H_LIM_VAL      10
// #define PCNT_L_LIM_VAL     -10
// #define PCNT_THRESH1_VAL    5
// #define PCNT_THRESH0_VAL   -5
// #define PCNT_INPUT_SIG_IO   5  // Pulse Input GPIO
// #define PCNT_INPUT_CTRL_IO  17  // Control GPIO HIGH=count up, LOW=count down
// #define LEDC_OUTPUT_IO      2 // Output GPIO of a sample 1 Hz pulse generator

// xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

// /* A sample structure to pass events from the PCNT
//  * interrupt handler to the main program.
//  */
// typedef struct {
//     int unit;  // the PCNT unit that originated an interrupt
//     uint32_t status; // information on the event type that caused the interrupt
// } pcnt_evt_t;

// /* Decode what PCNT's unit originated an interrupt
//  * and pass this information together with the event type
//  * the main program using a queue.
//  */
// static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
// {
//     int pcnt_unit = (int)arg;
//     pcnt_evt_t evt;
//     evt.unit = pcnt_unit;
//     /* Save the PCNT event type that caused an interrupt
//        to pass it to the main program */
//     pcnt_get_event_status(pcnt_unit, &evt.status);
//     xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
// }

// void encoded_output(void *args){
//   for(;;){
//     // gpio_set_level(2,1);
//     // gpio_set_level(2,0);
//     gpio_set_level(12,1);
//     vTaskDelay(50/portTICK_RATE_MS);
//     gpio_set_level(14,1);
//     vTaskDelay(50/portTICK_RATE_MS);
//     gpio_set_level(12,0);
//     vTaskDelay(50/portTICK_RATE_MS);
//     gpio_set_level(14,0);
//     // ESP_LOGI("OP", "hah");
//     vTaskDelay(50/portTICK_RATE_MS);
//   }
// }

// /* Configure LED PWM Controller
//  * to output sample pulses at 1 Hz with duty of about 10%
//  */
// static void ledc_init(void)
// {
//     // Prepare and then apply the LEDC PWM timer configuration
//     ledc_timer_config_t ledc_timer;
//     ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;
//     ledc_timer.timer_num        = LEDC_TIMER_1;
//     ledc_timer.duty_resolution  = LEDC_TIMER_10_BIT;
//     ledc_timer.freq_hz          = 1;  // set output frequency at 1 Hz
//     ledc_timer.clk_cfg = LEDC_AUTO_CLK;
//     ledc_timer_config(&ledc_timer);

//     // Prepare and then apply the LEDC PWM channel configuration
//     ledc_channel_config_t ledc_channel;
//     ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
//     ledc_channel.channel    = LEDC_CHANNEL_1;
//     ledc_channel.timer_sel  = LEDC_TIMER_1;
//     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
//     ledc_channel.gpio_num   = LEDC_OUTPUT_IO;
//     ledc_channel.duty       = 100; // set duty at about 10%
//     ledc_channel.hpoint     = 0;
//     ledc_channel_config(&ledc_channel);
// }

// /* Initialize PCNT functions:
//  *  - configure and initialize PCNT
//  *  - set up the input filter
//  *  - set up the counter events to watch
//  */
// static void pcnt_example_init(int unit)
// {
//     /* Prepare configuration for the PCNT unit */
//     pcnt_config_t pcnt_config = {
//         // Set PCNT input signal and control GPIOs
//         .pulse_gpio_num = PCNT_INPUT_SIG_IO,
//         .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
//         // What to do when control input is low or high?
//         .lctrl_mode = PCNT_MODE_DISABLE, // Reverse counting direction if low
//         .hctrl_mode = PCNT_MODE_DISABLE,    // Keep the primary counter mode if high
//         .channel = PCNT_CHANNEL_0,
//         .unit = unit,
//         // What to do on the positive / negative edge of pulse input?
//         .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
//         .neg_mode = PCNT_COUNT_INC,   // Keep the counter value on the negative edge
        
//         // Set the maximum and minimum limit values to watch
//         .counter_h_lim = 32767,
//         .counter_l_lim = -32768,
//     };
//     /* Initialize PCNT unit */
//     pcnt_unit_config(&pcnt_config);

//     /* Configure and enable the input filter */
//     pcnt_set_filter_value(unit, 80);
//     pcnt_filter_enable(unit);

//     /* Set threshold 0 and 1 values and enable events to watch */
//     pcnt_set_event_value(unit, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
//     pcnt_event_enable(unit, PCNT_EVT_THRES_1);
//     pcnt_set_event_value(unit, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
//     pcnt_event_enable(unit, PCNT_EVT_THRES_0);
//     /* Enable events on zero, maximum and minimum limit values */
//     pcnt_event_enable(unit, PCNT_EVT_ZERO);
//     pcnt_event_enable(unit, PCNT_EVT_H_LIM);
//     pcnt_event_enable(unit, PCNT_EVT_L_LIM);

//     /* Initialize PCNT's counter */
//     pcnt_counter_pause(unit);
//     pcnt_counter_clear(unit);

//     /* Install interrupt service and add isr callback handler */
//     pcnt_isr_service_install(0);
//     pcnt_isr_handler_add(unit, pcnt_example_intr_handler, (void *)unit);

//     /* Everything is set up, now go to counting */
//     pcnt_counter_resume(unit);
// }

// void app_main(void)
// {
//     gpio_set_direction(5, GPIO_MODE_INPUT);
//     int pcnt_unit = PCNT_UNIT_0;
//     /* Initialize LEDC to generate sample pulse signal */
//     ledc_init();

//     /* Initialize PCNT event queue and PCNT functions */
//     pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
//     pcnt_example_init(pcnt_unit);

//     int16_t count = 0;
//     pcnt_evt_t evt;
//     portBASE_TYPE res;
//     // xTaskCreatePinnedToCore(encoded_output, "encoded_output", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL, APP_CPU_NUM);
//     while (1) {
//         /* Wait for the event information passed from PCNT's interrupt handler.
//          * Once received, decode the event type and print it on the serial monitor.
//          */
//         res = xQueueReceive(pcnt_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
//         if (res == pdTRUE) {
//             pcnt_get_counter_value(pcnt_unit, &count);
//             ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);
//             if (evt.status & PCNT_EVT_THRES_1) {
//                 ESP_LOGI(TAG, "THRES1 EVT");
//             }
//             if (evt.status & PCNT_EVT_THRES_0) {
//                 ESP_LOGI(TAG, "THRES0 EVT");
//             }
//             if (evt.status & PCNT_EVT_L_LIM) {
//                 ESP_LOGI(TAG, "L_LIM EVT");
//             }
//             if (evt.status & PCNT_EVT_H_LIM) {
//                 ESP_LOGI(TAG, "H_LIM EVT");
//             }
//             if (evt.status & PCNT_EVT_ZERO) {
//                 ESP_LOGI(TAG, "ZERO EVT");
//             }
//         } else {
//             // pcnt_get_counter_value(pcnt_unit, &count);
//             ESP_LOGI(TAG, "Current counter value :%d", count);
//         }
//         ESP_LOGI(TAG, "GPIO5 : %d", gpio_get_level(5));
//         if(gpio_get_level(5) == 0){
//           count++;
//         }
//         vTaskDelay(20/portTICK_RATE_MS);
//     }
// }