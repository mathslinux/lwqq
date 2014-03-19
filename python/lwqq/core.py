from ctypes import CFUNCTYPE,POINTER,Structure,c_char_p,pointer,c_long,c_voidp,c_ulong,c_int,cast,byref
from .common import lib
from .vplist import Command


__all__ = [
        'Event',
        'Events',
        'Arguments',
        ]

class Event(object):
    class T(Structure):
        _fields_ = [
                ('result',c_int),
                ('failcode',c_int),
                ('lc',c_voidp)
                ]
    PT = POINTER(T)
    ptr_ = None

    def __init__(self,event):
        self.ptr_ = event

    @property
    def raw(self):
        return self.ptr_[0]

    @classmethod
    def new(self,http_req):
        return Event(lib.lwqq_async_event_new(http_req))

    def addListener(self,closure):
        lib.lwqq_async_add_event_listener(self.ptr_,Command.make(closure))
        return self

    def finish(self):
        lib.lwqq_async_event_finish(self.ptr_)
        return self

class Evset(object):
    evset_ = None

    def __init__(self,async_evset):
        evset_ = async_evset

    @classmethod
    def new(self):
        return Evset(lib.lwqq_async_evset_new())

    def addListener(self,closure):
        lib.lwqq_async_add_evset_listener(self.evset_,Command.make(closure))
        return self


class Events():
    class T(Structure):
        _fields_ = [
                ('start_login',Command),
                ('login_complete',Command),
                ('pull_msg',Command),
                ('pull_lost',Command),
                ('upload_fail',Command),
                ('new_friend',Command),
                ('new_group',Command),
                ('need_verify',Command),
                ('delete_group',Command),
                ('group_member_chg',Command),
                ]
    PT = POINTER(T)
    ref = None
    def __init__(self,ref): self.ref = ref
    @property
    def start_login(self): return self.ref[0].start_login
    @property
    def login_complete(self): return self.ref[0].login_complete
    @property
    def pull_msg(self): return self.ref[0].pull_msg
    @property
    def pull_lost(self): return self.ref[0].pull_lost
    @property
    def upload_fail(self): return self.ref[0].upload_fail
    @property
    def new_friend(self): return self.ref[0].new_friends
    @property
    def new_group(self): return self.ref[0].new_group
    @property
    def need_verify(self): return self.ref[0].need_verify
    @property
    def delete_group(self): return self.ref[0].delete_group
    @property
    def group_member_chg(self): return self.ref[0].group_member_chg

class Arguments():
    class T(Structure):
        _fields_ = [
                ('login_ec',c_int),
                ('buddy',c_voidp),
                ('group',c_voidp),
                ('vf_image',c_voidp),
                ('delete_group',c_voidp),
                ('serv_id',c_char_p),
                ('content',c_voidp),
                ('err',c_int)
                ]
    PT = POINTER(T)
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)

    @property 
    def login_ec(self): return pointer(c_int.from_buffer(self.ref[0],self.T.login_ec.offset))
    @property
    def buddy(self): return pointer(c_voidp.from_buffer(self.ref[0],self.T.buddy.offset))
    @property
    def group(self): return pointer(c_voidp.from_buffer(self.ref[0],self.T.group.offset))
    @property
    def vf_image(self): return pointer(c_voidp.from_buffer(self.ref[0],self.T.vf_image.offset))
    @property
    def delete_group(self): return pointer(c_voidp.from_buffer(self.ref[0],self.T.delete_group.offset))
    @property
    def serv_id(self): return pointer(c_char_p.from_buffer(self.ref[0],self.T.serv_id.offset))
    @property
    def content(self): return pointer(c_voidp.from_buffer(self.ref[0],self.T.content.offset))
    @property
    def err(self): return pointer(c_int.from_buffer(self.ref[0],self.T.err.offset))


def register_library(lib):
    #============LOW LEVEL ASYNC PART===============#
    lib.lwqq_add_event_listener.argtypes = [POINTER(Command),Command]

    lib.lwqq_async_event_new.argtypes = [c_voidp]
    lib.lwqq_async_event_new.restype = Event.PT

    lib.lwqq_async_event_finish.argtypes = [Event.PT]

    lib.lwqq_async_add_event_listener.argtypes = [Event.PT,Command]

    lib.lwqq_async_add_evset_listener.argtypes = [c_voidp,Command]

    lib.lwqq_async_evset_new.argtypes = []
    lib.lwqq_async_evset_new.restype = c_voidp

    lib.lwqq_async_evset_free.argtypes = [c_voidp]


register_library(lib)
