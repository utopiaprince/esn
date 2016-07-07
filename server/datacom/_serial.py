import serial

class object:
    def __init__(self, port, speed, callback):
        self.port = port;
        self.speed = speed;
        self.handle = '';
        self.recv_callback = callback;

    def open(self):
        self.__recv()

    def __recv(self):
        try:
            self.handle = serial.Serial(self.port, self.speed)
            print("serial open: %s %s" % (self.port.upper(), self.speed))
            while True:
                data = self.handle.read()
                if data == '':
                    continue
                else:
                    self.recv_callback(data)
                    continue
        except Exception as e:
            print(e)
