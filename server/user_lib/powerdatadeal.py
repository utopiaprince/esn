#coding=utf-8
import struct
import binascii
import re
import os
from ctypes import *
import _thread
import sched, time
import pymysql as mdb
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
    M_ACCE  = 0x33
    M_ATMO  = 0x34
    M_CAME  = 0x35

class acce_t:#加速度
    def __init__(self):
        self.x = 0
        self.y = 0
        self.z = 0

class atmos_t:#气象
    def __init__(self):
        self.driver_state_temperature = 0               #温度
        self.driver_state_wind_direction_speed = 0      #风向风速
        self.driver_state_pressure = 0                  #气压
        self.driver_state_compass = 0                   #电子罗盘
        self.driver_state_hyetometer = 0                #雨量计

        self.wind_direction = 0
        self.wind_speed = 0
        self.temperature = 0
        self.humidity = 0
        self.pressure = 0
        self.compass = 0
        self.rainfall_state = 0
        self.rainfall_streng = 0
        self.rainfall_total = 0
        self.rainfall_streng_unit = ''

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
    return

def GetMiddleStr(content,startStr,endStr):
    startIndex = content.index(startStr)
    if startIndex>=0:
        startIndex += len(startStr)
    endIndex = content.index(endStr)
    return content[startIndex:endIndex]

def save_pic(pic_name,uid):
    #print(pic_name, id)
    with open(pic_name, 'rb') as f:
        img = f.read()
        f.close()
    hexstr = binascii.b2a_hex(img).decode('utf-8')
    insert_pic = ("insert INTO pic_info (id,pic) VALUES (\"%s\",'%B')" % (uid,img))
    print(insert_pic)
    mysql.mdb_insert(insert_pic)
    return
    #insert_pic = ("call insert_pic(\"%s\",\"%s\")" % (id, img))
    #print(insert_pic)
    #rs, row = mysql.mdb_call(insert_pic)
    #print("创建图片记录%d,id:%s" % (rs[0]['LAST_INSERT_ID()'], id))
    #test
    select_pic = ("call select_pic(\"%s\",%d)" % (uid,1))
    rs, row = mysql.mdb_call(select_pic)
    for i in range(row):
        a =rs[i]['pic']

    with open(pic_name, 'wb') as fp:
        fp.write(a)
        fp.close()
    return

def camera(power, buf):#照片
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2

    total = buf[index] + (buf[index+1]<<8)
    index += 2
    current = buf[index] + (buf[index+1]<<8)
    index += 2

    pic_temp = ("%s\%s\%s.jpg" % (os.path.abspath('.'),"temp",power.monitor))

    if current == 1:
        with open(pic_temp, 'wb+') as f:
            for i in range(index,len(buf)-2):
                f.write(struct.pack("B",buf[i]))
    else:
        with open(pic_temp, 'ab+') as f:
            for i in range(index,len(buf)-2):
                f.write(struct.pack("B",buf[i]))
    f.close()

    if current == (total - 2):
        s = sched.scheduler(time.time, time.sleep)
        s.enter(2, 1, save_pic, (pic_temp,power.monitor,))
        s.run()
    return

def shock(power, buf):#震动报警
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


def distance(power, buf):#激光测距
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


def temperature(power, buf):#导线温度
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

def acceleration(power, buf):#加速度
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2

    acce = acce_t()
    acce.x = float(("%s.%s" % (int(buf[index+1]),int(buf[index]))))
    index +=2
    acce.y = float(("%s.%s" % (int(buf[index+1]),int(buf[index]))))
    index +=2
    acce.z = float(("%s.%s" % (int(buf[index+1]),int(buf[index]))))

    confirm_id(power)
    rse = find_id(power.monitor)
    insert_acceleration =  ("call insert_acceleration(\"%s\",%d,%f,%f,%f)" % (power.monitor,power.collect_time,
                                                         acce.x, acce.y,acce.z))
    rs, row = mysql.mdb_call(insert_acceleration)
    print("创建加速度记录%d,id:%s;加速度:[%s  %s  %s]" % (rs[0]['LAST_INSERT_ID()'], power.monitor,acce.x, acce.y,acce.z))
    return

def atmo(power, buf):#气象
    index = 0
    for i in range(0, 17):
        power.bmonitor += dec2hex_str(buf[i])
    index += 17
    power.collect_time = buf[index] + (buf[index + 1] << 8) + (buf[index + 2] << 16) + (buf[index + 3] << 24)
    index += 4
    power.alarm = buf[index]
    index += 2

    atmos = atmos_t()
    atmos.driver_state_temperature = buf[index] & 1
    atmos.driver_state_wind_direction_speed = buf[index] & 2
    atmos.driver_state_pressure = buf[index] & 4
    atmos.driver_state_compass = buf[index] & 8
    atmos.driver_state_hyetometer = buf[index] & 16

    if(atmos.driver_state_temperature >0 ):atmos.driver_state_temperature=1
    if(atmos.driver_state_wind_direction_speed >0 ):atmos.driver_state_wind_direction_speed=1
    if(atmos.driver_state_pressure >0 ):atmos.driver_state_pressure=1
    if(atmos.driver_state_compass >0 ):atmos.driver_state_compass=1
    if(atmos.driver_state_hyetometer >0 ):atmos.driver_state_hyetometer=1
    index += 2

    atmos.wind_direction = buf[index] + (buf[index+1]<<8)
    index += 2
    #风速
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.wind_speed = convert(s)
    #温度
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.temperature = convert(s)
    #湿度
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.humidity = convert(s)
    #气压
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.pressure = convert(s)
    #电子罗盘
    atmos.compass = buf[index] + (buf[index+1]<<8)
    index += 2
    #降雨状态
    atmos.rainfall_state = buf[index] + (buf[index+1]<<8)
    index += 2
    #降雨强度
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.rainfall_streng = convert(s)
    #累积降雨量
    f1 = hex(buf[index])[2:]
    index += 1
    f2 = hex(buf[index])[2:]
    index += 1
    f3 = hex(buf[index])[2:]
    index += 1
    f4 = hex(buf[index])[2:]
    index += 1
    s = ("%s%s%s%s" % (f4, f3, f2, f1))
    atmos.rainfall_total = convert(s)

    atmos.rainfall_streng_unit = buf[index+1] & 7
    if atmos.rainfall_streng_unit == 1:
        atmos.rainfall_streng_unit = "mm/s"
    elif atmos.rainfall_streng_unit == 2:
        atmos.rainfall_streng_unit = "mm/m"
    else:
        atmos.rainfall_streng_unit = "mm/h"

    state = ("\"%d,%d,%d,%d,%d\"" % (atmos.driver_state_temperature,atmos.driver_state_wind_direction_speed,atmos.driver_state_pressure,
                                 atmos.driver_state_compass,atmos.driver_state_hyetometer))
    val = ("\"%d,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%s\"" % (atmos.wind_direction,atmos.wind_speed,atmos.temperature,
                                              atmos.humidity,atmos.pressure,atmos.compass,
                                              atmos.rainfall_state,atmos.rainfall_streng,atmos.rainfall_total,
                                              atmos.rainfall_streng_unit))
    confirm_id(power)
    rse = find_id(power.monitor)
    insert_atmos =  ("call insert_atmo(\"%s\",%d,%s,%s)" % (power.monitor,power.collect_time,state,val))
    rs, row = mysql.mdb_call(insert_atmos)
    print("创建气象记录%d,id:%s;  [%s   %s]" % (rs[0]['LAST_INSERT_ID()'], power.monitor,state,val))

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
    elif power.message_type == message_type_e.M_ACCE:
        acceleration(power, data);
    elif power.message_type == message_type_e.M_ATMO:
        atmo(power, data);
    elif power.message_type == message_type_e.M_CAME:
        camera(power, data);
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
