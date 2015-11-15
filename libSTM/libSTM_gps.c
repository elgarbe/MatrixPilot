//
//  libSTM_gps.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 8/11/2015.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

#include "../libUDB/libUDB.h"
//#include "../libUDB/heartbeat.h"
//#include "../libUDB/uart.h"

//#include "stm32f4xx_hal.h"
//#include "cmsis_os.h"


background_callback gps_trigger_callback = NULL;

void udb_background_trigger(background_callback callback)
{
	gps_trigger_callback = callback;
	TriggerTaskGPS();
//	if (callback) callback();
}

void RunTaskGPS(void) // called from OS TaskGPS
{
	if (gps_trigger_callback) gps_trigger_callback();
//void udb_background_callback_triggered(void);
//		udb_background_callback_triggered();
//	udb_background_trigger(&gps_parse_common_callback);
}

