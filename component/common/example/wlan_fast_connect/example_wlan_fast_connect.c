/******************************************************************************
 *
 * Copyright(c) 2007 - 2015 Realtek Corporation. All rights reserved.
 *
 *
 ******************************************************************************/

 /** @file

	This example demonstrate how to implement wifi fast reconnection
**/
#include <platform_opts.h>
#include <wlan_fast_connect/example_wlan_fast_connect.h>


#include "task.h"
#include <platform/platform_stdlib.h>
#include <wifi/wifi_conf.h>
#include "flash_api.h"

write_reconnect_ptr p_write_reconnect_ptr;

extern void fATW0(void *arg);
extern void fATW1(void *arg);
extern void fATW2(void *arg);
extern void fATWC(void *arg);

/*
* Usage:
*       wifi connection indication trigger this function to save current
*       wifi profile in flash
*
* Condition: 
*       CONFIG_EXAMPLE_WLAN_FAST_CONNECT flag is set
*/

int wlan_wrtie_reconnect_data_to_flash(uint8_t *data, uint32_t len)
{
	flash_t flash;
	uint8_t read_data [FAST_RECONNECT_DATA_LEN];
	if(!data)
	  return -1;

	flash_stream_read(&flash, FAST_RECONNECT_DATA, FAST_RECONNECT_DATA_LEN, (uint8_t *) read_data);

	//wirte it to flash if different content: SSID, Passphrase, Channel
	if(memcmp((void *)data, (void *)read_data, NDIS_802_11_LENGTH_SSID) ||
       memcmp((void *)(data + NDIS_802_11_LENGTH_SSID + 4), (void *)(read_data + NDIS_802_11_LENGTH_SSID + 4), 32) ||
	   memcmp((void *)(data + NDIS_802_11_LENGTH_SSID + 4 + 32 + A_SHA_DIGEST_LEN * 2), (void *)(read_data + NDIS_802_11_LENGTH_SSID + 4 + 32 + A_SHA_DIGEST_LEN * 2), 4)){
	    printf("\r\n %s():not the same ssid/passphrase/channel, need to flash it", __func__);
            
	    flash_erase_sector(&flash, FAST_RECONNECT_DATA);
	    flash_stream_write(&flash, FAST_RECONNECT_DATA, len, (uint8_t *) data);
	}
	
	return 0;
}

/*
* Usage:
*       After wifi init done, waln driver call this function to check whether
*       auto-connect is required.
*
*       This function read previous saved wlan profile in flash and execute connection.
*
* Condition: 
*       CONFIG_EXAMPLE_WLAN_FAST_CONNECT flag is set
*/
int wlan_init_done_callback()
{
	flash_t		flash;
	uint8_t		*data;
	uint32_t	channel;
	uint8_t     pscan_config;
	char key_id;

	data = (uint8_t *)rtw_zmalloc(FAST_RECONNECT_DATA_LEN);
	flash_stream_read(&flash, FAST_RECONNECT_DATA, FAST_RECONNECT_DATA_LEN, (uint8_t *)data);
	if(*((uint32_t *) data) != ~0x0){
	    memcpy(psk_essid, (uint8_t *)data, NDIS_802_11_LENGTH_SSID + 4);
	    memcpy(psk_passphrase, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4, (IW_PASSPHRASE_MAX_SIZE + 1));
	    memcpy(wpa_global_PSK, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4 + (IW_PASSPHRASE_MAX_SIZE + 1), A_SHA_DIGEST_LEN * 2);
	    memcpy(&channel, (uint8_t *)data + NDIS_802_11_LENGTH_SSID + 4 + (IW_PASSPHRASE_MAX_SIZE + 1) + A_SHA_DIGEST_LEN * 2, 4);
	    sprintf(&key_id,"%d",(channel >> 28));
	    channel &= 0xff;	    
	    pscan_config = PSCAN_ENABLE | PSCAN_FAST_SURVEY;
	    //set partial scan for entering to listen beacon quickly
	    wifi_set_pscan_chan((uint8_t *)&channel, &pscan_config, 1);
	    
#ifdef CONFIG_AUTO_RECONNECT
	    wifi_set_autoreconnect(1);
#endif
	    //set wifi connect
	    //open mode
	    if(!strlen((char*)psk_passphrase)){
		fATW0((char*)psk_essid);
	    }
	    //wep mode
	    else if( strlen((char*)psk_passphrase) == 5 || strlen((char*)psk_passphrase) == 13){
		fATW0((char*)psk_essid);
		fATW1((char*)psk_passphrase);
		fATW2(&key_id);
	    }
	    //WPA/WPA2
	    else{
		fATW0((char*)psk_essid);
		fATW1((char*)psk_passphrase);
	    }
	    fATWC(NULL);
	}
	if(data)
	    rtw_mfree(data);
	return 0;
}


void example_wlan_fast_connect()
{
	// Call back from wlan driver after wlan init done
	p_wlan_init_done_callback = wlan_init_done_callback;

	// Call back from application layer after wifi_connection success
	p_write_reconnect_ptr = wlan_wrtie_reconnect_data_to_flash;

}
