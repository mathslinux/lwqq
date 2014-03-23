from .base import lib,LwqqBase
from .queue import *
from .types import *
from ctypes import POINTER,cast
from .core import Event

import ctypes

__all__ = [ 'Msg', 'MsgSeq', 'MsgContent', 'Face', 'Text', 'Img', 'CFace',
    'Message', 'BuddyMessage', 'GroupMessage', 'SessMessage', 'DiscuMessage',
    'GroupWebMessage', 'StatusChange', 'KickMessage', 'SystemMessage',
    'MsgAddBuddy', 'MsgVerifyRequired', 'MsgVerifyPass', 'MsgVerifyPassAdd',
    'GroupSystemMessage', 'BlistChange', 'InputNotify', 'ShakeMessage', 'RecvMsgList' ]

c_time_t = ctypes.c_long

class Msg(LwqqBase):
    class T(ctypes.Structure):
        _fields_ = [('typeid',MsgType)]
    PT = ctypes.POINTER(T)
    def destroy(self):
        lib.lwqq_msg_free(self.ref)
    @property
    def typeid(self): return self.ref[0].typeid.value

    def trycast(self,dest_type):
        if dest_type == Message:
            return MsgType.mt(self.typeid) == MsgType.mt(dest_type.TypeID)
        return self.typeid == dest_type.TypeID
    @classmethod
    def new(cls,**kw):
        ins = super().new(malloc=True,**kw)
        ins.ref[0].typeid = cls.TypeID
        return ins

class MsgSeq(Msg):
    class T(Msg.T):
        _fields_ = [
            ('sender',ctypes.c_char_p),
            ('to',ctypes.c_char_p),
            ('msg_id',ctypes.c_int),
            ('msg_id2',ctypes.c_int)]
    PT = ctypes.POINTER(T)
    @property
    def super(self): return self.ref[0].parent
    @property
    def sender(self): return self.ref[0].sender
    @property
    def to(self): return self.ref[0].to
    @property
    def msg_id(self): return self.ref[0].msg_id
    @property
    def msg_id2(self): return self.ref[0].msg_id2

class MsgContent(LwqqBase):
    class T(ctypes.Structure):
        _fields_ = [
            ('typeid',ContentType),
            ('entries',TAILQ_ENTRY)
            ]
    PT = ctypes.POINTER(T)
    @property 
    def typeid(self): return self.ref[0].typeid.value
    def trycast(self,dest_type):
        return self.typeid == dest_type.TypeID
    @classmethod
    def new(cls,**kw):
        instance = super().new(malloc=True,**kw)
        instance.ref[0].typeid = cls.TypeID
        return instance

class Face(MsgContent):
    class T(MsgContent.T):
        _fields_ = [
            ('face',ctypes.c_int),
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.FACE
    @property
    def face(self): return self.ref[0].face
    

class Text(MsgContent):
    class T(MsgContent.T):
        _fields_ =  [
            ('text',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.TEXT
    @property
    def text(self): return self.ref[0].text

class Img(MsgContent):
    class T(MsgContent.T):
        _fields_ =  [
            ('name',ctypes.c_char_p),
            ('data',ctypes.c_voidp),
            ('size',ctypes.c_size_t),
            ('success',ctypes.c_int),
            ('file_path',ctypes.c_char_p),
            ('url',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.OFFPIC
    @property
    def name(self): return self.ref[0].name
    @property
    def data(self): return self.ref[0].data
    @property
    def size(self): return self.ref[0].size
    @property
    def success(self): return self.ref[0].success
    @property
    def file_path(self): return self.ref[0].file_path
    @property
    def url(self): return self.ref[0].url

class CFace(MsgContent):
    class T(ctypes.Structure):
        _fields_ = MsgContent.T._fields_ + [
            ('name',ctypes.c_char_p),
            ('data',ctypes.c_voidp),
            ('size',ctypes.c_size_t),
            ('file_id',ctypes.c_char_p),
            ('key',ctypes.c_char_p),
            ('serv_ip',ctypes.c_char * 24),
            ('serv_port',ctypes.c_char * 8),
            ('url',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.CFACE
    @property
    def name(self): return self.ref[0].name
    @property
    def data(self): return self.ref[0].data
    @property
    def size(self): return self.ref[0].size
    @property
    def file_id(self): return self.ref[0].file_id
    @property
    def key(self): return self.ref[0].key
    @property
    def serv_ip(self): return self.ref[0].serv_ip
    @property
    def serv_port(self): return self.ref[0].serv_port
    @property
    def url(self): return self.ref[0].url

class Message(MsgSeq):
    class T(MsgSeq.T):
        _fields_ = [
            ('time',c_time_t),
            ('upload_retry',ctypes.c_int),

            ('f_name',ctypes.c_char_p),
            ('f_size',ctypes.c_int),
            ('f_style',FontStyle),
            ('f_color',ctypes.c_char * 7),
            ('content',TAILQ_HEAD.T),
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MT_MESSAGE
    content = None
    def __init__(self,ref): 
        super().__init__(ref)
        self.content = TAILQ_HEAD(self.ref[0].content,MsgContent.T.entries)
    def contents(self):
        for item in self.content.foreach():
            yield MsgContent(item)
    def append(self,content):
        self.content.insert_tail(content.ref)
        return self
    def __str__(self):
        ret = ""
        for item in self.contents():
            if item.trycast(Text):
                t = Text(item.ref).text
                if not t: continue
                ret+=Text(item.ref).text.decode("utf-8")
        return ret
    def send(self,client):
        client = client.ref if hasattr(client,'ref') else client
        return Event(lib.lwqq_msg_send(client,self.ref))
    @classmethod
    def new(cls,**kw):
        opt = {'upload_retry':3, 'f_color':b'000000', 'f_name':b'Arial',
                'f_size':11 }
        ins = super().new(**dict(opt,**kw))
        ins.content.init()
        return ins


class BuddyMessage(Message):
    class T(Message.T):
        _fields_ = [
            ('from',ctypes.c_void_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_BUDDY_MSG
    def send(self,buddy):
        self.ref[0].to = buddy.uin
        return super().send(buddy.lc)

class GroupMessage(Message):
    class T(Message.T):
        _fields_ = [
            ('send',ctypes.c_char_p),
            ('group_code',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_GROUP_MSG

class SessMessage(Message):
    class T(Message.T):
        _fields_ = [
            ('id',ctypes.c_char_p),
            ('group_sig',ctypes.c_char_p),
            ('service_type',GroupType)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_SESS_MSG
    @property
    def id(self): return self.ref[0].id
    @property
    def group_sig(self): return self.ref[0].group_sig
    @property
    def service_type(self): return self.ref[0].service_type.value

class DiscuMessage(GroupMessage):
    TypeID = MsgType.MS_DISCU_MSG

class GroupWebMessage(GroupMessage):
    TypeID = MsgType.MS_GROUP_WEB_MSG

class StatusChange(Msg):
    class T(Msg.T):
        _fields_ = [
                ('who',ctypes.c_char_p),
                ('status',ctypes.c_char_p),
                ('client_type',ctypes.c_int)
                ]
    PT = POINTER(T)
    TypeID = MsgType.MT_STATUS_CHANGE
    @property
    def who(self): return self.ref[0].who
    @property
    def status(self): return self.ref[0].status
    @property
    def client_type(self): return self.ref[0].client_type

class KickMessage(Msg):
    class T(Msg.T):
        _fields_ = [
                ('show_reason',ctypes.c_int),
                ('reason',ctypes.c_char_p),
                ('way',ctypes.c_char_p)
                ]
    PT = POINTER(T)
    TypeID = MsgType.MT_KICK_MESSAGE
    @property
    def show_reason(self): return self.ref[0].show_reason
    @property
    def reason(self): return self.ref[0].reason
    @property
    def way(self): return self.ref[0].way

class SystemMessage(MsgSeq):
    class T(MsgSeq.T):
        _fields_ = [
            ('seq',ctypes.c_char_p),
            ('typeid',MsgSystemType),
            ('account',ctypes.c_char_p),
            ('stat',ctypes.c_char_p),
            ('client_type',ctypes.c_char_p),
            ]
    PT = POINTER(T)
    TypeID = MsgType.MT_SYSTEM
    @property
    def seq(self): return self.ref[0].seq
    @property
    def typeid(self): return self.ref[0].typeid.value
    def trycast(self,dest_type): return self.typeid == dest_type.SubTypeID
    @property
    def account(self): return self.ref[0].account
    @property
    def stat(self): return self.ref[0].stat
    @property
    def client_type(self): return self.ref[0].client_type

class MsgAddBuddy(SystemMessage):
    class T(SystemMessage.T):
        _fields_ = [('sig',ctypes.c_char_p)]
    PT = POINTER(T)
    SubTypeID = MsgSystemType.ADDED_BUDDY_SIG
    @property
    def sig(self): return self.ref[0].sig

class MsgVerifyRequired(SystemMessage):
    class T(SystemMessage.T):
        _fields_ = [
                ('msg',ctypes.c_char_p),
                ('allow',ctypes.c_char_p)]
    PT = POINTER(T)
    SubTypeID = MsgSystemType.VERIFY_REQUIRED
    @property
    def msg(self): return self.ref[0].msg
    @property
    def allow(self): return self.ref[0].allow

class MsgVerifyPass(SystemMessage):
    class T(SystemMessage.T):
        _fields_ = [('group_id',ctypes.c_char_p)]
    PT = POINTER(T)
    SubTypeID = MsgSystemType.VERIFY_PASS
    @property
    def group_id(self): return self.ref[0].group_id

class MsgVerifyPassAdd(MsgVerifyPass):
    SubTypeID = MsgSystemType.VERIFY_PASS_ADD

class GroupSystemMessage(MsgSeq):
    class T(MsgSeq.T):
        _fields_ = [
                ('typeid',MsgSysGType),
                ('group_uin',ctypes.c_char_p),
                ('gcode',ctypes.c_char_p),
                ('account',ctypes.c_char_p),
                ('member_uin',ctypes.c_char_p),
                ('member',ctypes.c_char_p),
                ('admin_uin',ctypes.c_char_p),
                ('admin',ctypes.c_char_p),
                ('msg',ctypes.c_char_p),
                ('is_my_self',ctypes.c_char_p),
                ('group',ctypes.c_void_p)
                ]
    PT = POINTER(T)
    TypeID = MsgType.MT_SYS_G_MSG
    @property
    def typeid(self): return self.ref[0].typeid.value
    @property
    def group_uin(self):return self.ref[0].group_uin
    @property
    def gcode(self):return self.ref[0].gcode
    @property
    def account(self):return self.ref[0].account
    @property
    def member_uin(self):return self.ref[0].member_uin
    @property
    def member(self):return self.ref[0].member
    @property
    def admin_uin(self):return self.ref[0].admin_uin
    @property
    def admin(self):return self.ref[0].admin
    @property
    def msg(self):return self.ref[0].msg
    @property
    def is_my_self(self):return self.ref[0].is_my_self
    @property
    def group(self):return self.ref[0].group

    def get_stranger_info(self,client,out):
        if self.typeid == MsgSysGType.GROUP_REQUEST_JOIN:
            return Event(lwqq_info_get_stranger_info_by_msg(client.ref,self.ref,out.ref))
    def anser_request(self,client,answer,reason):
        if self.typeid == MsgSysGType.GROUP_REQUEST_JOIN:
            return Event(lwqq_info_answer_request_join_group(client.ref,self.ref,answer,reason))

class BlistChange(Msg):
    class T(Msg.T):
        _fields_ = [
                ('added_friends',LIST_HEAD.T),
                ('removed_friends',LIST_HEAD.T)
                ]
    PT = POINTER(T)
    TypeID = MsgType.MT_BLIST_CHANGE
    added_friend = None
    removed_friend = None

class InputNotify(Msg):
    class T(Msg.T):
        _fields_ = [
                ('sender',ctypes.c_char_p),
                ('to',ctypes.c_char_p),
                ('msg_type',ctypes.c_int)
                ]
    PT = POINTER(T)
    TypeID = MsgType.MT_INPUT_NOTIFY
    @property
    def sender(self): return self.ref[0].sender
    @property
    def to(self): return self.ref[0].to
    @property
    def msg_type(self): return self.ref[0].msg_type

class ShakeMessage(MsgSeq):
    class T(MsgSeq.T):
        _fields_ = [('reply_ip',ctypes.c_ulong)]
    PT = POINTER(T)
    TypeID = MsgType.MT_SHAKE_MESSAGE
    @property
    def reply_ip(self): return self.ref[0].reply_ip

class RecvMsgList():
    ref = None
    def __init__(self,ref):
        self.ref = ref
    def poll(self,flags):
        lib.lwqq_msglist_poll(self.ref,flags)
    def close(self):
        lib.lwqq_msglist_close(self.ref)
    def read(self):
        while True:
            msg = lib.lwqq_msglist_read(self.ref)
            if not msg: break
            yield Msg(msg)

def register_library(lib):
    lib.lwqq_msg_free.argtypes = [Msg.PT]

    lib.lwqq_msglist_poll.argtypes = [ctypes.c_voidp,ctypes.c_long]
    lib.lwqq_msglist_close.argtypes = [ctypes.c_voidp]
    lib.lwqq_msglist_read.argtypes = [ctypes.c_voidp]
    lib.lwqq_msglist_read.restype = Msg.PT

    

register_library(lib)
