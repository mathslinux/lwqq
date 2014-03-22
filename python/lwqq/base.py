import ctypes

__all__ = [
        'lib'
        'get_library'
        'LwqqBase'
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

def get_library():
    return ctypes.cdll.LoadLibrary("liblwqq.so")

lib = get_library()

