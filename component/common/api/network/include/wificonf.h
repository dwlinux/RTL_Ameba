//----------------------------------------------------------------------------//
#ifndef __WIFICONF_H
#define __WIFICONF_H

#ifdef __cplusplus
  extern "C" {
#endif

#include "main.h"
#include "util.h"    

typedef enum _WIFI_SECURITY_TYPE{
	WIFI_SECURITY_OPEN = 0,
	WIFI_SECURITY_WEP,
	WIFI_SECURITY_WPA,
	WIFI_SECURITY_WPA2
}WIFI_SECURITY_TYPE;

typedef enum _WIFI_COUNTRY_CODE{
	WIFI_COUNTRY_US = 0,
	WIFI_COUNTRY_EU,
	WIFI_COUNTRY_JP,
	WIFI_COUNTRY_CN
}WIFI_COUNTRY_CODE;

typedef enum _WIFI_MODE_TYPE{
	WIFI_MODE_STA = 0,
	WIFI_MODE_AP
}WIFI_MODE_TYPE;

typedef enum _WIFI_LINK_STATUS{
	WIFI_LINK_DISCONNECTED = 0,
	WIFI_LINK_CONNECTED
}WIFI_LINK_STATUS;

typedef struct _WIFI_AP{
	unsigned char 		*ssid;
	WIFI_SECURITY_TYPE	security_type;
	unsigned char 		*password;
	int 				ssid_len;
	int 				password_len;
	int					channel;
}WIFI_AP;

typedef struct _WIFI_NETWORK{
	unsigned char 		*ssid;
	WIFI_SECURITY_TYPE	security_type;
	unsigned char 		*password;
	int 				ssid_len;
	int 				password_len;
	int					key_id;
}WIFI_NETWORK;

typedef struct _WIFI_SETTING{
	WIFI_MODE_TYPE		mode;
	unsigned char 		ssid[33];
	unsigned char		channel;
	WIFI_SECURITY_TYPE	security_type;
	unsigned char 		password[33];
}WIFI_SETTING;

typedef enum _WIFI_NETWORK_MODE {
    WIFI_NETWORK_B   = 1,
	WIFI_NETWORK_BG  = 3,
	WIFI_NETWORK_BGN = 11
} WIFI_NETWORK_MODE;

int	wifi_connect(WIFI_NETWORK *pNetwork);
int wifi_disconnect(void);
WIFI_LINK_STATUS wifi_get_connect_status(WIFI_NETWORK *pWifi);
int wifi_set_country(WIFI_COUNTRY_CODE country_code);
int wifi_get_rssi(int *pRSSI);
int wifi_rf_on(void);
int wifi_rf_off(void);
int wifi_active_ap(WIFI_AP *pAP);
int wifi_scan(char *buf, int buf_len);
int wifi_get_setting(WIFI_SETTING *pSetting);
int wifi_show_setting(WIFI_SETTING *pSetting);
int wifi_set_network_mode(WIFI_NETWORK_MODE mode);
void wifi_indication( WIFI_EVENT_INDICATE event, char *buf, int buf_len);

#ifdef __cplusplus
  }
#endif

#endif // __WIFICONF_H

//----------------------------------------------------------------------------//
