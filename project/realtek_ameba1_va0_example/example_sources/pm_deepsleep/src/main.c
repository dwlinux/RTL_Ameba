/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "device.h"
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "sleep_ex_api.h"
#include "sys_api.h"
#include "diag.h"
#include "main.h"

#define GPIO_LED_PIN        PC_5
#define GPIO_IRQ_PIN        PC_4

// deep sleep can only be waked up by GPIOB_1 and GTimer
#define GPIO_WAKE_PIN       PB_1

void gpio_demo_irq_handler (uint32_t id, gpio_irq_event event)
{
    gpio_t *gpio_led;
    gpio_led = (gpio_t *)id;

    printf("Enter deep sleep...Wait 10s or give rising edge at PB_1 to wakeup system.\r\n\r\n");

    // turn off led
    gpio_write(gpio_led, 0);

    // turn off log uart
    sys_log_uart_off();

    // initialize wakeup pin at PB_1
    gpio_t gpio_wake;
    gpio_init(&gpio_wake, GPIO_WAKE_PIN);
    gpio_dir(&gpio_wake, PIN_INPUT);
    gpio_mode(&gpio_wake, PullDown);

    // enter deep sleep
    deepsleep_ex(DSLEEP_WAKEUP_BY_GPIO | DSLEEP_WAKEUP_BY_TIMER, 10000);
}

void main(void)
{
    gpio_t gpio_led;
    gpio_irq_t gpio_btn;

    // Init LED control pin
    gpio_init(&gpio_led, GPIO_LED_PIN);
    gpio_dir(&gpio_led, PIN_OUTPUT);    // Direction: Output
    gpio_mode(&gpio_led, PullNone);     // No pull

    // Initial Push Button pin as interrupt source
    gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, gpio_demo_irq_handler, (uint32_t)(&gpio_led));
    gpio_irq_set(&gpio_btn, IRQ_FALL, 1);   // Falling Edge Trigger
    gpio_irq_enable(&gpio_btn);

    // led on means system is in run mode
    gpio_write(&gpio_led, 1);
    printf("\r\nPush button at PC_4 to enter deep sleep\r\n");

    while(1);
}
