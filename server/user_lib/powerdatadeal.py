# coding=utf-8
import struct
from ctypes import *
import _thread

import user_lib.globalval as globalval
import user_lib.mysql as mysql

lock = _thread.allocate_lock()


class data_state_e():
    HEAD1 = 1
    HEAD2 = 2
    LEN1 = 3
    LEN2 = 4


class message_type_e():
    M_WIRE = 0x20
    M_SHOCK = 0x30
    M_DISTANCE = 0x31
    M_TEMPERATURE = 0x32

class power_t:
    def __init__(self):
        self.monitor = ''  # 监测设备
        self.frame_type = 0  # 帧类型
        self.message_type = 0  # 报文类型
        self.bmonitor = ''  # 被监测设备
        self.collect_time = 0  # 采集时间
        self.alarm = False  # 报警状态
        self.distance = 0  # 距离
        self.temperature = 0  # 温度
        self.ip = ''  # IP


def read_id_manage():  # 读取id_manage信息
    lock.acquire()
    globalval.id_list = []
    rs, row = mysql.mdb_call('call select_table_id_manage()')
    for i in range(0, row):
        globalval.id_list.append(rs[i])
    lock.release()


def dec2hex_str(dec):
    x = hex(dec)[2:]
    if len(x) == 1:
        x = '0' + x
    return x


def convert(s):
    i = int(s, 16)  # convert from hex to a Python int
    cp = pointer(c_int(i))  # make this into a c integer
    fp = cast(cp, POINTER(c_float))  # cast the int pointer to a float pointer
    return fp.contents.value  # dereference the pointer, get the float


def find_id(monitor):
    num = len(globalval.id_list)
    mark = False
    for i in range(0, num):
        id = globalval.id_list[i]['id']
        if id == monitor:
            rs = globalval.id_list[i]
            mark = True
            break
    if mark == False:
        rs = None
    return rs


def update_info_all(power, rs):
    sql = 'call update_id_manage_infoall(\"%s\",\"%s\",%d,%d,%d,%d,\"%s\",\"%s\")' % (power.monitor, power.bmonitor,
                                                                                      1, rs['shock_state'],
                                                                                      rs['temperature_state'],
                                                                                      rs['distance_state'], power.ip,
                                                                                      rs['gateway_id'])
    mysql.mdb_call(sql)


def confirm_id(power):  # 验证是否需要往id_manage中插入数据
    rs = find_id(power.monitor)
    if rs != None:
        if rs['online_state'] == 0:
            update_info_all(power, rs)
    else:
        sql = 'call insert_id_to_id_manage(\'%s\',\'%s\',\'%s\')' % (power.monitor, power.bmonitor, power.ip)
        mysql.mdb_call(sql)
        print('创建ID：%s' % (power.monitor))
        read_id_manage()


def shock(power, buf):
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2
    confirm_id(power)
    rse = find_id(power.monitor)
    rse['shock_state'] = power.alarm
    insert_shock = (("call insert_shock(\"%s\",%d,%d)") % (power.monitor, power.alarm, power.collect_time))
    rs, row = mysql.mdb_call(insert_shock)
    if power.alarm == 1:
        print("创建震动报警记录%d,id:%s" % (rs[0]['LAST_INSERT_ID()'], power.monitor))
    else:
        print("取消震动报警记录,id:%s" % (power.monitor))
    return


def distance(power, buf):
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2

    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    power.distance = convert(s)
    confirm_id(power)
    rse = find_id(power.monitor)
    insert_distance = (
        "call insert_distance(\"%s\",%d,%f,%d)" % (power.monitor, power.alarm,
                                                      power.distance, power.collect_time))
    rs, row = mysql.mdb_call(insert_distance)
    if rse['distance_state'] != power.alarm:
        if power.alarm == 1:
            rse['distance_state'] = 1
            print("创建距离报警记录%d,id:%s;距离:%s" % (rs[0]['LAST_INSERT_ID()'], power.monitor, power.distance))
        else:
            rse['distance_state'] = 0
            print("取消距离报警记录,id:%s;距离:%s" % (power.monitor,power.distance))
    else:
        if power.alarm == 1:
            rse['distance_state'] = 1
            print("创建距离报警记录%d,id:%s;距离:%s" % (rs[0]['LAST_INSERT_ID()'], power.monitor,power.distance))
    return


def temperature(power, buf):
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2

    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    power.temperature = convert(s)
    confirm_id(power)
    rse = find_id(power.monitor)
    insert_temperature = (
        "call insert_temperature(\"%s\",%d,%f,%d)" % (power.monitor, power.alarm,
                                                         power.temperature, power.collect_time))
    rs, row = mysql.mdb_call(insert_temperature)
    if rse['temperature_state'] != power.alarm:
        if power.alarm == 1:
            rse['temperature_state'] = 1
            print("创建温度报警记录%d,id:%s;温度:%s" % (rs[0]['LAST_INSERT_ID()'], power.monitor,power.temperature))
        else:
            rse['temperature_state'] = 0
            print("取消温度报警记录,id:%s;温度:%s" % (power.monitor,power.temperature))
    else:
        if power.alarm == 1:
            rse['temperature_state'] = 1
            print("创建温度报警记录%d,id:%s;温度:%s" % (rs[0]['LAST_INSERT_ID()'], power.monitor,power.temperature))
    return


def frame_data_deal(power, buf, length):
    power.message_type = buf[0]
    data = buf[1:]
    if power.message_type == message_type_e.M_SHOCK:
        shock(power, data)
    elif power.message_type == message_type_e.M_DISTANCE:
        distance(power, data)
    elif power.message_type == message_type_e.M_TEMPERATURE:
        temperature(power, data)
    return


def frame_deal(buf, length, client_address):
    power = power_t()
    power.ip = (client_address[0] + ':' + str(client_address[1]))
    index = 0
    if length > 17:
        for i in range(0, 17):
            power.monitor += dec2hex_str(buf[i])
        index += 17;
        length -= 17
        power.frame_type = buf[index]
        index += 1;
        length -= 1
        if power.frame_type == 1:  # 收到数据帧
            b = buf[index:]
            frame_data_deal(power, b, length)
    return


def recv_data(buf, client_address):
    length = len(buf)
    state = data_state_e.HEAD1
    if length < 5:
        return length
    index = 0;
    while length != 0:
        if state == data_state_e.HEAD1:
            if buf[index] == 0xa5:
                state = data_state_e.HEAD2
            else:
                state = data_state_e.HEAD1
            index += 1
        elif state == data_state_e.HEAD2:
            if buf[index] == 0x5a:
                state = data_state_e.LEN1
            else:
                state = data_state_e.HEAD1
            index += 1
        elif state == data_state_e.LEN1:
            frame_len = buf[index]
            index += 1
            state = data_state_e.LEN2
        else:
            frame_len = frame_len + (buf[index] << 8)
            index += 1
            frame_len += 21  # 额外长度，见协议，状态检测装置ID+帧类型+报文类型+校验位
            if (length - 1) < frame_len:
                return length + 3
            else:
                frame = []
                for i in range(index, index + frame_len):
                    frame.append(buf[i])
                length -= frame_len
                index += frame_len
                frame_deal(frame, frame_len, client_address)
            state = data_state_e.HEAD1
        length -= 1
    return 0
