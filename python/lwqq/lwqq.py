from ctypes import CFUNCTYPE,POINTER,Structure,c_char_p,pointer,c_long,c_voidp,c_ulong,c_int,cast,byref,c_void_p
import ctypes
from .common import lib
from .vplist import Command
from .core import *
from .http import HttpHandle
from .types import *
from .queue import *

from .lwjs import *

__all__ = [
        'Lwqq',
        'Buddy',
        'SimpleBuddy'
        ]

HASHFUNC = CFUNCTYPE(c_voidp,c_char_p,c_char_p,c_voidp)
DISPATCH_FUNC = CFUNCTYPE(None,Command,c_ulong)
FIND_BUDDY_FUNC = CFUNCTYPE(c_void_p,c_void_p,c_char_p)

class Buddy():
    class T(Structure):
        _fields_ = [
                ('uin',c_char_p),
                ('qqnumber',c_char_p),
                ('face',c_char_p),
                ('occupation',c_char_p),
                ('phone',c_char_p),
                ('allow',c_char_p),
                ('college',c_char_p),
                ('reg_time',c_char_p),
                ('constel',Constel),
                ('blood',BloodType),
                ('homepage',c_char_p),
                ('country',c_char_p),
                ('city',c_char_p),
                ('personal',c_char_p),
                ('nick',c_char_p),
                ('long_nick',c_char_p),
                ('shengxiao',ShengXiao),
                ('email',c_char_p),
                ('province',c_char_p),
                ('gender',Gender),
                ('mobile',c_char_p),
                ('vip_info',c_char_p),
                ('markname',c_char_p),
                ('stat',Status),
                ('client_type',ClientType),
                ('birthday',c_ulong),
                ('flag',c_char_p),
                ('cate_index',c_int),
                ('avatar',c_void_p),
                ('avatar_len',ctypes.c_size_t),
                ('last_modify',c_ulong),
                ('token',c_char_p),
                ('data',c_char_p),
                ('level',c_int),
                ('entries',LIST_ENTRY)
                ]
    PT = POINTER(T)
    ref = None
    def __init__(self,ref): self.ref = cast(ref.ref,self.PT) if hasattr(ref,'ref') else cast(ref,self.PT)
    def destroy(self): lib.lwqq_buddy_free(self.ref)
    @property
    def uin(self): return self.ref[0].uin
    @property
    def qqnumber(self): return self.ref[0].qqnumber
    @property
    def face(self): return self.ref[0].face
    @property
    def occupation(self): return self.ref[0].occupation
    @property
    def phone(self): return self.ref[0].phone
    @property
    def allow(self): return self.ref[0].allow
    @property
    def college(self): return self.ref[0].college
    @property
    def reg_time(self): return self.ref[0].reg_time
    @property
    def constel(self): return self.ref[0].constel.value
    @property
    def blood(self): return self.ref[0].blood.value
    @property
    def homepage(self): return self.ref[0].homepage
    @property
    def country(self): return self.ref[0].country
    @property
    def city(self): return self.ref[0].city
    @property
    def personal(self): return self.ref[0].personal
    @property
    def nick(self): return self.ref[0].nick
    @property
    def long_nick(self): return self.ref[0].long_nick
    @property
    def shengxiao(self): return self.ref[0].shengxiao.value
    @property
    def email(self): return self.ref[0].email
    @property
    def province(self): return self.ref[0].province
    @property
    def gender(self): return self.ref[0].gender.value
    @property
    def mobile(self): return self.ref[0].mobile
    @property
    def vip_info(self): return self.ref[0].vip_info
    @property
    def markname(self): return self.ref[0].markname
    @property
    def stat(self): return self.ref[0].stat.value
    @property
    def client_type(self): return self.ref[0].client_type.value
    @property
    def birthday(self): return self.ref[0].birthday
    @property
    def flag(self): return self.ref[0].flag
    @property
    def cate_index(self): return self.ref[0].cate_index
    @property
    def avatar(self): return self.ref[0].avatar
    @property
    def avatar_len(self): return self.ref[0].avatar_len
    @property
    def last_modify(self): return self.ref[0].last_modify
    @property
    def token(self): return self.ref[0].token
    @property
    def data(self): return self.ref[0].data
    @property
    def level(self): return self.ref[0].level
    @property
    def entries(self): return self.ref[0].entries


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
                ('find_buddy_by_uin',FIND_BUDDY_FUNC),
                ('find_buddy_by_qqnumber',FIND_BUDDY_FUNC),

                ('friends',LIST_HEAD.T),
                ('categories',LIST_HEAD.T),
                ('groups',LIST_HEAD.T),
                ('discus',LIST_HEAD.T),
                ]
    _events_ref = [] #keep reference registerd events
    PT = POINTER(T)
    ref = None
    username = None
    password = None

    events = None #events
    args = None 
    msg_list = None

    friend_list = None

    def __init__(self,username,password):
        self.username = username
        self.password = password
        u = c_char_p(username)
        p = c_char_p(password)
        self.ref = lib.lwqq_client_new(u,p)

        self.events = Events(self.ref[0].events)
        self.args = Arguments(self.ref[0].args)
        self.msg_list = RecvMsgList(self.ref[0].msg_list)

        self.friend_list = LIST_HEAD(self.ref[0].friends,Buddy.T.entries)

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

    def friends(self):
        for item in self.friend_list.foreach():
            yield Buddy(item)

    def http(self):
        return lib.lwqq_get_http_handle(self.ref)[0]

    def sync(self,yes):
        self.http().synced = yes
    ###### http actions ######
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


class SimpleBuddy():
    class T(Structure):
        _fields_ = [
                ('uin',c_char_p),
                ('qq',c_char_p),
                ('nick',c_char_p),
                ('card',c_char_p),
                ('client_type',ClientType),
                ('stat',Status),
                ('mflag',MemberFlag),
                ('cate_index',c_char_p),
                ('group_sig',c_char_p),
                ('entries',LIST_ENTRY)
                ]
    PT = POINTER(T)
    ref = None
    def __init__(self,ref): self.ref = cast(ref.ref,self.PT) if hasattr(ref,'ref') else cast(ref,self.PT)
    def destroy(self): lib.lwqq_simple_buddy_free(self.ptr_)
    @property
    def uin(self): return self.ref[0].uin
    @property
    def qq(self): return self.ref[0].qq
    @property
    def nick(self): return self.ref[0].nick
    @property
    def card(self): return self.ref[0].card
    @property
    def client_type(self): return self.ref[0].client_type.value
    @property
    def stat(self): return self.ref[0].stat.value
    @property
    def mflag(self): return self.ref[0].mflag.value
    @property
    def cate_index(self): return self.ref[0].cate_index
    @property
    def group_sig(self): return self.ref[0].group_sig
    @property
    def entries(self): return self.ref[0].entries

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
