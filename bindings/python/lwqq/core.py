from .common import get_library
from .common import c_object_p

from . import enumerations

from ctypes import c_long,c_char_p,c_int

__all__ = [
        #'Status',
        'Lwqq',
        'Buddy'
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

def register_library(library):
    lib.lwqq_client_new.argtypes = [c_char_p,c_char_p]
    lib.lwqq_client_new.restype = c_object_p

    lib.lwqq_client_free.argtypes = [c_object_p]
    lib.lwqq_client_free.restype = None

    lib.lwqq_buddy_new.argtypes = []
    lib.lwqq_buddy_new.restype = c_object_p

    lib.lwqq_buddy_free.argtypes = [c_object_p]
    lib.lwqq_buddy_free.restype = None

    lib.lwqq_login.argtypes = [c_object_p,c_int,c_int]
    lib.lwqq_login.restype = None

    lib.lwqq_log_set_level.argtypes = [c_int]
    lib.lwqq_log_set_level.restype = None

    lib.lwqq_time.argtypes = []
    lib.lwqq_time.restype = c_long

#def register_enumerations():
#    for name, value in enumerations.Status:
#        Status.register(name, value)

register_library(lib)
#register_enumerations()
