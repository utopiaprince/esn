#coding=utf-8
import os
import user_lib.mysql as mysql
from user_lib.mysocket import (tcp_server,tcp_client,udp_server)
import user_lib.powerdatadeal as power
import time
import threading
import user_lib.globalval as globalval

forwardsend = False
dip='58.214.236.152'
dport = 8065

def create_pic_dir():#创建存放临时图片的路径
    pic_temp = ("%s\%s" % (os.path.abspath('.'),"temp"))
    if os.path.exists(pic_temp) == False:
        os.makedirs(pic_temp)
    return

def maintain_online_event():#维护设备在线状态
    mysql.mdb_call('call maintain_online()')

def socket_start(host,port):
    globalval.tcp_client_handle = 0
    if forwardsend == True: #这里启动转发数据连接
        globalval.tcp_client_handle = tcp_client(dip,dport)
        t = threading.Thread(target=globalval.tcp_client_handle.start)
        t.setDaemon(True)
        t.start();

    tcp_server('',8888).start()

def main():
    create_pic_dir()
    mysql.mdb_cnn()
    maintain_online_event()
    power.read_id_manage()

    t = threading.Thread(target=socket_start,args=('',8888))
    t.setDaemon(True)
    t.start();

    while True:
        time.sleep(10)
    return

if __name__ == '__main__':
    main()
