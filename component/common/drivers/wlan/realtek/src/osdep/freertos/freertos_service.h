#ifndef _FREERTOS_SERVICE_H_
#define _FREERTOS_SERVICE_H_

//----- ------------------------------------------------------------------
// Include Files
//----- ------------------------------------------------------------------
#include "wireless.h"

// --------------------------------------------
//	Platform dependent include file
// --------------------------------------------
#if defined(CONFIG_PLATFORM_8195A) || defined(CONFIG_PLATFORM_8711B)
#include "platform/platform_stdlib.h"
extern VOID RtlUdelayOS(u32 us);
#else
// other MCU may use standard library 
#include <string.h>
#endif


#if (defined CONFIG_GSPI_HCI || defined CONFIG_SDIO_HCI) || defined(CONFIG_LX_HCI)
/* For SPI interface transfer and us delay implementation */
#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)
#include <rtwlan_bsp.h>	
#endif
#endif


// --------------------------------------------
//	Platform dependent type define
// --------------------------------------------
#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long 	s64;
typedef unsigned long long	u64;
typedef unsigned int		uint;
typedef signed int			sint;

#ifndef bool
typedef int				bool;
#define  true				1
#define  false				0
#endif

#define IN
#define OUT
#define VOID void
#define NDIS_OID uint
#define NDIS_STATUS uint
#ifndef	PVOID
typedef void *		PVOID;
#endif

typedef unsigned int		__kernel_size_t;
typedef int			__kernel_ssize_t;
typedef	__kernel_size_t		SIZE_T;	
typedef	__kernel_ssize_t	SSIZE_T;

#endif //CONFIG_PLATFORM_8195A

#define FIELD_OFFSET(s,field)	((SSIZE_T)&((s*)(0))->field)

// os types
typedef char			osdepCHAR;
typedef float			osdepFLOAT;
typedef double			osdepDOUBLE;
typedef long			osdepLONG;
typedef short			osdepSHORT;
typedef unsigned long	osdepSTACK_TYPE;
typedef long			osdepBASE_TYPE;
typedef unsigned long	osdepTickType;

typedef void*	_timerHandle;
typedef void*	_sema;
typedef void*	_mutex;
typedef void*	_lock;
typedef void*	_queueHandle;
typedef void*	_xqueue;
typedef struct timer_list	_timer;

typedef	struct sk_buff		_pkt;
typedef unsigned char		_buffer;

struct list_head {
	struct list_head *next, *prev;
};

struct	__queue	{
	struct	list_head	queue;
	_lock			lock;
};

typedef struct	__queue		_queue;
typedef struct	list_head	_list;
typedef unsigned long		_irqL;

typedef void*			_thread_hdl_;
typedef void			thread_return;
typedef void*			thread_context;

#define ATOMIC_T atomic_t
#define HZ configTICK_RATE_HZ

#define   KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
/* emulate a modern version */
#define LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 17)

static __inline _list *get_next(_list	*list)
{
	return list->next;
}	

static __inline _list	*get_list_head(_queue	*queue)
{
	return (&(queue->queue));
}

#define LIST_CONTAINOR(ptr, type, member) \
	((type *)((char *)(ptr)-(SIZE_T)((char *)&((type *)ptr)->member - (char *)ptr)))
#define container_of(p,t,n) (t*)((p)-&(((t*)0)->n))

#define TASK_PRORITY_LOW  				1
#define TASK_PRORITY_MIDDLE   			2
#define TASK_PRORITY_HIGH    			3
#define TASK_PRORITY_SUPER    			4

#define TIMER_MAX_DELAY    				0xFFFFFFFF

// rtl8195a uses receive_tasklet for wps
// 8189em uses interrupt_thread for wps 
#if defined(CONFIG_WPS) 
#define RECV_STACK_FOR_WPS 448//512//384 //Change to 512 for WPS (IAR STM32) stack overflow
#else
#define RECV_STACK_FOR_WPS 0
#endif

#ifdef CONFIG_DONT_CARE_TP
#define XMIT_STACKSIZE		192 //256
#define CMD_STACKSIZE		384 //512 
#else
#define XMIT_STACKSIZE		256
#define CMD_STACKSIZE		512 //1024
#endif //CONFIG_DONT_CARE_TP

#ifdef CONFIG_PLATFORM_8195A
#define RECV_STACKSIZE		256
#else //CONFIG_PLATFORM_8195A
#ifdef CONFIG_INCLUDE_WPA_PSK
#if PSK_SUPPORT_TKIP
#define RECV_STACKSIZE		(512 + 256 + 128 + RECV_STACK_FOR_WPS)
#else
#define RECV_STACKSIZE		(512 + 256 + RECV_STACK_FOR_WPS )
#endif
#else
#define RECV_STACKSIZE		(512 + 256 + RECV_STACK_FOR_WPS)	//Can be reduced
#endif
#endif //CONFIG_PLATFORM_8195A

#define XMIT_TASKLET_STACKSIZE	 256
#define RECV_TASKLET_STACKSIZE	(1024 + RECV_STACK_FOR_WPS)
#define SDIOXMIT_STACKSIZE	256


struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};

#define rtw_netdev_priv(netdev) ( ((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv )

#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) adapter->pnetdev->name
#define FUNC_NDEV_FMT "%s"
#define FUNC_NDEV_ARG(ndev) __func__
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, adapter->pnetdev->name

void save_and_cli(void);
void restore_flags(void);
void cli(void);

//----- ------------------------------------------------------------------
// Common Definition
//----- ------------------------------------------------------------------

#define __init
#define __exit
#define __devinit
#define __devexit

#define KERN_ERR
#define KERN_INFO
#define KERN_NOTICE

#define GFP_KERNEL			1
#define GFP_ATOMIC			1

#define SET_MODULE_OWNER(some_struct)	do { } while (0)
#define SET_NETDEV_DEV(dev, obj)	do { } while (0)
#define register_netdev(dev)		(0)
#define unregister_netdev(dev)		do { } while (0)
#define netif_queue_stopped(dev)	(0)
#define netif_wake_queue(dev)		do { } while (0)
#define printk				printf

#define DBG_ERR(fmt, args...)		printf("\n\r[%s] " fmt, __FUNCTION__, ## args)
#if WLAN_INTF_DBG
#define DBG_TRACE(fmt, args...)		printf("\n\r[%s] " fmt, __FUNCTION__, ## args)
#define DBG_INFO(fmt, args...)		printf("\n\r[%s] " fmt, __FUNCTION__, ## args)
#else
#define DBG_TRACE(fmt, args...)
#define DBG_INFO(fmt, args...)
#endif
#define HALT()				do { cli(); for(;;);} while(0)
#define ASSERT(x)			do { \
						if((x) == 0) \
							printf("\n\rAssert(" #x ") failed on line %d in file %s", __LINE__, __FILE__); \
						HALT(); \
					} while(0)

#undef DBG_ASSERT
#define DBG_ASSERT(x, msg)		do { \
						if((x) == 0) \
							printf("\n\r%s, Assert(" #x ") failed on line %d in file %s", msg, __LINE__, __FILE__); \
					} while(0)

//----- ------------------------------------------------------------------
// Atomic Operation
//----- ------------------------------------------------------------------
#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)	// for 8195A, it is defined in ..system../basic_types.h
typedef struct { volatile int counter; } atomic_t;
#endif


/*
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_read(v)  ((v)->counter)

/*
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_set(v,i) ((v)->counter = (i))

 /*
  *      These inlines deal with timer wrapping correctly. You are 
  *      strongly encouraged to use them
  *      1. Because people otherwise forget
  *      2. Because if the timer wrap changes in future you wont have to
  *         alter your driver code.
  *
  * time_after(a,b) returns true if the time a is after time b.
  *
  * Do this with "<0" and ">=0" to only test the sign of the result. A
  * good compiler would generate better code (and a really good compiler
  * wouldn't care). Gcc is currently neither.
  */
 #define time_after(a,b)	((long)(b) - (long)(a) < 0)
 #define time_before(a,b)	time_after(b,a)
  
 #define time_after_eq(a,b)	((long)(a) - (long)(b) >= 0)
 #define time_before_eq(a,b)	time_after_eq(b,a)


extern void	rtw_init_listhead(_list *list);
extern u32	rtw_is_list_empty(_list *phead);
extern void	rtw_list_insert_head(_list *plist, _list *phead);
extern void	rtw_list_insert_tail(_list *plist, _list *phead);
extern void	rtw_list_delete(_list *plist);

#endif /* _FREERTOS_SERVICE_H_ */
