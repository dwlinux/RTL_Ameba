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
#include "sleep_ex_api.h"
#include "diag.h"
#include "main.h"

#define GPIO_LED_PIN        PC_5
#define GPIO_PUSHBT_PIN     PA_5

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
    gpio_t gpio_led, gpio_btn;
    int old_btn_state, new_btn_state;

    DBG_INFO_MSG_OFF(_DBG_GPIO_);

    // Init LED control pin
    gpio_init(&gpio_led, GPIO_LED_PIN);
    gpio_dir(&gpio_led, PIN_OUTPUT);    // Direction: Output
    gpio_mode(&gpio_led, PullNone);     // No pull

    // Initial Push Button pin
    gpio_init(&gpio_btn, GPIO_PUSHBT_PIN);
    gpio_dir(&gpio_btn, PIN_INPUT);     // Direction: Input
    gpio_mode(&gpio_btn, PullDown);

    old_btn_state = new_btn_state = 0;
    gpio_write(&gpio_led, 1);

    DiagPrintf("Push button to sleep...\r\n");
    while(1){
        new_btn_state = gpio_read(&gpio_btn);

        if (old_btn_state == 1 && new_btn_state == 0) {
            gpio_write(&gpio_led, 0);
            gpio_mode(&gpio_btn, PullUp);

            DiagPrintf("Sleep 8s... (Or wakeup by pushing button)\r\n");
            standby_wakeup_event_add(STANDBY_WAKEUP_BY_STIMER, 8000, 0);
            standby_wakeup_event_add(STANDBY_WAKEUP_BY_PA5, 0, 1);
            deepstandby_ex();
            DiagPrintf("This line should not be printed\r\n");
        }
        old_btn_state = new_btn_state;
    }
}

