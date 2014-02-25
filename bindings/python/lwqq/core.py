from .common import get_library
from .common import c_object_p
from .vplist import Command

from .http import HttpHandle

from . import enumerations

from ctypes import c_long,c_char_p,c_int,c_voidp,c_ulong
from ctypes import Structure,CFUNCTYPE,POINTER,cast,pointer,byref

__all__ = [
        'Event',
        'Events',
        'Arguments',
        'Lwqq',
        'Buddy',
        'SimpleBuddy'
        ]

lib = get_library()

class Event(object):
    class T(Structure):
        _fields_ = [
                ('result',c_int),
                ('failcode',c_int),
                ('lc',c_object_p)
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


class Events(Structure):
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
class Arguments(Structure):
    _fields_ = [
            ('login_ec',c_int),
            ('buddy',c_object_p),
            ('group',c_object_p),
            ('vf_image',c_object_p),
            ('delete_group',c_object_p),
            ('serv_id',c_char_p),
            ('content',c_object_p),
            ('err',c_int)
            ]


class Lwqq(object):
    DISPATCH_T = CFUNCTYPE(None,Command,c_ulong)
    class T(Structure):
        _fields_ = [
                ('username',c_char_p),
                ('password',c_char_p),
                ('version',c_char_p),
                ('clientid',c_char_p),
                ('seskey',c_char_p),
                ('cip',c_char_p),
                ('index',c_char_p),
                ('port',c_char_p),
                ('vfwebqq',c_char_p),
                ('psessionid',c_char_p),
                ('last_err',c_char_p),
                ('gface_key',c_char_p),
                ('gface_sig',c_char_p),
                ('login_sig',c_char_p),
                ('stat',c_long),
                ('error_description',c_char_p),
                ('new_ptwebqq',c_char_p),

                ('myself',c_object_p),
                ('vc',c_object_p),
                ('events',POINTER(Events)),
                ('args',POINTER(Arguments)),
                ('dispatch',CFUNCTYPE(None,Command,c_ulong))
                ]
    PT = POINTER(T)
    lc_ = None

    def __init__(self,username,password):
        u = c_char_p(username)
        p = c_char_p(password)
        self.lc_ = lib.lwqq_client_new(u,p)

    def __del_(self):
        lib.lwqq_client_free(self.lc_)

    @property
    def raw(self):
        return self.lc_[0];
    @property
    def events(self):
        return lib.lwqq_client_get_events(self.lc_)[0]
    @property
    def args(self):
        return lib.lwqq_client_get_args(self.lc_)[0]

    def setDispatcher(self,dispatcher):
        self.raw.dispatch = self.DISPATCH_T(dispatcher)

    def dispatch(self,cmd,delay = 50):
        self.raw.dispatch(cmd,delay)

    def addListener(self,events,called):
        lib.lwqq_add_event_listener(byref(events),called)

    def http(self):
        return lib.lwqq_get_http_handle(self.lc_)[0]

    def sync(self,yes):
        self.http().synced = yes

    def login(self,status):
        lib.lwqq_login(self.lc_,status,0)

    def logout(self):
        lib.lwqq_logout(self.lc_,0)

    def relink(self):
        return Event(lib.lwqq_relink(self.lc_))

    @classmethod
    def time(cls):
        return lib.lwqq_time()

    @classmethod
    def log_level(cls,level):
        lib.lwqq_log_set_level(level)

class Buddy(object):
    ptr_ = None

    def __init__(self):
        self.ptr_ = lib.lwqq_buddy_new()

    def __del__(self):
        lib.lwqq_buddy_free(self.ptr_)

class SimpleBuddy(object):
    ptr_ = None

    def __init__(self):
        self.ptr_ = lib.lwqq_simple_buddy_new()

    def __del__(self):
        lib.lwqq_simple_buddy_free(self.ptr_)

def register_library(lib):
    #============LOW LEVEL ASYNC PART===============#

    lib.lwqq_client_get_events.argtypes= [Lwqq.PT]
    lib.lwqq_client_get_events.restype = POINTER(Events)
    lib.lwqq_client_get_args.argtypes= [Lwqq.PT]
    lib.lwqq_client_get_args.restype = POINTER(Arguments)

    lib.lwqq_add_event_listener.argtypes = [POINTER(Command),Command]
    lib.lwqq_add_event_listener.restype = None

    lib.lwqq_async_event_new.argtypes = [c_object_p]
    lib.lwqq_async_event_new.restype = Event.PT

    lib.lwqq_async_event_finish.argtypes = [Event.PT]
    lib.lwqq_async_event_finish.restype = None

    lib.lwqq_async_add_event_listener.argtypes = [Event.PT,Command]
    lib.lwqq_async_add_event_listener.restype = None

    lib.lwqq_async_add_evset_listener.argtypes = [c_object_p,Command]
    lib.lwqq_async_add_evset_listener.restype = None

    lib.lwqq_async_evset_new.argtypes = []
    lib.lwqq_async_evset_new.restype = c_object_p

    lib.lwqq_async_evset_free.argtypes = [c_object_p]
    lib.lwqq_async_evset_free.restype = None

    lib.lwqq_get_http_handle.argtypes = [c_object_p]
    lib.lwqq_get_http_handle.restype = POINTER(HttpHandle)


    lib.lwqq_client_new.argtypes = [c_char_p,c_char_p]
    lib.lwqq_client_new.restype = Lwqq.PT

    lib.lwqq_client_free.argtypes = [Lwqq.PT]
    lib.lwqq_client_free.restype = None

    lib.lwqq_buddy_new.argtypes = []
    lib.lwqq_buddy_new.restype = c_object_p

    lib.lwqq_buddy_free.argtypes = [c_object_p]
    lib.lwqq_buddy_free.restype = None

    lib.lwqq_simple_buddy_new.argtypes = []
    lib.lwqq_simple_buddy_new.restype = c_object_p

    lib.lwqq_simple_buddy_free.argtypes = [c_object_p]
    lib.lwqq_simple_buddy_free.restype = None

    lib.lwqq_login.argtypes = [Lwqq.PT,c_int,c_int]
    lib.lwqq_login.restype = None

    lib.lwqq_logout.argtypes = [Lwqq.PT,c_int]
    lib.lwqq_logout.restype = None

    lib.lwqq_relink.argtypes = [Lwqq.PT]
    lib.lwqq_relink.restype = Event.PT

    lib.lwqq_log_set_level.argtypes = [c_int]
    lib.lwqq_log_set_level.restype = None


    lib.lwqq_time.argtypes = []
    lib.lwqq_time.restype = c_long

#def register_enumerations():
#    for name, value in enumerations.Status:
#        Status.register(name, value)

register_library(lib)
#register_enumerations()
