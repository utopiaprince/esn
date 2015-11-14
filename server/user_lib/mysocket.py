import socketserver
import user_lib.powerdatadeal as power

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

class tcp_service(socketserver.BaseRequestHandler):
    def handle(self):
        print ('connected from:', self.client_address)
        remain = []
        array = []
        while True:
            data = self.request.recv(1024).strip()
            if not data:break
            array=[]
            if len(remain)!=0:
                array.append(remain)
            #if globalval.isWindowsSystem() == True:
            temp = data
            #if globalval.isLinuxSystem() == True:
                #temp = convert2hex(data)
            if False == temp:
                continue
            array.extend(temp)
            array_len = len(array)
            remainlen = power.recv_data(array,self.client_address)
            remain=[]
            if remainlen != 0:
                remain = appand_remain(array,array_len,remainlen)
            #self.request.sendall(data)
        print ("closed from:", self.client_address)
        self.finish()

def tcp_service_start(host,port):
    addr = (host, port)
    tcpServ = socketserver.ThreadingTCPServer(addr, tcp_service)
    print ('waiting for connection...')
    tcpServ.serve_forever()