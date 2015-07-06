#ifndef __FREERTOS_PMU_H_
#define __FREERTOS_PMU_H_

#include "sleep_ex_api.h"

#define BIT(n)                   (1<<n)
// wakelock for system usage
#define WAKELOCK_OS              BIT(0)
#define WAKELOCK_WLAN_LPS        BIT(1)
#define WAKELOCK_WLAN_IPS        BIT(2)
#define WAKELOCK_LOGUART         BIT(3)

// wakelock for user defined
#define WAKELOCK_USER_BASE       BIT(16)

// default locked by OS and not to sleep until OS release wakelock in somewhere
#define DEFAULT_WAKELOCK         (WAKELOCK_OS | WAKELOCK_WLAN_LPS | WAKELOCK_WLAN_IPS)

#define DEFAULT_WAKEUP_EVENT (SLEEP_WAKEUP_BY_STIMER | SLEEP_WAKEUP_BY_WLAN)

void acquire_wakelock(uint32_t lock_id);
void release_wakelock(uint32_t lock_id);
uint32_t get_wakelock_status();

#endif
