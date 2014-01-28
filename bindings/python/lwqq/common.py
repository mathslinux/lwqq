from ctypes import POINTER
from ctypes import cdll
from ctypes import c_void_p

__all__ = [
        'lib'
        'get_library'
        'c_object_p'
        ]

c_object_p = c_void_p

def get_library():
    return cdll.LoadLibrary("liblwqq.so")

lib = get_library()

if __name__ == '__main__':
    lib = get_library()
