import ctypes

__all__ = [
        'lib'
        'get_library'
        'LwqqBase',
        's_malloc',
        's_malloc0',
        's_calloc',
        's_realloc',
        's_strdup',
        's_atol',
        ]


class LwqqBase():
    ref = None
    def __init__(self,ref):
        self.ref = ctypes.cast(ref.ref,self.PT) if hasattr(ref,'ref') else ctypes.cast(ref,self.PT)
    def __str__(self):
        ret = ""
        for field,_ in self.T._fields_:
            val = getattr(self.ref[0],field)
            val = val.decode() if isinstance(val,bytes) else str(val)
            ret +=field+':\t'+ val +'\n'
        return ret
    @classmethod
    def new(cls,malloc=False,**kw):
        ins = None
        if malloc:
            ins = ctypes.cast(s_malloc0(ctypes.sizeof(cls.T)),cls.PT)
            ref = ins[0]
            for k,v in kw.items():
                setattr(ref,k,v)
        else:
            ins = ctypes.byref(cls.T(**kw))
        return cls(ins)

def get_library():
    return ctypes.cdll.LoadLibrary("liblwqq.so")

def register_library(lib):
    lib.s_malloc.argtypes = [ctypes.c_size_t]
    lib.s_malloc.restype = None
    lib.s_malloc0.argtypes = [ctypes.c_size_t]
    lib.s_malloc0.restype = ctypes.c_voidp
    lib.s_calloc.argtypes = [ctypes.c_size_t,ctypes.c_size_t]
    lib.s_calloc.restype = ctypes.c_voidp
    lib.s_realloc.argtypes = [ctypes.c_void_p,ctypes.c_size_t]
    lib.s_realloc.restype = ctypes.c_voidp
    lib.s_strdup.argtypes = [ctypes.c_char_p]
    lib.s_strdup.restype = ctypes.POINTER(ctypes.c_char)
    lib.s_atol.argtypes = [ctypes.c_char_p,ctypes.c_long]
    lib.s_atol.restype = ctypes.c_long

lib = get_library()
register_library(lib)

s_malloc = lib.s_malloc
s_malloc0 = lib.s_malloc0
s_calloc = lib.s_calloc
s_realloc = lib.s_realloc
s_strdup = lib.s_strdup
s_atol = lib.s_atol

