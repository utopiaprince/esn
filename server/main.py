#coding=utf-8
import os
import user_lib.mysql as mysql
import user_lib.powerdatadeal as power
import time
import threading
import user_lib.globalval as globalval
import datacom
import sys

threads = []
forwardsend = False
dip='192.168.1.100'
dport = 8065

def create_pic_dir():#创建存放临时图片的路径
    pic_temp = ("%s\%s" % (os.path.abspath('.'),"temp"))
    if os.path.exists(pic_temp) == False:
        os.makedirs(pic_temp)
    return

def maintain_online_event():#维护设备在线状态
    mysql.mdb_call('call maintain_online()')

def tcp_client(dhost, port):
    globalval.tcp_client_handle = datacom._socket.tcp_client(dhost, port, None)
    globalval.tcp_client_handle.connect(sys.maxsize)

def tcp_server(ip, port):
    o_tcp_server = datacom._socket.tcp_server(ip,port,power.recv_data)
    o_tcp_server.listen()

if __name__ == '__main__':
    create_pic_dir()
    mysql.mdb_cnn()
    maintain_online_event()
    power.read_id_manage()

    t1 = threading.Thread(target=tcp_server, args=('', 8888))
    t2 = threading.Thread(target=tcp_client, args=(dip, dport))
    threads.append(t1)
    threads.append(t2)
    for t in threads:
        t.setDaemon(True)
        t.start()

    while True:
        time.sleep(10)
