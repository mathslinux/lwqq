from ctypes import CFUNCTYPE,POINTER,Structure,c_char_p,pointer,c_long,c_voidp,c_ulong,c_int,cast,byref,c_void_p
import ctypes
from .base import lib, LwqqBase
from .vplist import Command
from .core import *
from .http import HttpHandle
from .types import *
from .queue import *
from .msg import GroupSystemMessage

from .lwjs import *

__all__ = [ 'Lwqq', 'Buddy', 'SimpleBuddy', 'Category', 'Group', 'Discu' ]

HASHFUNC = CFUNCTYPE(c_voidp,c_char_p,c_char_p,c_voidp)
DISPATCH_FUNC = CFUNCTYPE(None,Command,c_ulong)
FIND_BUDDY_FUNC = CFUNCTYPE(c_void_p,c_void_p,c_char_p)

class Category(LwqqBase):
    class T(Structure):
        _fields_ = [
                ('index',c_int),
                ('sort',c_int),
                ('name',c_char_p),
                ('count',c_int),
                ('entries',LIST_ENTRY)
                ]
    PT = POINTER(T)
    @property
    def index(self): return self.ref[0].index
    @property
    def sort(self): return self.ref[0].sort
    @property
    def name(self): return self.ref[0].name
    @property
    def count(self): return self.ref[0].count

class Buddy(LwqqBase):
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
    lc = None
    cate_list = None
    def __init__(self,ref,client=None): 
        super().__init__(ref)
        if client: 
            self.lc = client.ref
            self.cate_list = client.cate_list
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
    def get_category(self):
        if not self.cate_list: return None
        for item in self.cate_list.foreach():
            c = Category(item)
            if c.index == self.cate_index:
                return c

    def get_qqnumber(self):
        if not self.lc: return None
        qqnumber = ctypes.addressof(self.ref[0])+self.T.qqnumber.offset
        return Event(lib.lwqq_info_get_qqnumber(self.lc,self.ref[0].uin,qqnumber))
    def get_detail(self):
        if not self.lc: return None
        return Event(lib.lwqq_info_get_friend_detail_info(self.lc,self.ref))
    def get_avatar(self):
        if not self.lc: return None
        return Event(lib.lwqq_info_get_avatar(self.lc,self.ref,None))
    def save_avatar(self,path):
        return lib.lwqq_info_save_avatar(self.ref,None,path.encode())
    def change_markname(self,mark):
        if not self.lc: return None
        return Event(lib.lwqq_info_change_buddy_markname(self.lc,self.ref,mark))
    def change_category(self,cate):
        if not self.lc: return None
        cate_idx = cate.index
        return Event(lib.lwqq_info_modify_buddy_category(self.lc,self.ref,cate_idx))
    def delete(self,del_type):
        if not self.lc: return None
        return Event(lib.lwqq_info_delete_friend(self.lc,self.ref,del_type))

class SimpleBuddy(LwqqBase):
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
    lc = None
    def __init__(self,ref,client=None): 
        super().__init__(ref)
        if client: 
            self.lc = client.ref if hasattr(client,'ref') else client
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

    def get_qqnumber(self):
        if not self.lc: return None
        qqnumber = ctypes.addressof(self.ref[0])+self.T.qq.offset
        return Event(lib.lwqq_info_get_qqnumber(self.lc,self.ref[0].uin,qqnumber))


class Group(LwqqBase):
    class T(Structure):
        _fields_ = [
                ('typeid',GroupType),
                ('name',c_char_p),
                ('gid',c_char_p),
                ('account',c_char_p),
                ('info_seq',c_char_p),
                ('code',c_char_p),
                ('markname',c_char_p),
                ('face',c_char_p),
                ('memo',c_char_p),
                ('class_type',c_char_p),
                ('fingermemo',c_char_p),
                ('createtime',c_ulong),
                ('level',c_char_p),
                ('owner',c_char_p),
                ('flag',c_char_p),
                ('option',c_char_p),
                ('mask',MaskType),
                ('group_sig',c_char_p),
                ('last_modify',c_ulong),
                ('avatar',c_void_p),
                ('avatar_len',ctypes.c_size_t),
                ('data',c_void_p),
                ('entries',LIST_ENTRY),
                ('members',LIST_HEAD.T)
                ]
    PT = POINTER(T)
    lc = None
    member_list = None
    def __init__(self,ref,client=None): 
        super().__init__(ref)
        self.member_list = LIST_HEAD(self.ref[0].members,SimpleBuddy.T.entries)
        if client: 
            self.lc = client.ref
    def destroy(self): lib.lwqq_group_free(self.ref)
    @property
    def typeid(self): return self.ref[0].typeid.value
    @property
    def name(self): return self.ref[0].name
    @property
    def gid(self): return self.ref[0].gid
    did = gid
    @property
    def account(self): return self.ref[0].account
    qqnumber = account
    @property
    def info_seq(self): return self.ref[0].info_seq
    @property
    def code(self): return self.ref[0].code
    @property
    def markname(self): return self.ref[0].markname
    @property
    def face(self): return self.ref[0].face
    @property
    def memo(self): return self.ref[0].memo
    @property
    def class_type(self): return self.ref[0].class_type
    @property
    def fingermemo(self): return self.ref[0].fingermemo
    @property
    def createtime(self): return self.ref[0].createtime
    @property
    def level(self): return self.ref[0].level
    @property
    def owner(self): return self.ref[0].owner
    @property
    def flag(self): return self.ref[0].flag
    @property
    def option(self): return self.ref[0].option
    @property
    def mask(self): return self.ref[0].mask.value
    @property
    def group_sig(self): return self.ref[0].group_sig
    @property
    def last_modify(self): return self.ref[0].last_modify
    @property
    def avatar(self): return self.ref[0].avatar
    @property
    def avatar_len(self): return self.ref[0].avatar_len
    @property
    def data(self): return self.ref[0].data
    def members(self): 
        for item in self.member_list.foreach():
            yield SimpleBuddy(item)
    def find_member(self,uin=None,qqnumber=None):
        for m in self.members():
            if uin and m.uin == uin: return m
            if qqnumber and m.qq == qqnumber: return m

    def get_qqnumber(self):
        if not self.lc: return None
        qqnumber = ctypes.addressof(self.ref[0])+self.T.account.offset
        return Event(lib.lwqq_info_get_qqnumber(self.lc,self.ref[0].gid,qqnumber))
    def get_avatar(self):
        if not self.lc: return None
        return Event(lib.lwqq_info_get_avatar(self.lc,None,self.ref))
    def save_avatar(self,path):
        return lib.lwqq_info_save_avatar(None,self.ref,path.encode())
    def get_detail(self):
        if not self.lc: return None
        return Event(lib.lwqq_info_get_group_detail_info(self.lc,self.ref,None))
    def change_markname(self,mark):
        if not self.lc: return None
        return Event(lib.lwqq_info_change_group_markname(self.lc,self.ref,mark))
    def change_topic(self,topic):
        if not self.lc: return None
        if self.typeid == GroupType.DISCU:
            return Event(lib.lwqq_info_change_discu_topic(self.lc,self.ref,topic))
    def change_mask(self,mask):
        if not self.lc: return None
        return Event(lib.lwqq_info_mask_group(self.lc,self.ref,mask))
    def delete(self,del_type):    
        if not self.lc: return None
        return Event(lib.lwqq_info_delete_group(self.lc,self.ref,del_type))


Discu = Group

class Lwqq(LwqqBase):
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
    username = None
    password = None

    events = None #events
    args = None 
    msg_list = None

    friend_list = None
    cate_list = None
    group_list = None
    discu_list = None

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
        self.cate_list = LIST_HEAD(self.ref[0].categories,Category.T.entries)
        self.group_list = LIST_HEAD(self.ref[0].groups,Group.T.entries)
        self.discu_list = LIST_HEAD(self.ref[0].discus,Discu.T.entries)

    def __del__(self):
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
            yield Buddy(item,self)
    def categories(self):
        for item in self.cate_list.foreach():
            yield Category(item)
    def groups(self):
        for item in self.group_list.foreach():
            yield Group(item,self)
    def discus(self):
        for item in self.discu_list.foreach():
            yield Discu(item,self)

    def http(self):
        return lib.lwqq_get_http_handle(self.ref)[0]

    def sync(self,yes):
        self.http().synced = yes
    def find_buddy(self,uin=None,qqnumber=None):
        found = None
        if uin:
            found = self.ref[0].find_buddy_by_uin(self.ref,uin)
        if qqnumber:
            found = self.ref[0].find_buddy_by_qqnumber(self.ref,uin)
        if found:
            return Buddy(cast(found,Buddy.PT),self)
    def find_group(self,gid=None,qqnumber=None):
        for g in self.groups():
            if gid and g.gid == gid: return g
            if qqnumber and g.qq == qqnumber: return g
    def find_discu(self,did=None):
        for d in self.discus():
            if did and d.did == did: return d
    ###### http actions ######
    def login(self,status): lib.lwqq_login(self.ref,status,0)
    def logout(self): lib.lwqq_logout(self.ref,0)
    def relink(self): return Event(lib.lwqq_relink(self.ref))

    def get_onlines(self):
        return Event(lib.lwqq_info_get_online_buddies(self.ref,None))
    def get_friends_info(self,hashfunc,data):
        return Event(lib.lwqq_info_get_friends_info(self.ref,HASHFUNC(hashfunc),data))
    def get_all_friend_qqnumbers(self):
        return Event(lib.lwqq_info_get_all_friend_qqnumber(self.ref,None))
    def get_group_list(self):
        return Event(lib.lwqq_info_get_group_name_list(self.ref,None))
    def get_discu_list(self):
        return Event(lib.lwqq_info_get_discu_name_list(self.ref))
    def get_stranger_info(self,serv_id,out):
        return Evnet(lib.lwqq_info_get_stranger_info(self.ref,serv_id,out.ref))
    get_group_member_detail = get_stranger_info
    def change_status(self,stat):
        return Event(lib.lwqq_info_change_status(self.ref,stat))

    def search_friend(self,qq_or_email,out):
        return Event(lib.lwqq_info_search_friend(self.lc,qq_or_email,out.ref))
    def add_friend(self,buddy,message):
        return Event(lib.lwqq_info_add_friend(self.lc,buddy.ref,message))
    def answer_request_friend(self,qq,allow,extra):
        return Event(lib.lwqq_info_answer_request_friend(self.ref,qq,allow,extra))
    def search_group(self,qq,out):
        return Event(lib.lwqq_info_search_group_by_qq(self.lc,qq,out.ref))
    def add_group(self,group,message):
        return Event(lib.lwqq_info_add_group(self.lc,group.ref,message))
    def add_group_member_as_friend(self,buddy,markname):
        return Event(lib.lwqq_info_add_group_member_as_friend(self.ref,buddy.ref,markmake))

    @classmethod
    def time(cls):
        return lib.lwqq_time()

    @classmethod
    def log_level(cls,level):
        lib.lwqq_log_set_level(level)


def register_library(lib):
    lib.lwqq_client_new.argtypes = [c_char_p,c_char_p]
    lib.lwqq_client_new.restype = Lwqq.PT

    lib.lwqq_client_free.argtypes = [Lwqq.PT]

    lib.lwqq_buddy_new.argtypes = []
    lib.lwqq_buddy_new.restype = c_voidp

    lib.lwqq_buddy_free.argtypes = [Buddy.PT]
    lib.lwqq_group_free.argtypes = [Group.PT]

    lib.lwqq_simple_buddy_new.argtypes = []
    lib.lwqq_simple_buddy_new.restype = c_voidp

    lib.lwqq_simple_buddy_free.argtypes = [c_voidp]

    lib.lwqq_login.argtypes = [Lwqq.PT,Status,c_int]

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
    lib.lwqq_info_get_qqnumber.argtypes = [Lwqq.PT,c_char_p,c_void_p]
    lib.lwqq_info_get_qqnumber.restype = Event.PT
    lib.lwqq_info_get_all_friend_qqnumbers.argtypes = [Lwqq.PT,c_void_p]
    lib.lwqq_info_get_all_friend_qqnumbers.restype = None
    lib.lwqq_info_get_friend_detail_info.argtypes = [Lwqq.PT,Buddy.PT]
    lib.lwqq_info_get_friend_detail_info.restype = Event.PT
    lib.lwqq_info_get_group_name_list.argtypes = [Lwqq.PT,c_void_p]
    lib.lwqq_info_get_group_name_list.restype = Event.PT
    lib.lwqq_info_get_discu_name_list.argtypes = [Lwqq.PT]
    lib.lwqq_info_get_discu_name_list.restype = Event.PT
    lib.lwqq_info_get_avatar.argtypes = [Lwqq.PT,Buddy.PT,Group.PT]
    lib.lwqq_info_get_avatar.restype = Event.PT
    lib.lwqq_info_save_avatar.argtypes = [Buddy.PT,Group.PT,c_char_p]
    lib.lwqq_info_save_avatar.restype = ErrorCode
    lib.lwqq_info_get_group_detail_info.argtypes = [Lwqq.PT,Group.PT,c_void_p]
    lib.lwqq_info_get_group_detail_info.restype = Event.PT
    lib.lwqq_info_get_online_buddies.argtypes = [Lwqq.PT,c_void_p]
    lib.lwqq_info_get_online_buddies.restype = Event.PT
    lib.lwqq_info_change_buddy_markname.argtypes = [Lwqq.PT,Buddy.PT,c_char_p]
    lib.lwqq_info_change_buddy_markname.restype = Event.PT
    lib.lwqq_info_change_group_markname.argtypes = [Lwqq.PT,Group.PT,c_char_p]
    lib.lwqq_info_change_group_markname.restype = Event.PT
    lib.lwqq_info_change_discu_topic.argtypes = [Lwqq.PT,Group.PT,c_char_p]
    lib.lwqq_info_change_discu_topic.restype = Event.PT
    lib.lwqq_info_modify_buddy_category.argtypes = [Lwqq.PT,Buddy.PT,c_int]
    lib.lwqq_info_modify_buddy_category.restype = Event.PT
    lib.lwqq_info_delete_friend.argtypes = [Lwqq.PT,Buddy.PT,DelType]
    lib.lwqq_info_delete_friend.restype = Event.PT
    lib.lwqq_info_delete_group.argtypes = [Lwqq.PT,Group.PT,DelType]
    lib.lwqq_info_delete_group.restype = Event.PT
    lib.lwqq_info_change_status.argtypes = [Lwqq.PT,Status]
    lib.lwqq_info_change_status.restype = Event.PT
    lib.lwqq_info_mask_group.argtypes = [Lwqq.PT,Group.PT,MaskType]
    lib.lwqq_info_mask_group.restype = Event.PT
    lib.lwqq_info_search_friend.argtypes = [Lwqq.PT,c_char_p,Buddy.PT]
    lib.lwqq_info_search_friend.restype = Event.PT
    lib.lwqq_info_add_friend.argtypes = [Lwqq.PT,Buddy.PT,c_char_p]
    lib.lwqq_info_add_friend.restype = Event.PT
    lib.lwqq_info_search_group_by_qq.argtypes = [Lwqq.PT,c_char_p,Group.PT]
    lib.lwqq_info_search_group_by_qq.restype = Event.PT
    lib.lwqq_info_add_group.argtypes = [Lwqq.PT,Group.PT,c_char_p]
    lib.lwqq_info_add_group.restype = Event.PT
    lib.lwqq_info_get_stranger_info.argtypes = [Lwqq.PT,c_char_p,Buddy.PT]
    lib.lwqq_info_get_stranger_info.restype = Event.PT
    lib.lwqq_info_add_group_member_as_friend.argtypes = [Lwqq.PT,Buddy.PT,c_char_p]
    lib.lwqq_info_add_group_member_as_friend.restype = Event.PT
    
    #orginal in msg.py but has conflicts
    lib.lwqq_info_get_stranger_info_by_msg.argtypes = [Lwqq.PT,GroupSystemMessage.PT,Buddy.PT]
    lib.lwqq_info_get_stranger_info_by_msg.restype = Event.PT
    lib.lwqq_info_answer_request_join_group.argtypes = [Lwqq.PT,GroupSystemMessage.PT,Answer,ctypes.c_char_p]
    lib.lwqq_info_answer_request_join_group.restype = Event.PT

register_library(lib)
