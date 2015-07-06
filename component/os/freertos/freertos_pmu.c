#include "FreeRTOS.h"

#include "freertos_pmu.h"

#include "platform_autoconf.h"
#include "sleep_ex_api.h"
#include "wlan_intf.h"

static uint32_t wakelock     = DEFAULT_WAKELOCK;
static uint32_t wakeup_event = DEFAULT_WAKEUP_EVENT;

_WEAK void rltk_wlan_pre_sleep_processing (void) {
    // Add dummy function for MP
}

_WEAK void rltk_wlan_post_sleep_processing (void) {
    // Add dummy function for MP
}

/* ++++++++ FreeRTOS macro implementation ++++++++ */

/*
 *  It is called in idle task.
 *
 *  @return  true  : System is ready to check conditions that if it can enter sleep.
 *           false : System keep awake.
 **/
int freertos_ready_to_sleep() {
    return wakelock == 0;
}

/*
 *  It is called when freertos is going to sleep.
 *  At this moment, all sleep conditons are satisfied. All freertos' sleep pre-processing are done.
 *
 *  @param  expected_idle_time : The time that FreeRTOS expect to sleep.
 *                               If we set this value to 0 then FreeRTOS will do nothing in its sleep function.
 **/
void freertos_pre_sleep_processing(unsigned int *expected_idle_time) {

#ifdef CONFIG_SOC_PS_MODULE

    unsigned int stime;

    /* To disable freertos sleep function and use our sleep function, 
     * we can set original expected idle time to 0. */
    stime = *expected_idle_time;
    *expected_idle_time = 0;

#ifdef CONFIG_LITTLE_WIFI_MCU_FUNCTION_THREAD
#ifdef CONFIG_POWER_SAVING
    if (wakeup_event & SLEEP_WAKEUP_BY_WLAN) {
        rltk_wlan_pre_sleep_processing();
    }
#endif
#endif

    sleep_ex(wakeup_event, stime);

#ifdef CONFIG_LITTLE_WIFI_MCU_FUNCTION_THREAD
#ifdef CONFIG_POWER_SAVING
    if (wakeup_event & SLEEP_WAKEUP_BY_WLAN) {
        rltk_wlan_post_sleep_processing();
    }
#endif
#endif

#else
    // If PS is not enabled, then use freertos sleep function
#endif
}

/* -------- FreeRTOS macro implementation -------- */

void acquire_wakelock(uint32_t lock_id) {
    wakelock |= lock_id;
}

void release_wakelock(uint32_t lock_id) {
    wakelock &= ~lock_id;
}

uint32_t get_wakelock_status() {
	return wakelock;
}

void add_wakeup_event(uint32_t event) {
    wakeup_event |= event;
}

void del_wakeup_event(uint32_t event) {
    wakeup_event &= ~event;
    // To fulfill tickless design, system timer is required to be wakeup event
    wakeup_event |= SLEEP_WAKEUP_BY_STIMER;
}

