from .common import get_library

from ctypes import c_long,c_char_p,c_int,c_voidp,c_size_t
from ctypes import Structure

lib = get_library()

class HttpHandle(Structure):
    class Proxy(Structure):
        _fields_ = [
                ('type',c_int),
                ('host',c_char_p),
                ('port',c_int),
                ('username',c_char_p),
                ('password',c_char_p)
                ]
    _fields_ = [
            ('proxy',Proxy),
            ('quit',c_int),
            ('synced',c_int)
            ]

