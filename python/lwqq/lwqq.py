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
DISPATCH_FUNC = CFUNCTYPE(None,Command,c_ulong)

class Lwqq(object):
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
                ('error_description',c_char_p),
                ('new_ptwebqq',c_char_p),

                ('myself',c_voidp),
                ('vc',c_voidp),
                ('events',Events.PT),
                ('args',Arguments.PT),
                ('msg_list',c_voidp),

                ('msg_id',c_long),
                ('stat',c_int),

                ('dispatch',DISPATCH_FUNC),
                ]
    _events_ref = [] #keep reference registerd events
    PT = POINTER(T)
    ref = None
    username = None
    password = None

    events = None #events
    args = None 
    msg_list = None

    def __init__(self,username,password):
        self.username = username
        self.password = password
        u = c_char_p(username)
        p = c_char_p(password)
        self.ref = lib.lwqq_client_new(u,p)

        self.events = Events(self.ref[0].events)
        self.args = Arguments(self.ref[0].args)
        self.msg_list = RecvMsgList(self.ref[0].msg_list)

    def __del_(self):
        lib.lwqq_client_free(self.ref)

    def setDispatcher(self,dispatcher):
        self.ref[0].dispatch = DISPATCH_FUNC(dispatcher)

    def dispatch(self,cmd,delay = 50):
        self.ref[0].dispatch(cmd,delay)

    def addListener(self,events,called):
        if not isinstance(called,Command):
            called = Command.make('void',called)
        self._events_ref.append(called)
        lib.lwqq_add_event_listener(byref(events),called)

    def http(self):
        return lib.lwqq_get_http_handle(self.ref)[0]

    def sync(self,yes):
        self.http().synced = yes

    def login(self,status): lib.lwqq_login(self.ref,status,0)
    def logout(self): lib.lwqq_logout(self.ref,0)
    def relink(self): return Event(lib.lwqq_relink(self.ref))
    def get_friends_info(self,hashfunc,data):
        return Event(lib.lwqq_info_get_friends_info(self.ref,HASHFUNC(hashfunc),data))

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

    lib.lwqq_get_http_handle.argtypes = [c_voidp]
    lib.lwqq_get_http_handle.restype = POINTER(HttpHandle)

    lib.lwqq_info_get_friends_info.argtypes = [Lwqq.PT,HASHFUNC,c_voidp]
    lib.lwqq_info_get_friends_info.restype = Event.PT


register_library(lib)
