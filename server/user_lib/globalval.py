import platform
global id_list
global tcp_client_handle


def isWindowsSystem():
    return 'Windows' in platform.system()

def isLinuxSystem():
    return 'Linux' in platform.system()
