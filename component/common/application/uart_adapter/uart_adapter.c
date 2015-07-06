#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#include <lwip/sockets.h>
#include <lwip/raw.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip_netconf.h>

#include <dhcp/dhcps.h>

#include "stdlib.h"
#include "string.h"
#include <stdint.h>
#include "autoconf.h"
#include "PinNames.h"

#include "hal_platform.h"
#include "serial_api.h"
#include "serial_ex_api.h"
#include "lwip/api.h"
#include "uart_adapter.h"
#include "wifi_conf.h"
#include "wifi_util.h"
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "osdep_service.h"
//#include "utils/common.h"
#include <lwip/netif.h>

#include "flash_api.h"
#include <mDNS/mDNS.h>

#include <example_wlan_fast_connect.h>
/***********************************************************************
 *                                                            Macros                                                                    *
 ***********************************************************************/
#define   UA_TCP_SERVER_FD_NUM	1
#define   UA_TCP_CLIENT_FD_NUM	       1

#define 	UA_UART_RECV_BUFFER_LEN	8192
#define 	UA_UART_FRAME_LEN	       1400
#define   UA_UART_MAX_DELAY_TIME   100

#define UA_DATA_SOCKET_PORT           5001
#define UA_CONTROL_SOCKET_PORT	 6001

//in 8711am,there are 1 uart 
#define UA_MAX_UART_NUM   1

#define UA_GPIO_LED_PIN        PC_5
#define UA_GPIO_IRQ_PIN        PC_4

#define UA_CONTROL_PREFIX     "AMEBA_UART"

/***********************************************************************
 *                                                Variables Declarations                                                           *      
 ***********************************************************************/
unsigned char ua_work_thread_terminate = 0;
unsigned char ua_work_thread_suspend = 0;

int ua_tcp_data_fd_list[UA_TCP_SERVER_FD_NUM] = {-1};  
int ua_tcp_control_fd_list[UA_TCP_CLIENT_FD_NUM] = {-1};  
int ua_tcp_data_client_fd = -1; 

int ua_tcp_data_server_listen_fd = -1;  
int ua_tcp_control_server_listen_fd = -1;  

char ua_tcp_server_ip[16];

char ua_uart_recv_buf[UA_UART_RECV_BUFFER_LEN] = {0};
int ua_uart_recv_bytes = 0;
static int ua_pread = 0, ua_pwrite= 0;
static char ua_overlap = 0;

static volatile char ua_rcv_ch=0;
unsigned int ua_tick_last_update = 0;
unsigned int ua_tick_current = 0;

xSemaphoreHandle ua_uart_action_sema = NULL;
xSemaphoreHandle ua_tcp_sema = NULL;
xSemaphoreHandle ua_work_thread_sema = NULL;
xSemaphoreHandle ua_main_sema = NULL;

int ua_gpio_irq_happen = 0;
//int ua_gpio_irq_happen = 0;

long irq_rx_cnt = 0;
long tcp_tx_cnt = 0;
long irq_tx_cnt = 0;
long irq_miss_cnt = 0;

int ua_debug_print_en = 0;
int ua_tcp_send_flag = 0;

serial_t ua_uart_config[UA_MAX_UART_NUM];	//gloable uart_config
serial_t ua_sobj;

static gpio_t gpio_led;
static gpio_irq_t gpio_btn;

struct ua_tcp_rx_buffer
{
	char data[UA_UART_FRAME_LEN];
	int data_len;
};
sys_mbox_t mbox_for_uart_tx;

rtw_wifi_setting_t ua_wifi_setting = {RTW_MODE_NONE, {0}, 0, RTW_SECURITY_OPEN, {0}};

/************************************************************************
 *                                                  extern variables                                                                       *
 ************************************************************************/
extern struct netif xnetif[NET_IF_NUM];

//static int is_fixed_channel;
//extern int fixed_channel_num;
//extern unsigned char g_ssid[32];
//extern int g_ssid_len;
//extern unsigned char g_bssid[];
//extern  unsigned char g_security_mode;
//extern int simple_config_result;
//extern unsigned int simple_config_cmd_start_time;
enum ua_sc_result {
	SC_ERROR = -1,	/* default error code*/
	SC_NO_CONTROLLER_FOUND = 1, /* cannot get sta(controller) in the air which starts a simple config session */
	SC_CONTROLLER_INFO_PARSE_FAIL, /* cannot parse the sta's info  */
	SC_TARGET_CHANNEL_SCAN_FAIL, /* cannot scan the target channel */
	SC_JOIN_BSS_FAIL, /* fail to connect to target ap */
	SC_DHCP_FAIL, /* fail to get ip address from target ap */
	 /* fail to create udp socket to send info to controller. note that client isolation
		must be turned off in ap. we cannot know if ap has configured this */
	SC_UDP_SOCKET_CREATE_FAIL,
	SC_SUCCESS,	/* default success code */
};

//typedef int (*wlan_init_done_ptr)(void);
extern unsigned char psk_essid[NET_IF_NUM][NDIS_802_11_LENGTH_SSID+4];
extern unsigned char psk_passphrase[NET_IF_NUM][IW_PASSPHRASE_MAX_SIZE + 1];
extern unsigned char wpa_global_PSK[NET_IF_NUM][A_SHA_DIGEST_LEN * 2];

extern wlan_init_done_ptr p_wlan_uart_adapter_callback;

/************************************************************************
 *                                                  extern funtions                                                                       *
 ************************************************************************/
#if CONFIG_INCLUDE_SIMPLE_CONFIG
//extern void simple_config_callback(unsigned char *buf, unsigned int len, void* userdata);
//extern int SC_connect_to_AP(void);
//extern int SC_send_simple_config_ack(void);
//extern void SC_check_and_show_connection_info(void);
extern int simple_config_test(void);
//extern void print_simple_config_result(enum sc_result sc_code);
extern int init_test_data(char *custom_pin_code);
extern void deinit_test_data(void);
extern void filter_add_enable();
extern void remove_filter();
//extern void rtk_restart_simple_config(void);
//extern int promisc_fixed_channel_num(void * fixed_bssid, u8 *ssid, int * ssid_length);
//extern int promisc_get_fixed_channel(void * fixed_bssid, u8 * ssid, int * ssid_length);
//extern void get_connection_info_from_profile(rtw_security_t security_mode, rtw_network_info_t *wifi);
extern void wifi_enter_promisc_mode();
#endif
/*************************************************************************
*                                                   uart releated                                                                         *
*************************************************************************/
#define UA_PRINT_DATA(_HexData, _HexDataLen)			\
			if(UA_DEBUG_LEVEL == UA_DEBUG)	\
			{									\
				int __i;								\
				u8	*ptr = (u8 *)_HexData;				\
				printf("--------Len=%d\n\r", _HexDataLen);						\
				for( __i=0; __i<(int)_HexDataLen; __i++ )				\
				{								\
					printf("%02X%s", ptr[__i], (((__i + 1) % 4) == 0)?"  ":" ");	\
					if (((__i + 1) % 16) == 0)	printf("\n\r");			\
				}								\
				printf("\n\r");							\
			}

#define ____________UART__RELATED____________________
static void uartadapter_uart_irq(uint32_t id, SerialIrq event)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if(event == RxIrq) {		
		ua_rcv_ch = serial_getc(&ua_sobj);
		ua_uart_recv_buf[ua_pwrite] = ua_rcv_ch;
		ua_pwrite++;	//point to uart data recved
		xSemaphoreGiveFromISR( ua_uart_action_sema, &xHigherPriorityTaskWoken );	//up action semaphore 					
		
		if(ua_pwrite > (UA_UART_RECV_BUFFER_LEN -1)){	//restart from  head if  reach tail
			ua_pwrite = 0;
			ua_overlap = 1;		
		}
		
		if(ua_overlap && (ua_pwrite - 1) >= ua_pread ){
 		       //ua_printf(UA_ERROR, "IRQ missing data %d byte!", ua_pread - ua_pwrite + 1);
 		       irq_miss_cnt ++;
			ua_pread = ua_pwrite;		//if pwrite overhead pread ,pread is always flow rwrite 					
		}
		ua_tick_last_update =  xTaskGetTickCountFromISR();	// update tick everytime recved data
		irq_rx_cnt ++;
	}
}

static int uartadapter_uart_recv_data(void)
{
	//int ret = 0;
	int uart_recv_len = 0;

	ua_tick_current = xTaskGetTickCount();
	while((ua_tick_current -ua_tick_last_update) < (UA_UART_MAX_DELAY_TIME/portTICK_RATE_MS) 
	|| ua_tick_current <= ua_tick_last_update){
        	if(!ua_overlap){
        		uart_recv_len = ua_pwrite - ua_pread;
        	}else{
        		uart_recv_len = (UA_UART_RECV_BUFFER_LEN - ua_pread) + ua_pwrite;
        	}

        	if(uart_recv_len >= UA_UART_FRAME_LEN){      	
			return 2;
		}
		ua_tick_current = xTaskGetTickCount();		
	}     

	return 1;
}

int uartadapter_uart_read(void *read_buf, size_t size)
{
	/*the same as socket*/
	int ret = 0;
	int read_bytes;
	int pread_local,pwrite_local;
	char *ptr;
	ua_printf(UA_DEBUG, "==>uart adapter read uart");
	if(!size || !read_buf){
		ua_printf(UA_ERROR, "inpua error,size should not be null");
		ret = -1;
		return ret;
	}
	
	pread_local = ua_pread;
	pwrite_local = ua_pwrite;
	ptr = (char *)read_buf;
	
	/*calculate how much data not read */
	if(!ua_overlap){
		ua_uart_recv_bytes = pwrite_local - pread_local;
	}else{
		ua_uart_recv_bytes = (UA_UART_RECV_BUFFER_LEN - pread_local) + pwrite_local;
	}
	
	/*decide how much data shoule copy to application*/
	if(size >=  ua_uart_recv_bytes ){
		read_bytes = ua_uart_recv_bytes;
		ret = ua_uart_recv_bytes;
	}else{
		read_bytes = size;
		ret = size;
	}	

	if(!ua_overlap){
		memcpy(ptr, (ua_uart_recv_buf+ pread_local), read_bytes );
	}else {
		ua_printf(UA_DEBUG, "uart recv buf is write overlap!!");
		if((pread_local + read_bytes) > UA_UART_RECV_BUFFER_LEN){
			memcpy(ptr,(ua_uart_recv_buf+ pread_local),(UA_UART_RECV_BUFFER_LEN-pread_local));
			memcpy(ptr+(UA_UART_RECV_BUFFER_LEN-pread_local),ua_uart_recv_buf,read_bytes-(UA_UART_RECV_BUFFER_LEN- pread_local));
		}else{
			memcpy(ptr,(ua_uart_recv_buf+ pread_local),read_bytes);
		}
	}
	
	ua_uart_recv_bytes = 0;
	if((pread_local + read_bytes) >= UA_UART_RECV_BUFFER_LEN){		//update pread
		ua_pread = (pread_local + read_bytes) - UA_UART_RECV_BUFFER_LEN;
		ua_overlap = 0;		//clean overlap flags
	}else{
		ua_pread = pread_local + read_bytes;		
	}
	
	return ret;
}


int uartadapter_uart_write(char *pbuf, size_t size)
{
	/*the same as socket*/
	int ret = 0;
	//int cnt = 0;

	if(!size || !pbuf){
		//ua_printf(UA_ERROR, "inpua error,please check!");
		ret = -1;
		return ret;
	}	

#if 1
	do{	
    		ret = serial_send_stream_dma(&ua_sobj, pbuf, size);
    	}while(ret != HAL_OK);   	
#else
	while (size){
		serial_putc(&ua_sobj, *pbuf);
		//printf("uart write %d \n", *pbuf);		
       	size--;
		pbuf++;
    	}
#endif
	return ret;
}

static void uartadapter_uart_rx_handle(void* param)
{
	char	*rxbuf = NULL;
	int ret =0;
	int read_len = 0;

	rxbuf = pvPortMalloc(UA_UART_FRAME_LEN);
	if(NULL == rxbuf){
		ua_printf(UA_ERROR, "TCP: Allocate rx buffer failed.\n");
		return;
	}	
	
	while(xSemaphoreTake(ua_uart_action_sema, portMAX_DELAY) == pdTRUE){	
		if(ua_debug_print_en)	{
		      //ua_printf(UA_INFO, "uartadapter_uart_thread_handle loop!");
		}
		
		ret = uartadapter_uart_recv_data();		
		if(ret == -1){
			ua_printf(UA_ERROR, "uart recv data error!");
		}else{			
           		read_len = uartadapter_uart_read(rxbuf, UA_UART_FRAME_LEN);
            		if(read_len > 0){	           			
                      	uartadapter_tcpsend(rxbuf, read_len, 0);	                           	
            		}else if(read_len < 0){
                     	ua_printf(UA_ERROR, "tcp send read_len = %d", read_len);            		
            		}
		}	
	}
}

static void uartadapter_uart_tx_handle(void* param)
{
	struct ua_tcp_rx_buffer *uart_tx;
	//int ret;
	
	while(1)
	{
		sys_mbox_fetch(&mbox_for_uart_tx, (void *)&uart_tx);
	      	uartadapter_uart_write(uart_tx->data, uart_tx->data_len);
		vPortFree(uart_tx);
	}
}

int uartadapter_uart_open(char *uartname, ua_uart_set_str *puartpara)
{
	PinName uart_tx,uart_rx;

	if(!strcmp("uart0", uartname)){
		uart_tx = PA_7;
		uart_rx = PA_6;
		ua_uart_config[0].hal_uart_adp.BaudRate     = puartpara->BaudRate;
		ua_uart_config[0].hal_uart_adp.FlowControl  = puartpara->FlowControl;
		ua_uart_config[0].hal_uart_adp.WordLen      = puartpara->number;
		ua_uart_config[0].hal_uart_adp.Parity           = puartpara->parity;
		ua_uart_config[0].hal_uart_adp.StopBit         = puartpara->StopBits;
	}else{
		ua_printf(UA_ERROR, "please check uart name!");
		return RTW_ERROR;
	}

	/*initial uart */
	serial_init(&ua_sobj,uart_tx,uart_rx);
    	serial_baud(&ua_sobj,puartpara->BaudRate);
    	serial_format(&ua_sobj, puartpara->number, (SerialParity)puartpara->parity, puartpara->StopBits);
	
	/*uart irq handle*/
	serial_irq_handler(&ua_sobj, uartadapter_uart_irq, (uint32_t)&ua_sobj);
	serial_irq_set(&ua_sobj, RxIrq, 1);	
	serial_irq_set(&ua_sobj, TxIrq, 1);	

	return 0;
}

int uartadapter_uart_baud(int baud_rate)
{
	int ret = 0;
    	serial_baud(&ua_sobj, baud_rate);
	
	return ret;
}

int uartadapter_uart_para(int word_len, int parity, int stop_bits)
{
	int ret = 0;

    	serial_format(&ua_sobj, word_len, (SerialParity)parity, stop_bits);
	
	return ret;
}

int uartadapter_uart_getnum(char *uartname)
{
	/*only uart 0 can used*/
	strcpy(uartname, "uart0\\");
	return UA_MAX_UART_NUM;  
}

int uartadapter_uart_getpara(char *uart_name, ua_uart_get_str *uart_para)
{
	int ret = 0;

	/*get the uart para according to uart_name*/
	if(!strcmp("uart0", uart_name)){
		uart_para->BaudRate       =ua_uart_config[0].hal_uart_adp.BaudRate;
		uart_para->FlowControl   =ua_uart_config[0].hal_uart_adp.FlowControl;
		uart_para->number         = ua_uart_config[0].hal_uart_adp.WordLen;
		uart_para->parity            = ua_uart_config[0].hal_uart_adp.Parity;
		uart_para->StopBits        = ua_uart_config[0].hal_uart_adp.StopBit;
	}else{
		ua_printf(UA_ERROR, "please check uart name!");
		return -1;
	}

	return ret;
}
void uartadapter_uart_init()
{
	ua_uart_set_str uartset;
	ua_uart_get_str uartget;
	int uartnum;
	char uarttest[]="uart0";
	char uartname[32] = {0};
    	
	uartset.BaudRate = 9600;
	uartset.number = 8;
	uartset.StopBits = 0;
	uartset.FlowControl = 0;
	uartset.parity = 0;
	strcpy(uartset.UartName,uarttest);

	uartnum = uartadapter_uart_getnum(uartname);
	ua_printf(UA_DEBUG, "there is %d uart on this platform",uartnum);
	ua_printf(UA_DEBUG, "uartname = %s",uartname);

	uartadapter_uart_open("uart0", &uartset);

	if(uartadapter_uart_getpara("uart0", &uartget))
		ua_printf(UA_ERROR, "get uart failed!");
	else
		ua_printf(UA_DEBUG,"uart pata:\r\n"\
			"uart->BaudRate = %d\r\n"\
			"uart->number = %d\r\n"\
			"uart->FlowControl = %d\r\n"\
			"uart->parity = %d\r\n"\
			"uart->StopBits = %d\r\n"\
			"\r\n",\
			uartget.BaudRate,\
			uartget.number,\
			uartget.FlowControl,\
			uartget.parity,\
			uartget.StopBits\
			);
}

#define _________FLASH___RELATED________________________

int uartadapter_flashread(int flashadd, char *pbuf, int len)
{
	int ret = 0;
	flash_t flash;	

	if( len == 0){
		ua_printf(UA_ERROR, "inpua error,data length should not be null!");
		ret = -1;
		return ret;
	}else	//as 8711am only canbe r/w in words.so make len is 4-bytes aligmented.
		len += 4 - ((len%4)==0 ? 4 : (len%4));
	
	while(len){
		if(flash_read_word(&flash, flashadd, (unsigned int *)pbuf) !=1 ){
			ua_printf(UA_ERROR, "read flash error!");
			ret = -1;
			return ret;
		}
		len -= 4;
		pbuf += 4;
		flashadd += 4;
	}

	return len;
}

int uartadapter_flashwrite(int flashadd, char *pbuf, int len)
{	
	int ret = 0;
	flash_t flash;	

	if( len == 0){
		ua_printf(UA_ERROR, "inpua error,data length should not be null!");
		ret = -1;
		return ret;
	}
	else	//as 8711am only canbe r/w in words.so make len is 4-bytes aligmented.
		len += 4 - ((len%4)==0 ? 4 : (len%4));
	
	while(len){
		if(flash_write_word(&flash, flashadd, *(unsigned int *)pbuf) !=1 ){
			ua_printf(UA_ERROR, "write flash error!");
			ret = -1;
			return ret;
		}
		len -= 4;
		pbuf += 4;
		flashadd += 4;
	}

	return ret;
}

int uartadapter_flasherase(int flashadd, int erase_bytelen)
{
	int ret = 0;
    	flash_t flash;
	
	flash_erase_sector(&flash, flashadd);

	return ret;
}

#define _________GPIO___RELATED________________________
void uartadapter_systemreload(void)
{
	// Cortex-M3 SCB->AIRCR
	HAL_WRITE32(0xE000ED00, 0x0C, (0x5FA << 16) |                             // VECTKEY
            (HAL_READ32(0xE000ED00, 0x0C) & (7 << 8))  |   				// PRIGROUP
                                (1 << 2));                                  						// SYSRESETREQ	
}
void uartadapter_gpio_irq (uint32_t id, gpio_irq_event event)
{
	//int ret = 0;
	//int address = FAST_RECONNECT_DATA;	

       ua_printf(UA_DEBUG, "GPIO push button!!");

	
       ua_gpio_irq_happen = 1;
	xSemaphoreGive(ua_main_sema);


}

void uartadapter_gpio_init()
{
    	gpio_init(&gpio_led, UA_GPIO_IRQ_PIN);
    	gpio_dir(&gpio_led, PIN_INPUT);    // Direction: Output
    	gpio_mode(&gpio_led, PullNone);     // No pull
    	
    	gpio_irq_init(&gpio_btn, UA_GPIO_IRQ_PIN, uartadapter_gpio_irq, (uint32_t)(&gpio_led));
    	gpio_irq_set(&gpio_btn, IRQ_FALL, 1);   // Falling Edge Trigger
    	gpio_irq_enable(&gpio_btn);
}


#define _________CONTROL__DATA__RELATED________________________
int uartadapter_strncmp(char *cs, char *ct, size_t count)
{
    	unsigned char c1, c2;

    	while (count) {
       	c1 = *cs++;
        	c2 = *ct++;
        	if (c1 != c2)
           		return c1 < c2 ? -1 : 1;
        	if (!c1)
            		break;          
        	count--;
    	}
    
    	return 0;
}
int uartadapter_control_set_req_handle(u8 *pbuf, u32 sz)
{
	u8 *p = pbuf;
	u8 type = 0, len = 0;

       ua_printf(UA_DEBUG, "\n===>uartadapter_control_set_req_handle()");
	UA_PRINT_DATA(pbuf, sz);

	while(p < (pbuf+sz)){
		type = *p++;
		len = *p++;
		ua_printf(UA_DEBUG, "type=%d len=%d\n", type, len);
		switch(type)
		{
			case UART_CTRL_TYPE_BAUD_RATE:
				ua_uart_config[0].hal_uart_adp.BaudRate = *(u32 *)p;
       			ua_printf(UA_INFO, "SET UART BAUD_RATE to %d.\n", ua_uart_config[0].hal_uart_adp.BaudRate);
				serial_baud(&ua_sobj, ua_uart_config[0].hal_uart_adp.BaudRate);
				break;
			case UART_CTRL_TYPE_WORD_LEN:
				ua_uart_config[0].hal_uart_adp.WordLen = *p;
       			ua_printf(UA_INFO, "SET UART WORD_LEN to %d.\n", ua_uart_config[0].hal_uart_adp.WordLen);
				serial_format(&ua_sobj, 
							ua_uart_config[0].hal_uart_adp.WordLen, 
							(SerialParity)ua_uart_config[0].hal_uart_adp.Parity,
							ua_uart_config[0].hal_uart_adp.StopBit);
				break;
			case UART_CTRL_TYPE_PARITY:
				ua_uart_config[0].hal_uart_adp.Parity = *p;
       			ua_printf(UA_INFO, "SET UART PARITY to %d.\n", ua_uart_config[0].hal_uart_adp.Parity);
				serial_format(&ua_sobj, 
							ua_uart_config[0].hal_uart_adp.WordLen, 
							(SerialParity)ua_uart_config[0].hal_uart_adp.Parity,
							ua_uart_config[0].hal_uart_adp.StopBit);
				break;
			case UART_CTRL_TYPE_STOP_BIT:
				ua_uart_config[0].hal_uart_adp.StopBit = *p;
       			ua_printf(UA_INFO, "SET UART STOP_BIT to %d.\n", ua_uart_config[0].hal_uart_adp.StopBit);
				serial_format(&ua_sobj, 
							ua_uart_config[0].hal_uart_adp.WordLen, 
							(SerialParity)ua_uart_config[0].hal_uart_adp.Parity,
							ua_uart_config[0].hal_uart_adp.StopBit);
				break;
			case UART_CTRL_TYPE_FLOW_CTRL:
				ua_uart_config[0].hal_uart_adp.FlowControl = *p;
       			ua_printf(UA_INFO, "SET UART FLOW_CTRL to %d.\n", ua_uart_config[0].hal_uart_adp.FlowControl);
       			ua_printf(UA_INFO, "SET UART FLOW_CTRL not support now.\n");
				//TODO
				break;
		}
		p += len;
	}
	return 0;
}

int uartadapter_control_get_req_handle(u8 type, u8 *prsp, u32 *sz)
{
	u8 *p = prsp;

 	ua_printf(UA_DEBUG, "===>uartadapter_control_get_req_handle()");
	sprintf((char *)p, UA_CONTROL_PREFIX);
	p += strlen(UA_CONTROL_PREFIX);
	*p++ = UART_CTRL_MODE_GET_RSP;

	if(type & UART_CTRL_TYPE_BAUD_RATE){
		*p++ = UART_CTRL_TYPE_BAUD_RATE;
		*p++ = 4;
		*(u32*)p = ua_uart_config[0].hal_uart_adp.BaudRate;
		p += 4;
	}
	if(type & UART_CTRL_TYPE_WORD_LEN){
		*p++ = UART_CTRL_TYPE_WORD_LEN;
		*p++ = 1;
		*p = ua_uart_config[0].hal_uart_adp.WordLen;
		p += 1;
	}
	if(type & UART_CTRL_TYPE_PARITY){
		*p++ = UART_CTRL_TYPE_PARITY;
		*p++ = 1;
		*p = ua_uart_config[0].hal_uart_adp.Parity;
		p += 1;
	}
	if(type & UART_CTRL_TYPE_STOP_BIT){
		*p++ = UART_CTRL_TYPE_STOP_BIT;
		*p++ = 1;
		*p = ua_uart_config[0].hal_uart_adp.StopBit;
		p += 1;
	}
	if(type & UART_CTRL_TYPE_FLOW_CTRL){
		*p++ = UART_CTRL_TYPE_FLOW_CTRL;
		*p++ = 1;
		*p = ua_uart_config[0].hal_uart_adp.FlowControl;
		p += 1;
	}
	*sz = p - prsp;

	UA_PRINT_DATA(prsp, *sz);
	return 0;		
}

int uartadapter_control_process(int fd, char *pbuf, size_t size)
{
	/*the same as socket*/
	int ret = 0;

	if(!size || !pbuf){
		//ua_printf(UA_ERROR, "control data input error,please check!");
		ret = -1;
		return ret;
	}

	UA_PRINT_DATA(pbuf, size);

       if(uartadapter_strncmp(pbuf, UA_CONTROL_PREFIX, 10) != 0)
       {
       	ua_printf(UA_ERROR, "control data prefix wrong!");
		return -1;
	}
	else
	{
		u8 *p = (u8*)pbuf + strlen(UA_CONTROL_PREFIX);
		u8 mode = *p++;
		switch(mode)
		{
			case UART_CTRL_MODE_SET_REQ: //AMEBA_UART-MODE-TYPE-LEN-VAL-TYPE-LEN-VAL...
			{
				char rsp[32] = {0};  //AMEBA_UART-MODE
				u32 sz = strlen(UA_CONTROL_PREFIX);
				uartadapter_control_set_req_handle(p, (size - strlen(UA_CONTROL_PREFIX)));
				sprintf(rsp, UA_CONTROL_PREFIX);
				*(rsp + sz) = UART_CTRL_MODE_SET_RSP;
				sz ++;
				sprintf(rsp + sz, "\n");
				sz ++;				
				uartadapter_tcpsend(rsp, sz, 1);
				break;
			}
			case UART_CTRL_MODE_GET_REQ: //AMEBA_UART-MODE-TYPE
			{
				char rsp[128] = {0};
				u32 sz = 0;
				u8 type = *p;
				uartadapter_control_get_req_handle(type, (u8*)rsp, &sz);
				sprintf(rsp + sz, "\n");
				sz ++;				
				uartadapter_tcpsend(rsp, sz, 1);
				break;
			}
			default:
				ua_printf(UA_ERROR, UA_CONTROL_PREFIX": Mode (%d) not support!", mode);
				break;
		}
	}
	return 0;
}

#define _________TCP__RELATED________________________
int uartadapter_tcpclient(const char *host_ip, unsigned short usPort)
{
       int	iAddrSize;
       int	iSockFD = -1;
       int	iStatus;
       //int	enable = 1;
       struct sockaddr_in  sAddr;

	FD_ZERO(&sAddr);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(usPort);
	sAddr.sin_addr.s_addr = inet_addr(host_ip);

	iAddrSize = sizeof(struct sockaddr_in);

	iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if( iSockFD < 0 ) {
		ua_printf(UA_ERROR, "TCP ERROR: create tcp client socket fd error!");
		return 0;
	}

	ua_printf(UA_DEBUG, "TCP: ServerIP=%s port=%d.", host_ip, usPort);
	ua_printf(UA_DEBUG, "TCP: Create socket %d.", iSockFD);
	// connecting to TCP server
	iStatus = connect(iSockFD, (struct sockaddr *)&sAddr, iAddrSize);
	if (iStatus < 0) {
		ua_printf(UA_ERROR, "TCP ERROR: tcp client connect server error! ");
		goto Exit;
	}

#if 0
	iStatus = setsockopt(iSockFD, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
	if (iStatus < 0) {
		printf("\n\rTCP ERROR: tcp client socket set opt error! ");
		goto Exit;
	}
#endif	    
       
	ua_printf(UA_INFO, "TCP: Connect server successfully.");
#if 1
	ua_work_thread_suspend = 1;
	vTaskDelay(1500);
	if(ua_tcp_data_fd_list[0] != -1){
		ua_printf(UA_INFO, "TCP: Close old data client fd %d.", ua_tcp_data_fd_list[0]);		
		close(ua_tcp_data_fd_list[0]);
	}	
	ua_tcp_data_fd_list[0] = iSockFD;	
	ua_printf(UA_INFO, "connect new data socket %d successfully.", iSockFD);								
	ua_work_thread_suspend = 0;
	xSemaphoreGive(ua_work_thread_sema);
#else
	ua_tcp_data_client_fd = iSockFD;	
#endif

	return 0;

Exit:
	//ua_printf(UA_ERROR, "TCP client fd list exceed.");
	close(iSockFD);
	return 0;
}


int uartadapter_tcpserver(unsigned short usPort, u8 isctrl)
{
	struct sockaddr_in  sLocalAddr;	
	int			iAddrSize;
	int			iSockFD;
	int			iStatus;

	iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if( iSockFD < 0 ) {
             ua_printf(UA_ERROR, "create server_socket error!");  		
		goto Exit;
	}

	ua_printf(UA_DEBUG, "TCP: Create Tcp server socket %d", iSockFD);

	//filling the TCP server socket address
	memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
	sLocalAddr.sin_family      = AF_INET;
	sLocalAddr.sin_len         = sizeof(sLocalAddr);
	sLocalAddr.sin_port        = htons(usPort);
	sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iAddrSize = sizeof(sLocalAddr);

	iStatus = bind(iSockFD, (struct sockaddr *)&sLocalAddr, iAddrSize);
	if( iStatus < 0 ) {
		ua_printf(UA_ERROR, "bind tcp server socket fd error! ");
		goto Exit;
	}
	ua_printf(UA_DEBUG, "TCP: Bind successfully.");

	iStatus = listen(iSockFD, 10);
	if( iStatus != 0 ) {
		ua_printf(UA_ERROR, "listen tcp server socket fd error!");
		goto Exit;
	}
	ua_printf(UA_INFO, "TCP Server: Listen on port %d", usPort);

	if(isctrl)
		ua_tcp_control_server_listen_fd = iSockFD;
	else
		ua_tcp_data_server_listen_fd = iSockFD;

	return 0;
	
Exit:	 
	close(iSockFD);
       ua_printf(UA_INFO, "Tcp server listen on port %d closed!", usPort);
	return 0;
}

void uartadapter_tcp_data_server_handler(void *param)
{
	unsigned short port = UA_DATA_SOCKET_PORT;
	
	ua_printf(UA_DEBUG, "Uart Adapter: Start Tcp Data Server!");
       uartadapter_tcpserver(port, 0);
	
#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)
	ua_printf(UA_DEBUG, "Min available stack size of %s = %d * %d bytes\n\r", __FUNCTION__, uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
#endif
	ua_printf(UA_DEBUG, "TCP: Tcp data server stopped!");
	vTaskDelete(NULL);
}

void uartadapter_tcp_data_client_handler(void *param)
{
	unsigned short port = UA_DATA_SOCKET_PORT;
	
	ua_printf(UA_DEBUG, "Uart Adapter: Start Tcp Data Client!");
       uartadapter_tcpclient(ua_tcp_server_ip, port);
	
#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)
	ua_printf(UA_DEBUG, "Min available stack size of %s = %d * %d bytes\n\r", __FUNCTION__, uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
#endif
	ua_printf(UA_DEBUG, "TCP: Tcp data client stopped!");
	vTaskDelete(NULL);
}

void uartadapter_tcp_control_server_handler(void *param)
{
	unsigned short port = UA_CONTROL_SOCKET_PORT;
	
	ua_printf(UA_DEBUG, "Uart Adapter: Start Tcp Control Server!");
       uartadapter_tcpserver(port, 1);
	
#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)
	ua_printf(UA_DEBUG, "Min available stack size of %s = %d * %d bytes", __FUNCTION__, uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
#endif
	ua_printf(UA_DEBUG, "TCP: Tcp control server stopped!");
	vTaskDelete(NULL);
}

void uartadapter_tcpsend(char *buffer, int size, u8 isctrl)
{    
	int                 iStatus;

	ua_tcp_send_flag = 1;
	
	if(!isctrl){
		if(ua_tcp_data_fd_list[0] != -1){	
      			//xSemaphoreTake(ua_tcp_sema, portMAX_DELAY);		
			iStatus = send(ua_tcp_data_fd_list[0], buffer, size, 0 );
       		//xSemaphoreGive(ua_tcp_sema);
			
			if( iStatus <= 0 ){
				ua_printf(UA_ERROR, "tcp data socket send data error!  iStatus:%d!", iStatus);
				//goto Exit;
			}else if(iStatus != size){
				ua_printf(UA_DEBUG, "uart tcp data socket send: size %d,  ret = %d!", size, iStatus);			
			}
				
			if(ua_debug_print_en){
				ua_printf(UA_INFO, "uart tcp data socket send %d bytes, ret %d!", size, iStatus);	
			}			
		}
	}
	else{
		if(ua_tcp_control_fd_list[0] != -1){ 		
      			//xSemaphoreTake(ua_tcp_sema, portMAX_DELAY);	
			iStatus = send(ua_tcp_control_fd_list[0], buffer, size, 0 );
      	 		//xSemaphoreGive(ua_tcp_sema);
			
			if( iStatus <= 0 ){
				ua_printf(UA_ERROR,"tcp control socket send data error!  iStatus:%d!", iStatus);
				goto Exit;
			}else if(iStatus != size){
				ua_printf(UA_DEBUG,"uart tcp control socket send: size %d,  ret = %d!", size, iStatus);			
			}
			
			if(ua_debug_print_en){
				ua_printf(UA_INFO,"uart tcp control socket send %d bytes, ret %d!", size, iStatus);	
			}				
		}
	}
	
       ua_tcp_send_flag = 0;
        
Exit:

	return;
}

void uartadapter_tcp_data_fd_handler(char *tcp_rxbuf)
{
     	int recv_len;
       struct ua_tcp_rx_buffer  *rx_buffer;
	rx_buffer = pvPortMalloc(sizeof(struct ua_tcp_rx_buffer));
	if(NULL == rx_buffer){
		ua_printf(UA_ERROR, "Allocate tcp data buffer failed.\n");
		return;
	}     
   	
	//xSemaphoreTake(ua_tcp_sema, portMAX_DELAY);
	recv_len = recv(ua_tcp_data_fd_list[0], tcp_rxbuf, UA_UART_FRAME_LEN, 0);
	//xSemaphoreGive(ua_tcp_sema);

	if(recv_len < 0){
		ua_printf(UA_ERROR, "Tcp Data Socket %d Recv Error", ua_tcp_data_fd_list[0]);
		//goto EXIT;
	}
	ua_printf(UA_DEBUG, "Tcp Data Socket %d Recv %d Data", ua_tcp_data_fd_list[0], recv_len);					

#if 1
	if(recv_len > 0){
		memcpy(rx_buffer->data, tcp_rxbuf, recv_len);
		rx_buffer->data_len = recv_len;
		sys_mbox_post(&mbox_for_uart_tx, (void *)rx_buffer);
	}else{
		vPortFree(rx_buffer);	
	}
#else
	uartadapter_uart_write(tcp_rxbuf, recv_len);
#endif

	tcp_tx_cnt += recv_len;
	
	return;
}

void uartadapter_tcp_control_fd_handler()
{
    	char tcp_rxbuf[UA_UART_FRAME_LEN];
     	int recv_len;
   	
	//xSemaphoreTake(ua_tcp_sema, portMAX_DELAY);
	recv_len = recv(ua_tcp_control_fd_list[0], tcp_rxbuf, UA_UART_FRAME_LEN, 0);  //MSG_DONTWAIT   MSG_WAITALL
	//xSemaphoreGive(ua_tcp_sema);

	if(recv_len<0){
		ua_printf(UA_ERROR, "Tcp Control Socket %d Recv Error", ua_tcp_control_fd_list[0]);
		//goto EXIT;              		      
	}   
	ua_printf(UA_DEBUG, "Tcp Control Socket %d Recv %d Data", ua_tcp_control_fd_list[0], recv_len);									

	uartadapter_control_process(ua_tcp_control_fd_list[0], (void*)tcp_rxbuf, recv_len);

	return;

}

void uartadapter_tcp_data_listen_fd_handler(int old_data_fd)
{
	struct sockaddr_in  sAddr;    
	int addrlen = sizeof(sAddr);
	
	ua_tcp_data_fd_list[0] = accept(ua_tcp_data_server_listen_fd, (struct sockaddr *)&sAddr, (socklen_t*)&addrlen);
	if( ua_tcp_data_fd_list[0] < 0 ) {
		ua_printf(UA_ERROR, "Accept tcp data client socket fd error");
		goto EXIT;
	} 	
	ua_printf(UA_INFO, "Accept new data socket %d on port %d successfully.", ua_tcp_data_fd_list[0], sAddr.sin_port);				       			
	if(old_data_fd != -1)
	{
		shutdown(old_data_fd, 2);	
		close(old_data_fd);
		ua_printf(UA_INFO, "Close old data socket %d.", old_data_fd);	
		old_data_fd = -1;
	}

	return;

EXIT:
	if(ua_tcp_data_server_listen_fd != -1){
     	 	close(ua_tcp_data_server_listen_fd);
     	 	ua_tcp_data_server_listen_fd = -1;    	 
	}			
}

void uartadapter_tcp_control_listen_fd_handler(int old_control_fd)
{
	struct sockaddr_in  sAddr;    
	int addrlen = sizeof(sAddr);
	
	ua_tcp_control_fd_list[0] = accept(ua_tcp_control_server_listen_fd, (struct sockaddr *)&sAddr, (socklen_t*)&addrlen);
	if( ua_tcp_control_fd_list[0] < 0 ) {
		ua_printf(UA_ERROR, "Accept tcp control client socket fd error");
		goto EXIT;
	} 		
	ua_printf(UA_INFO, "Accept new control socket %d on port %d successfully.", ua_tcp_control_fd_list[0], sAddr.sin_port);				       			       			
	if(old_control_fd != -1)
	{
		close(old_control_fd);
		ua_printf(UA_INFO, "Close old control socket %d.", old_control_fd);	
		old_control_fd = -1;
	}
	
	return;

EXIT:
	if(ua_tcp_data_server_listen_fd != -1){
     	 	close(ua_tcp_data_server_listen_fd);
     	 	ua_tcp_data_server_listen_fd = -1;    	 
	}			
}

void uartadapter_tcp_select(void *param)
{
	int max_fd;	
	struct timeval tv;
    	fd_set readfds;
   	int ret = 0;
    	char *tcp_rxbuf;
	tcp_rxbuf = pvPortMalloc(UA_UART_FRAME_LEN);
	if(NULL == tcp_rxbuf){
		printf("\n\rTCP: Allocate client buffer failed.\n");
		return;
	}
    	
	
	while(1){
		if(ua_work_thread_suspend){
			ua_printf(UA_DEBUG, "uart adapter test thread suspended!");
			if(xSemaphoreTake(ua_work_thread_sema, portMAX_DELAY) == TRUE)
				ua_printf(UA_DEBUG, "uart adapter test thread take semaphore!");	
		}		
		 
        	FD_ZERO(&readfds);
	 	max_fd = -1;

     	 	if(ua_tcp_data_fd_list[0] != -1){
       	 	FD_SET(ua_tcp_data_fd_list[0], &readfds);	
     			if(ua_tcp_data_fd_list[0] > max_fd)
     				max_fd = ua_tcp_data_fd_list[0];
     		}	 

	 	if(ua_tcp_control_fd_list[0] != -1){
	         	FD_SET(ua_tcp_control_fd_list[0], &readfds);		
			if(ua_tcp_control_fd_list[0] > max_fd)
				max_fd = ua_tcp_control_fd_list[0];				
		}	

	 	if(ua_tcp_control_server_listen_fd != -1){
	         	FD_SET(ua_tcp_control_server_listen_fd, &readfds);		
			if(ua_tcp_control_server_listen_fd > max_fd)
				max_fd = ua_tcp_control_server_listen_fd;				
		}	

	 	if(ua_tcp_data_server_listen_fd != -1){
	         	FD_SET(ua_tcp_data_server_listen_fd, &readfds);		
			if(ua_tcp_data_server_listen_fd > max_fd)
				max_fd = ua_tcp_data_server_listen_fd;				
		}			
	 
       	tv.tv_sec = 1;
       	tv.tv_usec = 0;

       	ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

      		if(ua_debug_print_en){
	       	ua_printf(UA_INFO, "uart adapter test select ret = %x",ret);	
        	}	 
        	if(ret > 0){         			
                  	if(ua_tcp_data_fd_list[0] != -1 && FD_ISSET(ua_tcp_data_fd_list[0], &readfds)){ 
				uartadapter_tcp_data_fd_handler(tcp_rxbuf);			
  			}
  			 					
                	if(ua_tcp_control_fd_list[0] != -1 && FD_ISSET(ua_tcp_control_fd_list[0], &readfds)){
				uartadapter_tcp_control_fd_handler();			
			}	

                  	if(ua_tcp_data_server_listen_fd != -1 && FD_ISSET(ua_tcp_data_server_listen_fd, &readfds)){ 
				uartadapter_tcp_data_listen_fd_handler(ua_tcp_data_fd_list[0]);
        		}

                  	if(ua_tcp_control_server_listen_fd != -1 && FD_ISSET(ua_tcp_control_server_listen_fd, &readfds)){ 
				uartadapter_tcp_control_listen_fd_handler(ua_tcp_control_fd_list[0]);
        		}        		 			
		}
	}

	//vTaskDelete(NULL);	
}


#define _________WIFI__RELATED________________________

int uartadapter_connect_wifi(rtw_network_info_t *p_wifi, uint32_t channel, uint8_t pscan_config)
{
	int retry = 3;
	rtw_wifi_setting_t setting;	
	int ret;
	while (1) {
		if(wifi_set_pscan_chan((uint8_t *)&channel, &pscan_config, 1) < 0){
			printf("\n\rERROR: wifi set partial scan channel fail");
			ret = SC_TARGET_CHANNEL_SCAN_FAIL;
			return ret;
		}

		ret = wifi_connect((char*)p_wifi->ssid.val,
					  p_wifi->security_type,
					  (char*)p_wifi->password,
					  p_wifi->ssid.len,
					  p_wifi->password_len,
					  p_wifi->key_id,
					  NULL);

		if (ret == RTW_SUCCESS) {
			ret = LwIP_DHCP(0, DHCP_START);	
			wifi_get_setting(WLAN0_NAME, &setting);
			wifi_show_setting(WLAN0_NAME, &setting);		
			if (ret == DHCP_ADDRESS_ASSIGNED)
				return SC_SUCCESS;
			else
				return SC_DHCP_FAIL;			
		}

		if (retry == 0) {
			ret = SC_JOIN_BSS_FAIL;
			break;
		}
		retry --;
	}

	return ret;
}
	

static int uartadapter_load_wifi_config()
{
	flash_t		flash;
	uint8_t		*data;
	uint32_t	channel;
	uint8_t     pscan_config;
	char key_id;	
	rtw_network_info_t wifi = {0};
	int ret = SC_SUCCESS;
	

	data = (uint8_t *)rtw_zmalloc(FAST_RECONNECT_DATA_LEN);
	flash_stream_read(&flash, FAST_RECONNECT_DATA, FAST_RECONNECT_DATA_LEN, (uint8_t *)data);
	if(*((uint32_t *) data) != ~0x0)
	{
	    ua_printf(UA_INFO, "AP Profile read from FLASH, try to connect");                     			
	    memcpy(psk_essid, (uint8_t *)data, NDIS_802_11_LENGTH_SSID + 4);
	    memcpy(psk_passphrase, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4, (IW_PASSPHRASE_MAX_SIZE + 1));
	    memcpy(wpa_global_PSK, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4 + (IW_PASSPHRASE_MAX_SIZE + 1), A_SHA_DIGEST_LEN * 2);
	    memcpy(&channel, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4 + (IW_PASSPHRASE_MAX_SIZE + 1) + A_SHA_DIGEST_LEN * 2, 4);
	    sprintf(&key_id,"%d",(channel >> 28));
	    channel &= 0xff;	    
	    pscan_config = PSCAN_ENABLE | PSCAN_FAST_SURVEY;
	    //set partial scan for entering to listen beacon quickly
	    //wifi_set_pscan_chan((uint8_t *)&channel, &pscan_config, 1);
	    
#ifdef CONFIG_AUTO_RECONNECT
	    wifi_set_autoreconnect(1);
#endif
	    //set wifi connect
	    wifi.ssid.len = (int)strlen((char*)psk_essid);
	    memcpy(wifi.ssid.val, psk_essid, wifi.ssid.len);
	    wifi.key_id = key_id;
	    
	    //open mode
	    if(!strlen((char*)psk_passphrase)){
		wifi.security_type = RTW_SECURITY_OPEN;
	    }
	    //wep mode
	    else if( strlen((char*)psk_passphrase) == 5 || strlen((char*)psk_passphrase) == 13){
		wifi.security_type = RTW_SECURITY_WEP_PSK;
		wifi.password = (unsigned char *)psk_passphrase;
		wifi.password_len = (int)strlen((char const *)psk_passphrase);
	    }
	    //WPA/WPA2
	    else{
		wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
		wifi.password = (unsigned char *)psk_passphrase;
		wifi.password_len = (int)strlen((char const *)psk_passphrase);
	    }
	    
	    ret = uartadapter_connect_wifi(&wifi, channel, pscan_config);

	    //print_simple_config_result((enum ua_sc_result)ret);

	    if(data)
	        rtw_mfree(data, FAST_RECONNECT_DATA_LEN);

	    if(ret == SC_SUCCESS)
	    	return RTW_SUCCESS;
	    else
	    	return RTW_ERROR;
	}else{
	    ua_printf(UA_INFO, "No AP Profile read from FLASH, start simple configure");                     			

	    if(data)
	        rtw_mfree(data, FAST_RECONNECT_DATA_LEN);

	    return RTW_ERROR;
	}
}

int uartadapter_simple_config(char *pin_code){
#if CONFIG_INCLUDE_SIMPLE_CONFIG
	int ret = SC_SUCCESS;
	wifi_enter_promisc_mode();
	if(init_test_data(pin_code) == 0){
		filter_add_enable();
		//uartadapter_simple_config_handler();
		ret = simple_config_test();
		//print_simple_config_result(ret);
		remove_filter();
		if(ret == SC_SUCCESS)
			return RTW_SUCCESS;
		else
			return RTW_ERROR;
	}else{
		return RTW_ERROR;
	}
#endif	
}

#define _________MDNS__RELATED________________________
static void uartadapter_mdns_thread(void *param)
{
	DNSServiceRef dnsServiceRef = NULL;
	DNSServiceRef dnsServiceRef2 = NULL;	
	//TXTRecordRef txtRecord;
	//unsigned char txt_buf[100];	// use fixed buffer for text record to prevent malloc/free
	//TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);
	//TXTRecordSetValue(&txtRecord, "text1", strlen("text1_content_new"), "text1_content_new");

	// Delay to wait for IP by DHCP
	//vTaskDelay(5000);
	struct netif * pnetif = &xnetif[0];	

	ua_printf(UA_DEBUG, "Uart Adapter mDNS Init");

	if(mDNSResponderInit() == 0) {
		ua_printf(UA_INFO, "mDNS Register service");
				char hostname[32] = {0};  //AMEBA_UART-MODE
				u32 prefix_len = strlen("AMEBA_");
				sprintf(hostname, "AMEBA_");
				sprintf(hostname+prefix_len, "%02x%02x%02x%02x%02x%02x",
				pnetif->hwaddr[0], pnetif->hwaddr[1], pnetif->hwaddr[2],
				pnetif->hwaddr[3], pnetif->hwaddr[4], pnetif->hwaddr[5]);
				
		//TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);
		//TXTRecordSetValue(&txtRecord, "text1", strlen("text1_content"), "text1_content");
		//TXTRecordSetValue(&txtRecord, "text2", strlen("text2_content"), "text2_content");
		dnsServiceRef = mDNSRegisterService(hostname, "_uart_data._tcp", "local", 5001, NULL);
		if(dnsServiceRef == NULL)
			ua_printf(UA_ERROR, "mdns data service register failed!");
		dnsServiceRef2 = mDNSRegisterService(hostname, "_uart_control._tcp", "local", 6001, NULL);		
		if(dnsServiceRef2 == NULL)
			ua_printf(UA_ERROR, "mdns control service register failed!");

	}else{
		ua_printf(UA_INFO, "mDNS Init Failed");	
	}

	//vTaskDelete(NULL);
}

#define _________INIT__RELATED________________________
void uartadapter_auto_connect(void *param)
{
	int ret = 0;
	
	if(wifi_is_ready_to_transceive(RTW_STA_INTERFACE) == RTW_SUCCESS) {
		ua_printf(UA_INFO, "wifi connect successfully!");                     		
		goto START;
	}else{
RETRY:	
		ret = uartadapter_load_wifi_config();
		if(ret != RTW_SUCCESS){
       		ret = uartadapter_simple_config(NULL);
       		if(ret != RTW_SUCCESS){
				ua_printf(UA_INFO, "Simple configure connect failed, try again!");                     		
       			goto RETRY;
       		}
		}		
	}

START:
	if(!sys_thread_new("tcp data server", uartadapter_tcp_data_server_handler,  NULL, 256, UA_UART_THREAD_PRIORITY)) 
  	    ua_printf(UA_ERROR, "%s sys_thread_new data server failed\n", __FUNCTION__);

	vTaskDelay(50);
	if(!sys_thread_new("tcp control server", uartadapter_tcp_control_server_handler,  NULL, 256, UA_UART_THREAD_PRIORITY)) 
  	    ua_printf(UA_ERROR, "%s sys_thread_new control server failed\n", __FUNCTION__);

	vTaskDelay(50);

	if(!sys_thread_new("tcp control server", uartadapter_tcp_select,  NULL, 1024, UA_UART_THREAD_PRIORITY)) 
  	    ua_printf(UA_ERROR, "%s sys_thread_new tcp select failed\n", __FUNCTION__);


	vTaskDelay(50);

	//if(!sys_thread_new("MDNS thread", uartadapter_mdns_thread,  NULL, 768, 6)) 
  	//    ua_printf(UA_ERROR, "%s sys_thread_new serial failed\n", __FUNCTION__);

       uartadapter_mdns_thread(NULL);

	vTaskDelay(50);
	
	ua_printf(UA_INFO, "[MEM] After auao connect, available heap %d", xPortGetFreeHeapSize());

	/* Kill init thread after all init tasks done */
	vTaskDelete(NULL);       
	//auao_config = NULL;
}

void uartadapter_main(void *param)
{
	int ret = 0;
	unsigned int tick_start;
	unsigned int tick_current;
	int pin_high = 0;
	int address = FAST_RECONNECT_DATA;		
	
	while(xSemaphoreTake(ua_main_sema, portMAX_DELAY) == pdTRUE){	
		if(ua_gpio_irq_happen){
			pin_high = 0;
			tick_start = xTaskGetTickCount();
			tick_current = xTaskGetTickCount();			
			while(tick_current - tick_start < 3000){
				if (gpio_read(&gpio_led)){
					pin_high = 1;
					break;
				}else{
					tick_current = xTaskGetTickCount();
				}
				vTaskDelay(10);			
			}
			
			ua_gpio_irq_happen = 0;
			if(!pin_high){
				ret = uartadapter_flasherase(address, sizeof(rtw_wifi_config_t));
				if(ret < 0){
					ua_printf(UA_ERROR, "flash erase error!");
				}
		
				uartadapter_systemreload();			
			}
			
		}

		

	}
	/* Kill init thread after all init tasks done */
	vTaskDelete(NULL);       
	//auao_config = NULL;
}


int uartadapter_init()
{
	int ret = 0;

	ua_printf(UA_INFO, "==============>%s()\n", __func__);

	uartadapter_uart_init();
	uartadapter_gpio_init();

       ua_uart_action_sema = xSemaphoreCreateCounting(300, 0); 	

	vSemaphoreCreateBinary(ua_tcp_sema);
	
	vSemaphoreCreateBinary(ua_work_thread_sema);	
	xSemaphoreTake(ua_work_thread_sema, 1/portTICK_RATE_MS);	

	vSemaphoreCreateBinary(ua_main_sema);	
	xSemaphoreTake(ua_main_sema, 1/portTICK_RATE_MS);	
	
	
	ret= sys_mbox_new(&mbox_for_uart_tx, sizeof(struct ua_tcp_rx_buffer));
	if(ret != ERR_OK)
	{
		ua_printf(UA_ERROR, "mbox_for_uart_tx create failed");	
		goto Exit;
	}

	//wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, _uartadapter_event_handler, NULL);

	/*create uart_thread to handle send&recv data*/
	if(xTaskCreate(uartadapter_uart_rx_handle, ((const char*)"uart_rx"), 768, NULL, UA_UART_THREAD_PRIORITY, NULL) != pdPASS)
		ua_printf(UA_ERROR, "%s xTaskCreate(uart_rx) failed", __FUNCTION__);

	vTaskDelay(50);

	/*create uart_thread to handle send&recv data*/
	if(xTaskCreate(uartadapter_uart_tx_handle, ((const char*)"uart_tx"), 512, NULL, UA_UART_THREAD_PRIORITY, NULL) != pdPASS)
		ua_printf(UA_ERROR, "%s xTaskCreate(uart_tx) failed", __FUNCTION__);

	vTaskDelay(50);
	
	if(xTaskCreate(uartadapter_auto_connect, ((const char*)"auto connnect"), 1024, NULL, UA_UART_THREAD_PRIORITY, NULL) != pdPASS)
		ua_printf(UA_ERROR, "%s xTaskCreate(auao connnect) failed", __FUNCTION__);

	if(xTaskCreate(uartadapter_main, ((const char*)"uart main"), 256, NULL, UA_UART_THREAD_PRIORITY, NULL) != pdPASS)
		ua_printf(UA_ERROR, "%s xTaskCreate(auao connnect) failed", __FUNCTION__);

	
	return 0;
Exit:
	ua_printf(UA_ERROR, "%s(): Initialization failed!", __func__);
	return ret;
}

void example_uart_adapter_init()
{
	// Call back from wlan driver after wlan init done
	p_wlan_uart_adapter_callback = uartadapter_init;
}

#define _________CMD__RELATED________________________
void uartadapter_print_irq_rx_count()
{
		ua_printf(UA_INFO, "ua_tick_last_update: %d!\n", ua_tick_last_update);
		ua_printf(UA_INFO, "ua_tick_current: %d!\n", ua_tick_current);	
		ua_printf(UA_INFO, "ua current tick: %d!\n", xTaskGetTickCount());	
		ua_printf(UA_INFO, "ua_pwrite: %d!\n", ua_pwrite);	
		ua_printf(UA_INFO, "ua_pread: %d!\n", ua_pread);	
		ua_printf(UA_INFO, "ua_overlap: %d!\n", ua_overlap);	
		ua_printf(UA_INFO, "ua_rcv_ch: %d!\n", ua_rcv_ch);	
		ua_printf(UA_INFO, "ua_uart_recv_bytes: %d!\n", ua_uart_recv_bytes);	
		ua_printf(UA_INFO, "ua_rcv_ch: %d!\n", ua_rcv_ch);	
		ua_printf(UA_INFO, "irq_rx_cnt: %d!\n", irq_rx_cnt);
		ua_printf(UA_INFO, "irq_tx_cnt: %d!\n", irq_tx_cnt);
		ua_printf(UA_INFO, "irq_miss_cnt: %d!\n", irq_miss_cnt);		
		ua_printf(UA_INFO, "tcp_tx_cnt: %d!\n", tcp_tx_cnt);		
		ua_printf(UA_INFO, "tcp_send_flag: %d!\n", ua_tcp_send_flag);
		ua_printf(UA_INFO, "ua_tcp_data_fd_list: %d!\n", ua_tcp_data_fd_list[0]);
		ua_printf(UA_INFO, "ua_tcp_control_fd_list: %d!\n", ua_tcp_control_fd_list[0]);
		ua_printf(UA_INFO, "ua_tcp_data_server_listen_fd: %d!\n", ua_tcp_data_server_listen_fd);	
		ua_printf(UA_INFO, "ua_tcp_control_server_listen_fd: %d!\n", ua_tcp_control_server_listen_fd);	

}

void uartadapter_reset_irq_rx_count()
{
	irq_rx_cnt = 0;
	irq_tx_cnt = 0;	
	tcp_tx_cnt = 0;
	irq_miss_cnt = 0;	
}

void uartadapter_set_debug_print(bool enable)
{
	ua_debug_print_en = enable;
}

void cmd_uart_adapter(int argc, char **argv)
{
	if(argc < 2) {
		printf("\n\r inpua error\n");
		return;
	}

	//printf("\n\r haier_test: argv[1]=%s\n", argv[1]);

	if(strcmp(argv[1], "help") == 0){
		printf("\r\nUART THROUGH COMMAND LIST:");
		printf("\r\n==============================");
		printf("\r\n\tuart_baud");		
		printf("\r\n");	
	}else if(strcmp(argv[1], "irq_rx_get") == 0){
		uartadapter_print_irq_rx_count();
       }else if(strcmp(argv[1], "debug_print_en") == 0){
		uartadapter_set_debug_print(TRUE);
	}else if(strcmp(argv[1], "debug_print_dis") == 0){
		uartadapter_set_debug_print(FALSE);
       }else if(strcmp(argv[1], "irq_rx_reset") == 0){
		uartadapter_reset_irq_rx_count();	
	}else if(strcmp(argv[1], "tcp") == 0){
		if(strcmp(argv[2], "-c") == 0 || strcmp(argv[2], "c") == 0){
			strncpy(ua_tcp_server_ip, argv[3], (strlen(argv[3])>16)?16:strlen(argv[3]));
			if(!sys_thread_new("tcp dclient", uartadapter_tcp_data_client_handler,  NULL, 768, UA_UART_THREAD_PRIORITY)) 
  		       	printf("%s sys_thread_new serial failed\n", __FUNCTION__);										
		}else if(strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "s") == 0){
			if(!sys_thread_new("tcp dserver", uartadapter_tcp_data_server_handler,  NULL, 768, UA_UART_THREAD_PRIORITY)) 
  		       	printf("%s sys_thread_new serial failed\n", __FUNCTION__);			
		}
	}else if(strcmp(argv[1], "uart_baud") == 0){
		uartadapter_uart_baud(atoi(argv[2]));	
	}else if(strcmp(argv[1], "uart_para") == 0){
		uartadapter_uart_para(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));		
	}else{
		printf("\n\rCan not find the inpua commond!");
	}
}

