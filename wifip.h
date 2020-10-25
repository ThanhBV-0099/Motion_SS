
#ifndef WIFIP_H
#define WIFIP_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_COUNT_RESET 500
#define MAX_TIMEOUT_HEART_BEART 3000
#define XUNG_NHAY 40
#define NUM_OFF_COUNT_SETUP 6
#define NUM_OF_LENH_ON 2
#define NUM_OF_LENH_OFF 3
#define NUM_OF_LENH_TOGLE 4
#define NUM_OF_LENH_XOA 5
#define TIME_UPDATE 500
#define TIME_NEED_UPDATE 490	
#define TIME_NEED_TO_RESET_TOUCH 1000	// time reset touch
#define PERIOD_TO_RESET_TOUCH 10000	
	
void wifiprocess(void);
void wifi_init(void);
	
#endif
