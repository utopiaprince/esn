#include "esn_package.h"
#include <osel_arch.h>
#include <gprs.h>
#include <board.h>
static bool_t esn_gprs_send(uint8_t *data, uint16_t length)
{
	gprs_info_t gprs_info;
	gprs_driver.get(&gprs_info);
	if(gprs_info.gprs_state == WORK_ON)
	{
		led_set(LEN_GREEN, TRUE);
		uint16_t len = length - ID_MAX - 2;
		data[0] = 0xa5;
		data[1] = 0x5a;
		osel_memcpy(&data[2], &len, sizeof(uint16_t));
		length += 4+2;
		//gprs_driver.write(data,length);
		return TRUE;
	}
	else
	{
		led_set(LEN_GREEN, FALSE);
		return FALSE;
	}
}

void camera_send(camera_t *info, uint8_t *pdata, uint16_t len)
{
	uint8_t data[570];
	uint16_t length= 0;
	uint8_t *p = data;
	p+=4;

	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_CAME;
	osel_memcpy(package.bmonitor, info->bmonitor, ID_MAX);
	package.collect_time = info->collect_time;
	package.alarm = 0;	
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	p += sizeof(esn_package_t);
	osel_memcpy(p, &info->cnt, sizeof(uint16_t));
	length+=2;
	p+=2;
	osel_memcpy(p, &info->index, sizeof(uint16_t));
	length+=2;
	p+=2;
	osel_memcpy(p, pdata, len);
	length+=len;
	esn_gprs_send(data, length);
}

void acceleration_send(uint8_t *pdata, uint16_t len)
{
	if(len > sizeof(acceleration_t))
		return;
	uint8_t data[LARGE_PBUF_BUFFER_SIZE];
	uint8_t length= 0;
	uint8_t *p = data;
	p+=4;
	acceleration_t info;
	osel_memcpy(&info, pdata, len);
	
	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_ACCE;
	osel_memcpy(package.bmonitor, info.bmonitor, ID_MAX);
	package.collect_time = info.collect_time;
	package.alarm = 0;	
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	p += sizeof(esn_package_t);
	osel_memcpy(p, &info.x, sizeof(uint16_t));
	p += sizeof(uint16_t);
	osel_memcpy(p, &info.y, sizeof(uint16_t));
	p += sizeof(uint16_t);
	osel_memcpy(p, &info.z, sizeof(uint16_t));
	length+=(3*sizeof(uint16_t));
	esn_gprs_send(data, length);
}

void atmo_send(uint8_t *pdata, uint16_t len)
{
	if(len > sizeof(atmo_t))
		return;
	uint8_t data[100];
	uint8_t length= 0;
	uint8_t *p = data;
	p+=4;
	atmo_t info;
	osel_memcpy(&info, pdata, len);
	
	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_ATMO;
	osel_memcpy(package.bmonitor, info.bmonitor, ID_MAX);
	package.collect_time = info.collect_time;
	package.alarm = 0;	
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	p += sizeof(esn_package_t);
	osel_memcpy(p, &info.atmo_data, sizeof(atmo_data_t));
	length+=sizeof(atmo_data_t);
	esn_gprs_send(data, length);
}

void shock_send(uint8_t *pdata, uint16_t len)
{
	if(len > sizeof(shock_t))
		return;
	uint8_t data[LARGE_PBUF_BUFFER_SIZE];
	uint8_t length= 0;
	uint8_t *p = data;
	p+=4;
	shock_t info;
	osel_memcpy(&info, pdata, len);

	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_SHOCK;
	osel_memcpy(package.bmonitor, info.bmonitor, ID_MAX);
	package.collect_time = info.collect_time;
	package.alarm = TRUE;
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	esn_gprs_send(data, length);
}

void distance_send(uint8_t *pdata, uint16_t len)
{
	if(len > sizeof(distance_t))
		return;
	uint8_t data[LARGE_PBUF_BUFFER_SIZE];
	uint8_t length= 0;
	uint8_t *p = data;
	p+=4;
	distance_t info;
	osel_memcpy(&info, pdata, len);
	
	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_DISTANCE;
	osel_memcpy(package.bmonitor, info.bmonitor, ID_MAX);
	package.collect_time = info.collect_time;
	package.alarm = TRUE;
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	p += sizeof(esn_package_t);
	osel_memcpy(&p, &info.val, sizeof(float));
	length += sizeof(float);
	esn_gprs_send(data, length);
}

void temperature_package(uint8_t *pdata, uint16_t len)
{
	if(len > sizeof(temperature_t))
		return;
	uint8_t data[LARGE_PBUF_BUFFER_SIZE];
	uint8_t length= 0;
	uint8_t *p = data;
	p+=4;
	temperature_t info;
	osel_memcpy(&info, pdata, len);
	
	esn_package_t package;
	mac_addr_get(package.umonitor);
	package.frame_type = DATA;
	package.message_type = M_TEMPERATURE;
	osel_memcpy(package.bmonitor, info.bmonitor, ID_MAX);
	package.collect_time = info.collect_time;
	package.alarm = TRUE;
	
	osel_memcpy(p, &package, sizeof(esn_package_t));
	length += sizeof(esn_package_t);
	p += sizeof(esn_package_t);
	osel_memcpy(&p, &info.val, sizeof(float));
	length += sizeof(float);
	esn_gprs_send(data, length);
}
