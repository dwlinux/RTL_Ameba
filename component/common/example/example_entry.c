/******************************************************************************
 *
 * Copyright(c) 2007 - 2015 Realtek Corporation. All rights reserved.
 *
 *
 ******************************************************************************/
#include <platform_opts.h>
   
#if CONFIG_EXAMPLE_MDNS
#include <mdns/example_mdns.h>
#endif

#if CONFIG_EXAMPLE_MCAST
#include <mcast/example_mcast.h>
#endif

#if CONFIG_EXAMPLE_GOOGLE_NEST
#include <googlenest/example_google.h>  
#define FromDevice            1
#define ToDevice     		2 
#define TYPE         "ToDevice"
#endif   

#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#include <wlan_fast_connect/example_wlan_fast_connect.h>
#endif

/*
	Preprocessor of example
*/
void pre_example_entry(void)
{
#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
	example_wlan_fast_connect();
#endif

#if CONFIG_EXAMPLE_UART_ADAPTER
	example_uart_adapter_init();
#endif
}

/*
  	All of the examples are disabled by default for code size consideration
   	The configuration is enabled in platform_opts.h
*/
void example_entry(void)
{
#if (CONFIG_EXAMPLE_MDNS && !CONFIG_EXAMPLE_UART_ADAPTER) 
	example_mdns();
#endif

#if CONFIG_EXAMPLE_MCAST
	example_mcast();
#endif

#if CONFIG_EXAMPLE_GOOGLE_NEST
	example_google(TYPE);
#endif
}
