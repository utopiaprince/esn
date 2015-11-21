import socketserver
import user_lib.powerdatadeal as power
import traceback

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
                remainlen = power.recv_data(array,self.client_address)
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
            remainlen = power.recv_data(data,self.client_address)
            #self.request.sendall(data)
        except:
            traceback.print_exc()

def tcp_service_start(host,port):
    addr = (host, port)
    server = socketserver.ThreadingTCPServer(addr, tcp_service)
    print ('[TCP:%d] waiting for connection...' % port)
    server.serve_forever()

def udp_service_start(host,port):
    addr = (host, port)
    server = socketserver.UDPServer(addr, udp_service)
    print ('[UDP:%d] waiting for connection...' % port)
    server.serve_forever()