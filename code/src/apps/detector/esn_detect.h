/**
 * @brief       : this
 * @file        : esn_detect.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-05
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-05  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_DETECT_H__
#define __ESN_DETECT_H__

#ifndef __DATA_TYPE_DEF_H__
#error "include data_type_def.h must appear in source file before include esn_detect.h"
#endif

//extern bool_t camrea_has_sent;
#define RANGE_ENABLE                    (1u)
#define RANGE_DATA_TIME					(620u)		//*< 10分钟
#define RANGE_MIN_THRESHOLD				(100.0)       //*< 默认100米

#define ANGLE_ENABLE                    (1u)
#define ANGLE_DATA_TIME                 (600u)

#define TEMP_DATA_TIME					(600u)
#define TEMP_MAX_THRESHOLD				(100.0)

#define CAMERA_ENABLE                   (1u)
#define CAMERA_DATA_TIME				(14400u)	//*< 4小时

#define ATMOS_ENABLE                    (1u)
#define ATMOS_DATA_TIME					(610u)


/**
 * @brief init esn detect task
 * @return
 */
bool_t esn_detect_init(void);

#endif