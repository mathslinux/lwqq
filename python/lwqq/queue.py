from .base import lib
import ctypes
from ctypes import c_void_p,POINTER,pointer,cast

__all__ = ['TAILQ_HEAD', 'TAILQ_ENTRY', 'LIST_HEAD', 'LIST_ENTRY']

class TAILQ_ENTRY(ctypes.Structure):
    _fields_ = [
    ('tqe_next',POINTER(c_void_p)),
    ('tqe_prev',POINTER(POINTER(c_void_p)))
    ]

class TAILQ_HEAD():
    class T(ctypes.Structure):
        _fields_ = [
            ('tqh_first',POINTER(c_void_p)),
            ('tqh_last',POINTER(POINTER(c_void_p)))
        ]
    PT = POINTER(T)
    ref = None
    entries = None
    def __init__(self,ref,entries):
        self.ref = pointer(ref)
        self.entries = entries.offset
    def init(self):
        print(cast(self.ref[0].tqh_first,c_void_p),cast(self.ref[0].tqh_last,c_void_p))
        #lib.tailq_init(self.ref)
        self.ref[0].tqh_first = None
        self.ref[0].tqh_last = pointer(self.ref[0].tqh_first)
        print(cast(self.ref[0].tqh_first,c_void_p),cast(self.ref[0].tqh_last,c_void_p))
    def foreach(self):
        item = cast(self.ref[0].tqh_first,c_void_p).value
        while item:
            yield item
            entry = TAILQ_ENTRY.from_address(item+self.entries)
            item = entry.tqe_next
    def insert_tail(self,item):
        print(cast(self.ref[0].tqh_first,c_void_p),cast(self.ref[0].tqh_last,c_void_p))
        ptr = cast(item,c_void_p)
        lib.tailq_insert_tail(self.ref,item,ptr.value+self.entries)
        print(cast(self.ref[0].tqh_first,c_void_p),cast(self.ref[0].tqh_last,c_void_p))
        #elem.tqe_next = None
        #elem.tqe_prev = self.ref.tqh_last
        #tqh_last = c_void_p.from_buffer(self.ref.tqh_last.contents)
        #tqh_last.value = ctypes.addressof(item[0])
        #self.ref.tqh_last = pointer(elem.tqe_next)
        #tqe_prev = ctypes.c_void_p.from_buffer(elem,TAILQ_ENTRY.tqe_next.offset)
        #tqe_prev.value = self.ref.tqh_last.value
        #tqh_last = ctypes.c_void_p.from_address(self.ref.tqh_last.value)
        #tqh_last.value = ctypes.addressof(elem)
        #self.ref.tqh_last.value = ctypes.addressof(elem)+TAILQ_ENTRY.tqe_next.offset

        #elem.tqe_next = None
        #elem.tqe_prev.value = self.ref.tqh_last.value
        #self.ref.tqh_last[0] = ctypes.cast(ctypes.pointer(elem),ctypes.c_void_p)
        #self.ref.tqh_last = pointer(ctypes.c_void_p.from_buffer(elem,TAILQ_ENTRY.tqe_next.offset))
        ##ctypes.addressof(elem)+TAILQ_ENTRY.T.tqe_next.offset

class LIST_ENTRY(ctypes.Structure):
    _fields_ = [
            ('le_next',ctypes.c_void_p),
            ('le_prev',ctypes.c_void_p)
            ]

class LIST_HEAD():
    class T(ctypes.Structure):
        _fields_ = [('le_first',ctypes.c_void_p)]
    ref = None
    entries = None
    def __init__(self,ref,entries):
        self.ref = ref
        self.entries = entries.offset
    def foreach(self):
        item = self.ref.le_first
        while item:
            yield item
            entry = LIST_ENTRY.from_address(item+self.entries)
            item = entry.le_next
        pass

def register_library(lib):
    lib.tailq_init.argtypes = [TAILQ_HEAD.PT]
    lib.tailq_insert_tail.argtypes = [TAILQ_HEAD.PT,c_void_p,c_void_p]
    pass
register_library(lib)
