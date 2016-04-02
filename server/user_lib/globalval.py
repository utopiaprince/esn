import platform
import sys
global id_list
global tcp_client_handle


def isWindowsSystem():
    return 'Windows' in platform.system()

def isLinuxSystem():
    return 'Linux' in platform.system()

def get_cur_info():
    f = sys._getframe().f_back
    print('[ERROR]:%s [LINE]:%s' % (f.f_code.co_name, f.f_lineno))