
def hex2str(data):
    try:
        hexstr=''
        for i in range(0, len(data)):
            temp = ("%02x" % data[i])
            hexstr = ("%s %s" % (hexstr,temp))
        return hexstr
    except Exception as e:
        print(e)
        return False

