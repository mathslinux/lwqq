from .common import get_library
from .common import c_object_p

from . import enumerations

from ctypes import c_long,c_char_p,c_int,c_voidp,c_size_t
from ctypes import Structure,CFUNCTYPE

__all__ = [
        #'Status',
        'Lwqq',
        'Buddy',
        'SimpleBuddy'
        ]

lib = get_library()

#class Status(object):
#    """Represents an individual OpCode enumeration."""
#
#    _value_map = {}
#
#    def __init__(self, name, value):
#        self.name = name
#        self.value = value
#
#    def __repr__(self):
#        return 'Status.%s' % self.name
#
#    @staticmethod
#    def from_value(value):
#        """Obtain an OpCode instance from a numeric value."""
#        result = Status._value_map.get(value, None)
#
#        if result is None:
#            raise ValueError('Unknown OpCode: %d' % value)
#
#        return result
#
#    @staticmethod
#    def register(name, value):
#        """Registers a new OpCode enumeration.
#
#        This is called by this module for each enumeration defined in
#        enumerations. You should not need to call this outside this module.
#        """
#        if value in Status._value_map:
#            raise ValueError('OpCode value already registered: %d' % value)
#
#        status = Status(name, value)
#        Status._value_map[value] = status
#        setattr(Status, name, status)

class vp_list(Structure):
    _fields_ = [
            ('st',c_voidp),
            ('cur',c_voidp),
            ('sz',c_size_t)
            ]

class Command(Structure):
    _fields_ = [
            ('dsph',c_object_p),
            ('func',c_object_p),
            ('data',vp_list),
            ('next',c_object_p)
            ]

    @classmethod
    def make(self,closure):
        CLOSURE = CFUNCTYPE(None)
        return lib.vp_make_command(lib.vp_func_void,CLOSURE(closure))


class Event(object):
    event_ = None

    def __init__(self,async_event):
        event_ = async_event

    @classmethod
    def new(self,http_req):
        return Event(lib.lwqq_async_event_new(http_req))

    def addListener(self,closure):
        lib.lwqq_async_add_event_listener(self.event_,Command.make(closure))
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

class Lwqq(object):
    lc_ = None

    def __init__(self,username,password):
        u = c_char_p(username)
        p = c_char_p(password)
        self.lc_ = lib.lwqq_client_new(u,p)

    def __del_(self):
        lib.lwqq_client_free(self.lc_)

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

def register_library(library):
    lib.lwqq_client_new.argtypes = [c_char_p,c_char_p]
    lib.lwqq_client_new.restype = c_object_p

    lib.lwqq_client_free.argtypes = [c_object_p]
    lib.lwqq_client_free.restype = None

    lib.lwqq_buddy_new.argtypes = []
    lib.lwqq_buddy_new.restype = c_object_p

    lib.lwqq_buddy_free.argtypes = [c_object_p]
    lib.lwqq_buddy_free.restype = None

    lib.lwqq_simple_buddy_new.argtypes = []
    lib.lwqq_simple_buddy_new.restype = c_object_p

    lib.lwqq_simple_buddy_free.argtypes = [c_object_p]
    lib.lwqq_simple_buddy_free.restype = None

    lib.lwqq_login.argtypes = [c_object_p,c_int,c_int]
    lib.lwqq_login.restype = None

    lib.lwqq_logout.argtypes = [c_object_p,c_int]
    lib.lwqq_logout.restype = None

    lib.lwqq_relink.argtypes = [c_object_p]
    lib.lwqq_relink.restype = c_object_p

    lib.lwqq_log_set_level.argtypes = [c_int]
    lib.lwqq_log_set_level.restype = None

    lib.vp_make_command.argtypes = [c_voidp,c_voidp]
    lib.vp_make_command.restype = Command

    lib.vp_func_void.argtypes = [c_voidp,c_voidp,c_voidp]
    lib.vp_func_void.restype = None

    lib.lwqq_async_event_new.argtypes = [c_object_p]
    lib.lwqq_async_event_new.restype = c_object_p

    lib.lwqq_async_event_finish.argtypes = [c_object_p]
    lib.lwqq_async_event_finish.restype = None

    lib.lwqq_async_add_event_listener.argtypes = [c_object_p,Command]
    lib.lwqq_async_add_event_listener.restype = None

    lib.lwqq_async_add_evset_listener.argtypes = [c_object_p,Command]
    lib.lwqq_async_add_evset_listener.restype = None

    lib.lwqq_async_evset_new.argtypes = []
    lib.lwqq_async_evset_new.restype = c_object_p

    lib.lwqq_async_evset_free.argtypes = [c_object_p]
    lib.lwqq_async_evset_free.restype = None

    lib.lwqq_time.argtypes = []
    lib.lwqq_time.restype = c_long

#def register_enumerations():
#    for name, value in enumerations.Status:
#        Status.register(name, value)

register_library(lib)
#register_enumerations()
