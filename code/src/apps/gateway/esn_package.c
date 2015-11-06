#include "esn_package.h"
#include <osel_arch.h>
static void mac_addr_get(uint8_t *mac_addr)
{
        uint8_t id[ID_MAX] = {0xaa};
        osel_memcpy(mac_addr, id, ID_MAX);
}

pbuf_t *shock_package(esn_part_t *info)
{
        pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
        esn_package_t package;
        mac_addr_get(package.umonitor);
        package.frame_type = DATA;
        package.message_type = M_SHOCK;
        osel_memcpy(package.bmonitor, info->bmonitor, ID_MAX);
		package.collect_time = info->collect_time;
        package.alarm = TRUE;

        pbuf->data_p = pbuf->head;
        osel_memcpy(pbuf->data_p, &package, sizeof(esn_package_t));
        pbuf->data_len = sizeof(esn_package_t) - 4;
        return pbuf;
}

pbuf_t *distance_package(esn_part_t *info)
{
        pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
        esn_package_t package;
        mac_addr_get(package.umonitor);
        package.frame_type = DATA;
        package.message_type = M_DISTANCE;
        osel_memcpy(package.bmonitor, info->bmonitor, ID_MAX);
        package.collect_time = info->collect_time;
        package.alarm = TRUE;
		package.val = info->val;

        pbuf->data_p = pbuf->head;
        osel_memcpy(pbuf->data_p, &package, sizeof(esn_package_t));
        pbuf->data_len = sizeof(esn_package_t);
        return pbuf;
}

pbuf_t *temperature_package(esn_part_t *info)
{
        pbuf_t *pbuf = pbuf_alloc(LARGE_PBUF_BUFFER_SIZE __PLINE1);
        esn_package_t package;
        mac_addr_get(package.umonitor);
        package.frame_type = DATA;
        package.message_type = M_DISTANCE;
        osel_memcpy(package.bmonitor, info->bmonitor, 17);
        package.collect_time = info->collect_time;
        package.alarm = TRUE;
        package.val = info->val;

        pbuf->data_p = pbuf->head;
        osel_memcpy(pbuf->data_p, &package, sizeof(esn_package_t));
        pbuf->data_len = sizeof(esn_package_t);
        return pbuf;
}
