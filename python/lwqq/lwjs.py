from .base import lib
from .smemory import s_strdup
from .core import has_feature
from .types import Features
import ctypes

__all__ =['Lwjs']

class Lwjs():
    js = None
    jso = None
    hashref = None #keep reference
    @classmethod
    def hashfunc(self,uin,ptwebqq,data):
        uinp = ctypes.c_char_p(uin)
        ptwebqqp = ctypes.c_char_p(ptwebqq)
        datap = ctypes.c_voidp(data)
        pointer = lib.lwqq_js_hash(uinp,ptwebqqp,datap)
        return pointer
    def __init__(self):
        self.js = lib.lwqq_js_init()
    def __del__(self):
        lib.lwqq_js_close(self.js)
    def open(self,filename):
        jso = lib.lwqq_js_load(self.js,ctypes.c_char_p(filename))
    def close(self):
        lib.lwqq_js_unload(self.js,jso)
    def load(self,content):
        lib.lwqq_js_load_buffer(self.js,ctypes.c_char_p(content))
        

def register_library(lib):
    lib.lwqq_js_init.restype = ctypes.c_voidp
    lib.lwqq_js_close.argtypes = [ctypes.c_voidp]
    if has_feature(Features.WITH_MOZJS):
        lib.lwqq_js_load.argtypes = [ctypes.c_voidp,ctypes.c_char_p]
        lib.lwqq_js_load.restype = ctypes.c_voidp
        lib.lwqq_js_unload.argtypes = [ctypes.c_void_p,ctypes.c_void_p]
        lib.lwqq_js_load_buffer.argtypes = [ctypes.c_void_p,ctypes.c_char_p]

        lib.lwqq_js_hash.argtypes = [ctypes.c_char_p,ctypes.c_char_p,ctypes.c_void_p]
        lib.lwqq_js_hash.restype = ctypes.c_void_p

register_library(lib)
