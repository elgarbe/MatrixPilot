#ifndef _FLYBYWIRE_C_
#define _FLYBYWIRE_C_

#include "defines.h"
#if (FLYBYWIRE_ENABLE_METHOD != FLYBYWIRE_NONE)

#include "FlyByWire.h"


BYTE fbw_inject_pos = 0;
BYTE fbw_inject[LENGTH_OF_PACKET];
int fbw_pwm[NUM_INPUTS+1];

int get_fbw_pwm(int index)
{
	return fbw_pwm[index];
}
	
BYTE get_fbw_pos(void)
{
	return fbw_inject_pos;
}	

void fbw_live_begin( void )
{
	fbw_inject_pos = 1 ; // we never actually see the first value on UART
}


BOOL fbw_live_received_byte( unsigned char inbyte )
{
	switch (fbw_inject_pos)
	{
	case 0:
		if (inbyte == 'F')
			fbw_inject_pos++;
		else
			return FALSE;
		break;

	case 1:
		if (inbyte == 'b')
			fbw_inject_pos++;
		else
			return FALSE;
		break;

	case 2:
		if (inbyte == 'W')
			fbw_inject_pos++;
		else
			return FALSE;
		break;
			
	default:
		if (fbw_inject_pos < LENGTH_OF_PACKET)
		{
			fbw_inject[fbw_inject_pos++] = inbyte ;
		}
		else
		{
			return FALSE;
		}
		break;
	} // switch
		
	return TRUE;
}

void fbw_live_commit(void)
{
	fbw_live_commit_buf(fbw_inject);
}	

void fbw_live_commit_buf(BYTE* buf)
{
	// [0,1,2] = "FbW" Header packet
	// [3,4] = AILERON_INPUT_CHANNEL (LSB, MSB)
	// [5,6] = ELEVATOR_INPUT_CHANNEL (LSB, MSB)
	// [7,8] = MODE_SWITCH_INPUT_CHANNEL (LSB, MSB)
	// [9,10] = RUDDER_INPUT_CHANNEL (LSB, MSB)
	// [11,12] = THROTTLE_INPUT_CHANNEL (LSB, MSB)
	
	BYTE buf_index = LENGTH_OF_HEADER;
	WORD_VAL tempPWM;
	
	tempPWM.v[0] = buf[buf_index++]; // LSB first
	tempPWM.v[1] = buf[buf_index++];
	fbw_pwm[AILERON_INPUT_CHANNEL] = tempPWM.Val;
	
	tempPWM.v[0] = buf[buf_index++];
	tempPWM.v[1] = buf[buf_index++];
	fbw_pwm[ELEVATOR_INPUT_CHANNEL] = tempPWM.Val;
	
	tempPWM.v[0] = buf[buf_index++];
	tempPWM.v[1] = buf[buf_index++];
	fbw_pwm[MODE_SWITCH_INPUT_CHANNEL] = tempPWM.Val;
	
	tempPWM.v[0] = buf[buf_index++];
	tempPWM.v[1] = buf[buf_index++];
	fbw_pwm[RUDDER_INPUT_CHANNEL] = tempPWM.Val;
	
	tempPWM.v[0] = buf[buf_index++];
	tempPWM.v[1] = buf[buf_index++];
	fbw_pwm[THROTTLE_INPUT_CHANNEL] = tempPWM.Val;
}	
	
#endif // (UART_RX_FYBYWIRE == 1) && (FLYBYWIRE_ENABLE_METHOD != FLYBYWIRE_NONE)

#endif // _FLYBYWIRE_C_

