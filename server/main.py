#coding=utf-8
import os
import user_lib.mysql as mysql
import user_lib.mysocket as mysocket
import user_lib.powerdatadeal as power
def create_pic_dir():#创建存放临时图片的路径
    pic_temp = ("%s\%s" % (os.path.abspath('.'),"temp"))
    if os.path.exists(pic_temp) == False:
        os.makedirs(pic_temp)
    return

def maintain_online_event():#维护设备在线状态
    mysql.mdb_call('call maintain_online()')


def main():
    create_pic_dir()
    mysql.mdb_cnn()
    maintain_online_event()
    power.read_id_manage()
    mysocket.udp_service_start('',8888)
    #mysocket.tcp_service_start('',8888)
    return

if __name__ == '__main__':
    main()
