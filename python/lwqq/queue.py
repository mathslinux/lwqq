import ctypes

__all__ = ['TAILQ_HEAD','TAILQ_ENTRY']

class TAILQ_HEAD(ctypes.Structure):
    _field_ = [
            ('tqh_first',ctypes.c_void_p),
            ('tqh_last',ctypes.c_void_p)
            ]

class TAILQ_ENTRY(ctypes.Structure):
    _field_ = [
            ('tqe_next',ctypes.c_void_p),
            ('tqe_prev',ctypes.c_void_p)
            ]
