from .common import get_library

from ctypes import Structure,CFUNCTYPE

lib = get_library()

class Async(object):
    class T(Structure):
        _fields_ = [
                ('loop_create',CFUNCTYPE(None)),
                ('loop_run',CFUNCTYPE(None)),
                ]
