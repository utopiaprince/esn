# coding=utf-8

import socket
import time
import threading

def local_ip():
    return socket.gethostbyname(socket.gethostname())


class tcp_client:
    def __init__(self, ip, port, callback):
        self.dhost = ip
        self.port = port
        self.handle = ''
        self.connect_max = 0
        self.recv_callback = callback
        self.running = True
        self.timeout = 10

    def send(self, data, addr):
        try:
            if self.handle.closed:
                return
            self.handle.sendto(data, addr)
        except Exception as e:
            print(e)

    def connect(self, num):
        self.connect_max = num
        self.__start()

    def __start(self):
        connect_num = 0
        self.handle = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while self.running and (connect_num < self.connect_max):
            try:
                connect_num += 1
                self.handle.connect((self.dhost, self.port))
                print("[TCP:%s:%d] connect success" % (self.dhost, self.port))
                while self.running:
                    try:
                        data = self.handle.recv(1024).strip()
                        if not data:
                            print("[TCP:%s:%d] connect closed" % (self.dhost, self.port))
                            self.handle.close()
                            break
                        else:
                            self.recv_callback(data)
                    except Exception as e:
                        print(("[TCP:%s:%d]" % (self.dhost, self.port)), e)
                        self.handle.close()
            except Exception as e:
                print(("[TCP:%s:%d]" % (self.dhost, self.port)), e)
            time.sleep(self.timeout)

        if connect_num >= self.connect_max:
            print("[TCP:%s:%d] connect over range" % (self.dhost, self.port))
        self.handle.close()


class tcp_server:
    def __init__(self, ip, port, callback):
        self.ip = ip
        self.port = port
        self.handle = ''
        self.recv_callback = callback

    def listen(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            s.bind((self.ip, self.port))
        except Exception as e:
            print(e)
            s.close()
            return
        s.listen(50)
        print('[TCPS:%d] waiting for connection...' % self.port)
        while True:
            conn, addr = s.accept()
            conn.settimeout(300)
            t = threading.Thread(target=self._recv_handle, args=(conn, addr))
            t.setDaemon(True)
            t.start()

    def _recv_handle(self, conn, addr):
        print('connected from:', addr)
        client_address = addr
        remain = []
        while True:
            try:
                data = conn.recv(1024)
                if not data: break
                array=[]
                if len(remain)!=0:
                    array.extend(remain)
                array.extend(data)
                array_len = len(array)
                remainlen = self.recv_callback(array,(self.ip,self.port))
                if remainlen != 0:
                    remain=[]
                    remain.extend(array[(array_len-remainlen):])
                else :
                    remain = []
            except socket.timeout as e:
                print(client_address, e)
                return
        print("closed from:", client_address)
        conn.close()


class udp:
    def __init__(self, ip, port, callback):
        self.ip = ip
        self.port = port
        self.handle = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.recv_callback = callback;

    def send(self, data, addr):
        self.handle.sendto(data.encode(encoding='utf_8'), addr)

    def connect(self):
        print('[UDP:%d] start...' % self.port)
        while True:
            try:
                data,addr = self.handle.recvfrom(1024)
                if not data:break
                self.recv_callback(data)
            except Exception as e:
                print(e)
                break;
        self.handle.close()

    def listen(self):
        try:
            self.handle.bind((self.ip, self.port))
        except Exception as e:
            print(e)
            self.handle.close()
            return
        print('[UDPS:%d] start...' % self.port)
        while True:
            try:
                data, addr = self.handle.recvfrom(1024)
                if not data: break
                self.recv_callback(data)
                # self.send(data,addr)
            except Exception as e:
                print(e)
                break;
        self.handle.close()
