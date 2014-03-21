import ctypes

__all__ = ['TAILQ_HEAD', 'TAILQ_ENTRY', 'LIST_HEAD', 'LIST_ENTRY']

class TAILQ_ENTRY(ctypes.Structure):
    _fields_ = [
    ('tqe_next',ctypes.c_void_p),
    ('tqe_prev',ctypes.c_void_p)
    ]

class TAILQ_HEAD():
    class T(ctypes.Structure):
        _fields_ = [
            ('tqh_first',ctypes.c_void_p),
            ('tqh_last',ctypes.c_void_p)
        ]
    ref = None
    entries = None
    def __init__(self,ref,entries):
        self.ref = ref
        self.entries = entries.offset
    def foreach(self):
        item = self.ref.tqh_first
        while item:
            yield item
            entry = TAILQ_ENTRY.from_address(item+self.entries)
            item = entry.tqe_next
        pass

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


