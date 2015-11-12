#coding=utf-8
import user_lib.mysql as mysql
import user_lib.mysocket as mysocket
import user_lib.powerdatadeal as power
def maintain_online_event():#维护设备在线状态
    mysql.mdb_call('call maintain_online()')

def main():
    mysql.mdb_cnn()
    maintain_online_event()
    power.read_id_manage()
    mysocket.tcp_service_start('',8888)
    return

if __name__ == '__main__':
    main()