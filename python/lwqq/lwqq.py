from ctypes import CFUNCTYPE,POINTER,Structure,c_char_p,pointer,c_long,c_voidp,c_ulong,c_int,cast,byref
import ctypes
from .common import lib
from .vplist import Command
from .core import *
from .http import HttpHandle

from .lwjs import *

__all__ = [
        'Lwqq',
        'Buddy',
        'SimpleBuddy'
        ]

HASHFUNC = CFUNCTYPE(c_voidp,c_char_p,c_char_p,c_voidp)

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

                ('myself',c_voidp),
                ('vc',c_voidp),
                ('events',Events.PT),
                ('args',Arguments.PT),
                ('dispatch',CFUNCTYPE(None,Command,c_ulong))
                ]
    PT = POINTER(T)
    lc_ = None
    events_ref = [] #keep reference registerd events
    username = None
    password = None

    def __init__(self,username,password):
        self.username = username
        self.password = password
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
        return Events(lib.lwqq_client_get_events(self.lc_))
    @property
    def args(self):
        return Arguments(lib.lwqq_client_get_args(self.lc_))

    def setDispatcher(self,dispatcher):
        self.raw.dispatch = self.DISPATCH_T(dispatcher)

    def dispatch(self,cmd,delay = 50):
        self.raw.dispatch(cmd,delay)

    def addListener(self,events,called):
        self.events_ref.append(called)
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

    def get_friends_info(self,hashfunc,data):
        return lib.lwqq_info_get_friends_info(self.lc_,HASHFUNC(hashfunc),data)

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
    lib.lwqq_client_new.argtypes = [c_char_p,c_char_p]
    lib.lwqq_client_new.restype = Lwqq.PT

    lib.lwqq_client_free.argtypes = [Lwqq.PT]

    lib.lwqq_buddy_new.argtypes = []
    lib.lwqq_buddy_new.restype = c_voidp

    lib.lwqq_buddy_free.argtypes = [c_voidp]

    lib.lwqq_simple_buddy_new.argtypes = []
    lib.lwqq_simple_buddy_new.restype = c_voidp

    lib.lwqq_simple_buddy_free.argtypes = [c_voidp]

    lib.lwqq_login.argtypes = [Lwqq.PT,c_int,c_int]

    lib.lwqq_logout.argtypes = [Lwqq.PT,c_int]

    lib.lwqq_relink.argtypes = [Lwqq.PT]
    lib.lwqq_relink.restype = Event.PT

    lib.lwqq_log_set_level.argtypes = [c_int]

    lib.lwqq_time.argtypes = []
    lib.lwqq_time.restype = c_long

    lib.lwqq_client_get_events.argtypes= [Lwqq.PT]
    lib.lwqq_client_get_events.restype = Events.PT
    lib.lwqq_client_get_args.argtypes= [Lwqq.PT]
    lib.lwqq_client_get_args.restype = Arguments.PT
    lib.lwqq_get_http_handle.argtypes = [c_voidp]
    lib.lwqq_get_http_handle.restype = POINTER(HttpHandle)

    lib.lwqq_info_get_friends_info.argtypes = [Lwqq.PT,HASHFUNC,c_voidp]
    lib.lwqq_info_get_friends_info.restype = Event.PT

register_library(lib)
