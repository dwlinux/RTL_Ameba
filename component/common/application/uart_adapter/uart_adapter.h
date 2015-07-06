#include <platform/platform_stdlib.h>
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
/******************************************************
 *                    Macros
 ******************************************************/
#define UA_ERROR		0
#define UA_WARNING 	1
#define UA_INFO		       2
#define UA_DEBUG		3
#define UA_NONE		       0xFF
#define UA_DEBUG_LEVEL UA_INFO

#define	UA_UART_THREAD_PRIORITY	 5
#define	UA_UART_THREAD_STACKSIZE 512

#if (UA_DEBUG_LEVEL== UA_NONE)
#define ua_printf(level, fmt, arg...)
#else
#define ua_printf(level, fmt, arg...)     \
do {\
	if (level <= UA_DEBUG_LEVEL) {\
		if (level <= UA_ERROR) {\
			printf("\r\nERROR: " fmt, ##arg);\
		} \
		else {\
			printf("\r\n"fmt, ##arg);\
		} \
	}\
}while(0)
#endif

/******************************************************
 *                    Constants
 ******************************************************/
typedef enum
{
	UART_CTRL_MODE_SET_REQ = 0,
	UART_CTRL_MODE_SET_RSP = 1,
	UART_CTRL_MODE_GET_REQ = 2,
	UART_CTRL_MODE_GET_RSP = 3,
}ua_ctrl_mode_t;

typedef enum
{
	UART_CTRL_TYPE_BAUD_RATE = 0x01,
	UART_CTRL_TYPE_WORD_LEN = 0x02,
	UART_CTRL_TYPE_PARITY = 0x04,
	UART_CTRL_TYPE_STOP_BIT = 0x08,
	UART_CTRL_TYPE_FLOW_CTRL = 0x10,
}ua_ctrl_type_t;

/******************************************************
 *                   Structures
 ******************************************************/
typedef long time_t; 

typedef  struct  _uartadapter_timeval_st{ 
    long tv_sec; //秒
    long tv_hmsec; //百毫秒
}UARTTHROUGH_TIMEVAL_st; 

//获取网络DHCP参数时存储返回的IP，MAC,GW，MASK，DNS等
typedef struct _ua_net_para {
    char dhcp;
    char ip[16]; // such as string  "192.168.1.1"
    char gate[16];
    char mask[16];
    char dns[16];
    char mac[16]; // such as string "7E0000001111"
    char broadcastip[16];
} ua_net_para_st;

//softAP模式时存储搜索到的 AP列表信息
typedef  struct  _ua_ApList_str  
{  
    char ssid[32];  
    char ApPower;  // min:0, max:100
    char channel; 
    char encryption;
}ua_ApList_str; 

//softAP模式时存储搜索到的 AP列表信息
typedef  struct  _ua_UwtPara_str  
{  
    char ApNum;       //AP number
    ua_ApList_str * ApList; 
} ua_UwtPara_str;  


//获取UART配置信息时，存储获得的串口配置信息
typedef struct _ua_uart_get_str 
{ 
    int BaudRate;    //The baud rate 
    char number;     //The number of data bits 
    char parity;     //The parity(0: none, 1:odd, 2:evn, default:0)
    char StopBits;      //The number of stop bits 
    char FlowControl;    //support flow control is 1 
}ua_uart_get_str;
//配置UART参数时，存储相关的配置信息
typedef struct _ua_uart_set_str 
{ 
    char UartName[8];    // the name of uart 
    int BaudRate;    //The baud rate 
    char number;     //The number of data bits 
    char parity;     //The parity(default NONE) 
    char StopBits;      //The number of stop bits 
    char FlowControl;    //support flow control is 1 
}ua_uart_set_str;



//启动wifi连接时存储WIFI的配置信息
typedef struct _ua_network_InitTypeDef_st 
{ 
    char wifi_mode;    // SoftAp(0)station(1)  
    char wifi_ssid[32]; 
    char wifi_key[32]; 
    char local_ip_addr[16]; 
    char net_mask[16]; 
    char gateway_ip_addr[16]; 
    char dnsServer_ip_addr[16]; 
    char dhcpMode;       // disable(0), client mode(1), server mode(2) 
    char address_pool_start[16]; 
    char address_pool_end[16]; 
    int wifi_retry_interval;//sta reconnect interval, ms
    char channel; 
	char encryption;
} ua_network_InitTypeDef_st; 

#pragma pack(1)
struct ua_ieee80211_frame
{
   unsigned char i_fc[2];
   unsigned char i_dur[2];
   unsigned char i_addr1[6];
   unsigned char i_addr2[6];
   unsigned char i_addr3[6];
   unsigned char i_seq[2];
};
#pragma pack()

//extern void uartadapter_netcallback(ua_net_para_st *pnet);
//extern void uartadapter_wifistatushandler(int status);
extern int uartadapter_init();
//extern void uartadapter_deinit();
//extern void uartadapter_wifipoweron(void);
//extern void uartadapter_wifipoweroff(void);

//extern int uartadapter_startnetwork(ua_network_InitTypeDef_st* pNetworkInitPara);
//extern void uartadapter_wifidisconnect (void);
extern int uartadapter_getnetpara(ua_net_para_st * pnetpara);

//extern int  uartadapter_sethostname(char* name);

extern int uartadapter_readuart(int fd, void *read_buf, size_t size);
void uartadapter_tcp_data_fd_handler();
extern void uartadapter_tcpsend(char *buffer, int size, u8 isctrl);
//extern void uartadapter_test(void *param);
//extern void uartadapter_tcp_control_server_handler(void *param);
//extern void uartadapter_tcp_data_server_handler(void *param);
//extern void uartadapter_gpio_irq (uint32_t id, gpio_irq_event event);
void example_uart_adapter_init();
extern void cmd_uart_adapter(int argc, char **argv);
//#define cmd_uart_adapter	 cmd_ua

