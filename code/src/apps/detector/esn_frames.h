/**
 * @brief       : this
 * @file        : esn_frames.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-10
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-10  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_FRAMES_H__
#define __ESN_FRAMES_H__


typedef enum
{
	ESN_DATA = 0,
	ESN_CTRL = 1,
} esn_frame_type_t;

typedef enum
{
	ALARM_N = 0,
	ALARM_T = 1, 
}esn_alarm_type_t;

typedef enum
{
	DATATYPE_VIBRATION   = 0,
	DATATYPE_DISTANCE    = 1,
	DATATYPE_TEMPERATURE = 2,
	DATATYPE_PICTURE     = 3,
} ens_frame_data_type_t;

typedef enum
{
	PICTURE_START = 0,
	PICTURE_DATA  = 1,
	PICTURE_END   = 2,
} esn_frame_picture_type_t;


#endif