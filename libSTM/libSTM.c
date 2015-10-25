//
//  libSTM.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 17/6/2014.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

#include "../libUDB/libUDB.h"
#include "../libUDB/ADchannel.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/magnetometer.h"
#include "magnetometerOptions.h"
#include "../libUDB/events.h"
#include "../libUDB/osd.h"
#include "../libUDB/mpu6000.h"
#include "../libUDB/uart.h"

//#if (BOARD_TYPE == PX4_BOARD)

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
//#include "libUDB.h"
#include "radioIn.h"
#include "serialIO.h"
#include "servoOut.h"
//#include "mpu6000.h"
//#include "heartbeat.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


//uint16_t udb_heartbeat_counter;

//This is already defined on radioIn.c
//TODO: Where is the correct place to define this?
//
// mostly everything in the various libUDB/DCM/STM modules will
// ideally be moved to functionality specific modules.
//
// in the beta branch development has started on abstracting
// the various potential input control sources via module selectIn
// which complicates the answer in this branch
//
//int16_t udb_pwIn[MAX_INPUTS];   // pulse widths of radio inputs
//int16_t udb_pwTrim[MAX_INPUTS]; // initial pulse widths for trimming
int16_t udb_pwOut[MAX_OUTPUTS]; // pulse widths for servo outputs

union udb_fbts_byte udb_flags;

struct ADchannel udb_xaccel, udb_yaccel, udb_zaccel;    // x, y, and z accelerometer channels
struct ADchannel udb_xrate, udb_yrate, udb_zrate;       // x, y, and z gyro channels
struct ADchannel udb_vref;                              // reference voltage
struct ADchannel udb_analogInputs[4];

int16_t udb_magFieldBody[3];                    // magnetic field in the body frame of reference
int16_t udb_magOffset[3] = { 0 , 0 , 0 };       // magnetic offset in the body frame of reference
int16_t magGain[3] = { RMAX , RMAX , RMAX };    // magnetometer calibration gains
int16_t rawMagCalib[3] = { 0 , 0 , 0 };
uint8_t magreg[6];                              // magnetometer read-write buffer
int16_t magFieldRaw[3];

union longww battery_current;                   // battery_current._.W1 is in tenths of Amps
union longww battery_mAh_used;                  // battery_mAh_used._.W1 is in mAh
union longww battery_voltage;                   // battery_voltage._.W1 is in tenths of Volts
uint8_t rc_signal_strength;                     // rc_signal_strength is 0-100 as percent of full signal

int16_t magMessage;
int16_t vref_adj;

volatile uint16_t trap_flags;
volatile uint32_t trap_source;
volatile uint16_t osc_fail_count;
uint16_t mp_rcon = 3;                           // default RCON state at normal powerup

extern int mp_argc;
extern char **mp_argv;

uint8_t sil_radio_on;

// Functions only included with nv memory.
#if (USE_NV_MEMORY == 1)
UDB_SKIP_FLAGS udb_skip_flags = {0, 0, 0};

void udb_skip_radio_trim(boolean b)
{
	udb_skip_flags.skip_radio_trim = 1;
}

void udb_skip_imu_calibration(boolean b)
{
	udb_skip_flags.skip_imu_cal = 1;
}
#endif

uint8_t udb_cpu_load(void)
{
	return 5; // sounds reasonable for a fake cpu%
}

//NOTE: This is implemented on servoOut
//int16_t udb_servo_pulsesat(int32_t pw)
//{
//	if (pw > SERVOMAX) pw = SERVOMAX;
//	if (pw < SERVOMIN) pw = SERVOMIN;
//	return (int16_t)pw;
//}

void udb_servo_record_trims(void)
{
	int16_t i;

	for (i = 1; i <= NUM_INPUTS; i++)
	{
		udb_pwTrim[i] = udb_pwIn[i];
//		DPRINT("udb_pwTrim[%i] = %u\r\n", i, udb_pwTrim[i]);
	}
}

//NOTE: This is implemented on servoOut
//void udb_set_action_state(boolean newValue)
//{
//	// not simulated
//	(void)newValue;   // unused parameter
//}

void udb_a2d_record_offsets(void)
{
	UDB_XACCEL.offset = UDB_XACCEL.value;
	udb_xrate.offset  = udb_xrate.value;
	UDB_YACCEL.offset = UDB_YACCEL.value - (Y_GRAVITY_SIGN ((int16_t)(2*GRAVITY))); // opposite direction
	udb_yrate.offset  = udb_yrate.value;
	UDB_ZACCEL.offset = UDB_ZACCEL.value;
	udb_zrate.offset  = udb_zrate.value;
	udb_vref.offset   = udb_vref.value;
}

uint16_t get_reset_flags(void)
{
	return mp_rcon;
}

#if (MAG_YAW_DRIFT == 1)

static magnetometer_callback_funcptr magnetometer_callback = NULL;

void rxMagnetometer(magnetometer_callback_funcptr callback)
{
	magnetometer_callback = callback;
}

void I2C_doneReadMagData(void)
{
	magFieldRaw[0] = (magreg[0]<<8) + magreg[1];
	magFieldRaw[1] = (magreg[2]<<8) + magreg[3];
	magFieldRaw[2] = (magreg[4]<<8) + magreg[5];

	if (magMessage == 7)
	{
		udb_magFieldBody[0] = MAG_X_SIGN((__builtin_mulsu((magFieldRaw[MAG_X_AXIS]), magGain[MAG_X_AXIS]))>>14)-(udb_magOffset[0]>>1);
		udb_magFieldBody[1] = MAG_Y_SIGN((__builtin_mulsu((magFieldRaw[MAG_Y_AXIS]), magGain[MAG_Y_AXIS]))>>14)-(udb_magOffset[1]>>1);
		udb_magFieldBody[2] = MAG_Z_SIGN((__builtin_mulsu((magFieldRaw[MAG_Z_AXIS]), magGain[MAG_Z_AXIS]))>>14)-(udb_magOffset[2]>>1);

		if ((abs(udb_magFieldBody[0]) < MAGNETICMAXIMUM) &&
			(abs(udb_magFieldBody[1]) < MAGNETICMAXIMUM) &&
			(abs(udb_magFieldBody[2]) < MAGNETICMAXIMUM))
		{
//			udb_magnetometer_callback();
			if (magnetometer_callback != NULL)
			{
				magnetometer_callback();
			}
			else
			{
				printf("ERROR: magnetometer_callback function pointer not set\r\n");
			}
		}
		else
		{
			magMessage = 0;         // invalid reading, reset the magnetometer
		}
	}
}

void HILSIM_MagData(magnetometer_callback_funcptr callback)
{
	(void)callback;
//	magnetometer_callback = callback;
	magMessage = 7;                 // indicate valid magnetometer data
	I2C_doneReadMagData();          // run the magnetometer computations
}

#endif // MAG_YAW_DRIFT

uint16_t register_event_p(void (*event_callback)(void), eventPriority priority) { return 0; }
void trigger_event(uint16_t hEvent) {}
/*
void osd_init(void) {}
void osd_reset(void) {}
void osd_spi_init(void) {}
void osd_spi_write(int8_t address, int8_t byte) {}
void osd_spi_write_byte(int8_t byte) {}
void osd_spi_write_location(int16_t loc) {}
void osd_spi_write_string(const uint8_t* str) {}
void osd_spi_write_vertical_string_at_location(int16_t loc, const uint8_t* str) {}
void osd_spi_erase_chars(uint8_t n) {}
void osd_spi_write_number(int32_t val, int8_t num_digits, int8_t decimal_places, int8_t num_flags, int8_t header, int8_t footer) {}
 */
uint8_t retSD;    /* Return value for SD */
char SD_Path[4];  /* SD logical drive path */

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */

void filesys_init(void)
{
    FRESULT res;                                            /* FatFs function common result code */
	uint8_t wtext[] = "Matrix Pilot with FatFs support";    /* File write buffer */
    uint32_t byteswritten;                                  /* File write counts */
    /*## FatFS: Link the SD driver ###########################*/
    //  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path);
    if(FATFS_LinkDriver(&SD_Driver, SD_Path) == 0)
    {
        /*##-2- Register the file system object to the FatFs module ##############*/
        res=f_mount(&SDFatFs, (TCHAR const*)SD_Path, 0);
        if(res == FR_OK)
        {
            /*##-4- Create and Open a new text file object with write access #####*/
            res=f_open(&MyFile, "MP_Nucleo.TXT", FA_CREATE_ALWAYS | FA_WRITE);
            if(res == FR_OK)
            {
                /*##-5- Write data to the text file ################################*/
                res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
                if((byteswritten == 0) || (res == FR_OK))
                {
                    /*##-6- Close the open text file #################################*/
                    f_close(&MyFile);
                } else {
					printf("f_write() - FAILED\r\n");
				}

            } else {
				printf("f_open() - FAILED\r\n");
			}
        } else {
			printf("f_mount() - FAILED\r\n");
		}
    } else {
		printf("FATFS_LinkDriver() - FAILED\r\n");
    }
    /*##-11- Unlink the micro SD disk I/O driver ###############################*/
    FATFS_UnLinkDriver(SD_Path);
}

//static jmp_buf default_jmp_buf;

int setjmp(void)
{
	return 0;
}

void __delay32(unsigned long cycles)
{
    /* Insert delay in ms */
//    HAL_Delay(cycles);
}

//#if (CONSOLE_UART == 0)
//void console(void) {}
//#endif // CONSOLE_UART

//void Reset_Handler(void) {} // this must be loosely defined in the startup code and the default seems to call main()
//int mcu_init(void) {} // now provided by main() in the STMCubeMX generated code (redef'd to mcu_init()

void udb_init(void)
{
	udb_heartbeat_counter = 0;
	udb_flags.B = 0;
	sil_radio_on = 1;
//	sil_ui_init(mp_rcon);
	MPU6000_init16(&heartbeat); // initialise mpu from udb_init() from matrixpilot_init() - NOTE: now already called in hardware initialisation
}

void udb_run(void) // traditionally only idled or stopped the clock
{
//			udb_callback_read_sensors();

			udb_flags._.radio_on = (sil_radio_on &&
			    udb_pwIn[3] >= FAILSAFE_INPUT_MIN &&
			    udb_pwIn[3] <= FAILSAFE_INPUT_MAX);
//			    udb_pwIn[FAILSAFE_INPUT_CHANNEL] >= FAILSAFE_INPUT_MIN &&
//			    udb_pwIn[FAILSAFE_INPUT_CHANNEL] <= FAILSAFE_INPUT_MAX);

//			LED_GREEN = (udb_flags._.radio_on) ? LED_ON : LED_OFF;

			udb_heartbeat_40hz_callback(); // Run at 40Hz
			udb_heartbeat_callback(); // Run at HEARTBEAT_HZ

//			sil_ui_update();

//			if (udb_heartbeat_counter % (2 * HEARTBEAT_HZ) == 0)
//			{
//				writeEEPROMFileIfNeeded(); // Run at 0.5Hz
//			}

			udb_heartbeat_counter++;
//			nextHeartbeatTime = nextHeartbeatTime + UDB_STEP_TIME;
//			if (nextHeartbeatTime > UDB_WRAP_TIME) nextHeartbeatTime -= UDB_WRAP_TIME;
}

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

int one_hertz_flag = 0;
uint16_t udb_heartbeat_counter = 0;
uint16_t udb_heartbeat_40hz_counter = 0;
#define HEARTBEAT_MAX 57600 // Evenly divisible by many common values: 2^8 * 3^2 * 5^2

void heartbeat(void) // called from MPU6000 ISR
{
	// Capture cpu_timer once per second.
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/1) == 0)
	{
//		cpu_load_calc();
		one_hertz_flag = 1;
	}

	// This calls the state machine implemented in MatrixPilot/states.c
	// it is called at high priority to ensure manual control takeover can
	// occur, even if the lower priority tasks hang
	// Call the periodic callback at 40 Hz
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/40) == 0)
	{
		// call the FSM. implemented in states.c
		udb_heartbeat_40hz_callback(); // this was called udb_background_callback_periodic()
		udb_heartbeat_40hz_counter = (udb_heartbeat_40hz_counter+1) % HEARTBEAT_MAX;
	}

	udb_heartbeat_counter = (udb_heartbeat_counter+1) % HEARTBEAT_MAX;
	TriggerTaskIMU();
}

//void pulse(void) // called from TaskIMU
void RunTaskIMU(void) // called from OS TaskIMU
{
//	led_off(LED_BLUE);     // indicates logfile activity

#if (NORADIO != 1)
	// 20Hz testing of radio link
	if ((udb_heartbeat_counter % (HEARTBEAT_HZ/20)) == 1)
	{
		radioIn_failsafe_check();
	}
	// Computation of noise rate
	// Noise pulses are counted when they are detected, and reset once a second
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/1) == 1)
	{
		radioIn_failsafe_reset();
	}
#endif // NORADIO

#ifdef VREF
	vref_adj = (udb_vref.offset>>1) - (udb_vref.value>>1);
#else
	vref_adj = 0;
#endif // VREF

//	calculate_analog_sensor_values();
	udb_callback_read_sensors();
	udb_flags._.a2d_read = 1; // signal the A/D to start the next summation

	// process sensor data, run flight controller, generate outputs. implemented in libDCM.c
	udb_heartbeat_callback(); // this was called udb_servo_callback_prepare_outputs()
}



////////////////////////////////////////////////////////////////////////////////
/*
void vApplicationIdleHook(void)
{
	{
#if (USE_TELELOG == 1)
		telemetry_log();
#endif
#if (USE_USB == 1)
		USBPollingService();
#endif
#if (CONSOLE_UART != 0 && SILSIM == 0)
		console();
#endif
		udb_run();
	}
}
 */

////////////////////////////////////////////////////////////////////////////////

void ClrError(void)
{
//	if (U##x##STAbits.OERR) U##x##STAbits.OERR = 0;
}

void FSInit(void) {}

void vApplicationTickHook(void) {}


int tsirq = 0;

void TAMP_STAMP_IRQHandler(void)
{
	tsirq = 1;
}

//! Test if in interrupt mode
inline int isInterrupt(void)
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0 ;
}

static __IO uint32_t uwTick;

/**
  * @brief This function is called to increment  a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in Systick ISR.
 * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
void HAL_IncTick(void)
{
  uwTick++;
}

/**
  * @brief Provides a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
uint32_t HAL_GetTick(void)
{
	if (isInterrupt())
	{
//		uwTick++;
	}
	return uwTick;
}

void _StartDefaultTask(void const * argument)
{
	int16_t pw[8];

	setvbuf(stdout, NULL, _IONBF, 0);

	printf("MatrixPilot STM32-nucleo\r\n");

	matrixpilot_init(); // initialise MP from default CMSOS task in STM32 build
//	udb_init_GPS();
	udb_init_GPS(&udb_gps_callback_get_byte_to_send, &udb_gps_callback_received_byte);

	filesys_init(); // attempts to mount a file system (STM builds)

	for(;;)
	{
		matrixpilot_loop(); // calls udb_run()
//		HAL_Delay(1); // do we need to call anything to allow a yield?

		HAL_Delay(25); // should give us very roughly 40Hz

        // Read the inputs
        radioIn_getInput(pw, 8);
        // Update the servos
        set_pwm_outputs(pw);

		if (tsirq)
		{
			tsirq = 0;
			printf("#");
//			fflush(stdout);
		}
  }
}

//#endif // (BOARD_TYPE == PX4_BOARD)
