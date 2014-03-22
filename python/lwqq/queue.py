import ctypes
from ctypes import c_void_p,POINTER,pointer,cast

__all__ = ['TAILQ_HEAD', 'TAILQ_ENTRY', 'LIST_HEAD', 'LIST_ENTRY']

class TAILQ_ENTRY(ctypes.Structure):
    _fields_ = [
    ('tqe_next',POINTER(c_void_p)),
    ('tqe_prev',POINTER(POINTER(c_void_p)))
    ]
# must store in heap
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
        self.ref[0].tqh_first = None
        self.ref[0].tqh_last = pointer(self.ref[0].tqh_first)
    def foreach(self):
        item = cast(self.ref[0].tqh_first,c_void_p).value
        while item:
            yield item
            entry = TAILQ_ENTRY.from_address(item+self.entries)
            item = entry.tqe_next
    def insert_tail(self,item):
        elem = TAILQ_ENTRY.from_buffer(item[0],self.entries)
        elem.tqe_next = None
        elem.tqe_prev = self.ref[0].tqh_last
        tqh_last = c_void_p.from_buffer(self.ref[0].tqh_last.contents)
        tqh_last.value = ctypes.addressof(item[0])
        self.ref.tqh_last = pointer(elem.tqe_next)

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

