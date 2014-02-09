from .common import lib,c_object_p

from ctypes import c_voidp,c_int,Structure,c_size_t,CFUNCTYPE,POINTER

__all__ = [
        'Command',
        'vp_func'
        ]

class vp_list(Structure):
    _fields_ = [
            ('st',c_voidp),
            ('cur',c_voidp),
            ('sz',c_size_t)
            ]

vp_func = {
    'void' : (lib.vp_func_void,CFUNCTYPE(None)),
    'p'    : (lib.vp_func_p,CFUNCTYPE(None,c_voidp)),
    'pi'   : (lib.vp_func_pi,CFUNCTYPE(None,c_voidp,c_int)),
    '2p'   : (lib.vp_func_2p,CFUNCTYPE(None,c_voidp,c_voidp)),
    '2pi'  : (lib.vp_func_2pi,CFUNCTYPE(None,c_voidp,c_voidp,c_int)),
    '3p'   : (lib.vp_func_3p,CFUNCTYPE(None,c_voidp,c_voidp,c_voidp)),
    '3pi'  : (lib.vp_func_3pi,CFUNCTYPE(None,c_voidp,c_voidp,c_voidp,c_int)),
    '4p'   : (lib.vp_func_4p,CFUNCTYPE(None,c_voidp,c_voidp,c_voidp,c_voidp)),
    'p_i'  : (lib.vp_func_p_i,CFUNCTYPE(c_int,c_voidp)),
    '2p_i' : (lib.vp_func_2p_i,CFUNCTYPE(c_int,c_voidp,c_voidp)),
    '3p_i' : (lib.vp_func_3p_i,CFUNCTYPE(c_int,c_voidp,c_voidp,c_voidp))
    }


class Command(Structure):

    @classmethod
    def make(self,dsph,callback=None,*args):
        if(callback):
            vp_func_dsph = vp_func[dsph]
        else:
            vp_func_dsph = vp_func['void']
        return lib.vp_make_command(vp_func_dsph[0],vp_func_dsph[1](callback),*args)

    def invoke(self):
        lib.vp_do(self,0)

Command._fields_ = [
        ('dsph',c_object_p),
        ('func',c_object_p),
        ('data',vp_list),
        ('next',POINTER(Command))
        ]


def register_library(lib):
    lib.vp_do.argtypes = [Command,c_voidp]
    lib.vp_do.restype = None

    lib.vp_make_command.argtypes = [c_voidp,c_voidp]
    lib.vp_make_command.restype = Command

register_library(lib)


