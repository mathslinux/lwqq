from .base import lib
import ctypes

__all__ = [
        's_malloc',
        's_malloc0',
        's_calloc',
        's_realloc',
        's_strdup',
        's_atol',
        ]

s_malloc = lib.s_malloc
s_malloc0 = lib.s_malloc0
s_calloc = lib.s_calloc
s_realloc = lib.s_realloc
s_strdup = lib.s_strdup
s_atol = lib.s_atol

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

register_library(lib)
