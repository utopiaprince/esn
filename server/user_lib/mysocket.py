import socketserver
import socket
import user_lib.powerdatadeal as power
import traceback
import time

def convert2hex(d):
    try:
        hex_array = []
        for i in range(0,len(d)):
            hexstr =  "%s" % d[i].encode('hex')
            dec = int(hexstr, 16)
            hex_array.append(dec)
        return hex_array
    except:
        return False

def appand_remain(array,array_len,remainlen):
    remain = []
    for i in range(array_len -remainlen ,array_len):
        remain.append(array[i])
    return remain

class tcp_service(socketserver.BaseRequestHandler):
    def handle(self):
        print ('connected from:', self.client_address)
        remain = []
        while True:
            try:
                data = self.request.recv(1024).strip()
                if not data:break
                array=[]
                if len(remain)!=0:
                    array.extend(remain)
                #if globalval.isWindowsSystem() == True:
                temp = data
                #if globalval.isLinuxSystem() == True:
                    #temp = convert2hex(data)
                if False == temp:
                    continue
                array.extend(temp)
                array_len = len(array)
                remainlen = power.recv_data(array,self)
                if remainlen != 0:
                    remain = appand_remain(array,array_len,remainlen)
                else :
                    remain = []
                #self.request.sendall(data)
            except:
                traceback.print_exc()
                break
        print ("closed from:", self.client_address)
        self.finish()

class udp_service(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            data = self.request[0].strip()
            power.recv_data(data,self.client_address)
            #self.request.sendall(data)
        except:
            traceback.print_exc()

class tcp_server:
    def __init__(self,host,port):
        self.host = host
        self.port = port
    def start(self):
        addr = (self.host, self.port)
        server = socketserver.ThreadingTCPServer(addr, tcp_service)
        print ('[TCPS:%d] waiting for connection...' % self.port)
        server.serve_forever()

class udp_server:
    def __init__(self,host,port):
        self.host = host
        self.port = port
    def start(self):
        addr = (self.host, self.port)
        server = socketserver.UDPServer(addr, udp_service)
        print ('[UDPS:%d] waiting for connection...' % self.port)
        server.serve_forever()

class tcp_client:
     def __init__(self,dhost,dport):
        self.client = 0
        self.dhost = dhost
        self.dport = dport
        self.iscnnect = False
     def start(self):
        addr = (self.dhost, self.dport)
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            self.iscnnect = False
            try:
                if self.client._closed == True:
                    self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.client.connect(addr)
                self.iscnnect = True
                print("[TCP:%s:%d] connect success" % (self.dhost,self.dport))
                while True:
                    try:
                        data = self.client.recv(1024).strip()
                        if not data:
                            print("[TCP:%s:%d] connect closed" % (self.dhost,self.dport))
                            break
                    except:
                        break
                self.iscnnect = False
                self.client.close()
                time.sleep(5)
            except:
                print("[TCP:%s:%d] connect fail" % (self.dhost,self.dport))
            self.iscnnect = False
            time.sleep(10)