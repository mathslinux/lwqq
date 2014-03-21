from .common import lib
from .queue import *
from .types import MsgType,ContentType,FontStyle,ServiceType
from ctypes import POINTER,cast
import ctypes

__all__ = [ 'Msg', 'MsgSeq', 'MsgContent', 'Face', 'Text', 'Img', 'CFace',
    'Message', 'BuddyMessage'
           ]
           #, 'GroupMessage', 'GroupWebMessage',
    # 'SessMessage', 'DiscuMessage'

c_time_t = ctypes.c_long

class Msg():
    class T(ctypes.Structure):
        _fields_ = [('typeid',MsgType)]
    PT = ctypes.POINTER(T)
    ref = None
    def __init__(self,ref):
        self.ref = cast(ref,self.PT)
    def destroy(self):
        lib.lwqq_msg_free(self.ref)
    @property
    def typeid(self): return self.ref[0].typeid.value

    def trycast(self,dest_type):
        return self.typeid == dest_type.TypeID


class MsgSeq(Msg):
    class T(ctypes.Structure):
        _fields_ = Msg.T._fields_ + [
            ('sender',ctypes.c_char_p),
            ('to',ctypes.c_char_p),
            ('msg_id',ctypes.c_int),
            ('msg_id2',ctypes.c_int)]
    PT = ctypes.POINTER(T)
    ref = None
    def __init__(self,ref):
        self.ref = cast(ref,self.PT)
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

class MsgContent():
    class T(ctypes.Structure):
        _fields_ = [
            ('typeid',ContentType),
            ('entries',TAILQ_ENTRY)
            ]
    PT = ctypes.POINTER(T)
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)
    @property 
    def typeid(self): return self.ref[0].typeid.value
    def trycast(self,dest_type):
        return self.typeid == dest_type.TypeID

class Face(MsgContent):
    class T(ctypes.Structure):
        _fields_ = MsgContent.T._fields_ + [
            ('face',ctypes.c_int),
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.FACE
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)
    @property
    def face(self): return self.ref[0].face
    

class Text(MsgContent):
    class T(ctypes.Structure):
        _fields_ = MsgContent.T._fields_ + [
            ('text',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.TEXT
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)
    @property
    def text(self): return self.ref[0].text

class Img(MsgContent):
    class T(ctypes.Structure):
        _fields_ = MsgContent.T._fields_+ [
            ('name',ctypes.c_char_p),
            ('data',ctypes.c_voidp),
            ('size',ctypes.c_size_t),
            ('success',ctypes.c_int),
            ('file_path',ctypes.c_char_p),
            ('url',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = ContentType.OFFPIC
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)
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
    ref = None
    def __init__(self,ref): self.ref = cast(ref,self.PT)
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
    class T(ctypes.Structure):
        _fields_ = MsgSeq.T._fields_ + [
            ('time',c_time_t),
            ('upload_retry',ctypes.c_int),

            ('f_name',ctypes.c_char_p),
            ('f_size',ctypes.c_int),
            ('f_style',FontStyle),
            ('f_color',ctypes.c_char * 7),
            ('content',TAILQ_HEAD.T),
            ]
    PT = ctypes.POINTER(T)
    ref = None
    content = None
    def __init__(self,ref):
        self.ref = cast(ref,self.PT)
        self.content = TAILQ_HEAD(self.ref[0].content,MsgContent.T.entries)
    def contents(self):
        for item in self.content.foreach():
            yield MsgContent(item)

class BuddyMessage(Message):
    class T(ctypes.Structure):
        _fields_ = Message.T._fields_ + [
            ('from',ctypes.c_void_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_BUDDY_MSG
    ref = None
    def __init__(self,ref):
        super().__init__(ref)
        self.ref = cast(ref,self.PT)

class GroupMessage(Message):
    class T(ctypes.Structure):
        _fields_ = Message.T._fields_ + [
            ('send',ctypes.c_char_p),
            ('group_code',ctypes.c_char_p)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_GROUP_MSG
    ref = None
    def __init__(self,ref):
        super().__init__(ref)
        self.ref = cast(ref,self.PT)

class SessMessage(Message):
    class T(ctypes.Structure):
        _fields_ = Message.T._fields_ + [
            ('id',ctypes.c_char_p),
            ('group_sig',ctypes.c_char_p),
            ('service_type',ServiceType)
            ]
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_SESS_MSG
    ref = None
    def __init__(self,ref):
        super().__init__(ref)
        self.ref = cast(ref,self.PT)
    @property
    def id(self): return self.ref[0].id
    @property
    def group_sig(self): return self.ref[0].group_sig
    @property
    def service_type(self): return self.ref[0].service_type.value

class DiscuMessage(GroupMessage):
    class T(ctypes.Structure):
        _fields_ = GroupMessage.T._fields_
    PT = ctypes.POINTER(T)
    TypeID = MsgType.MS_DISCU_MSG
    ref = None
    def __init__(self,ref):
        super().__init__(ref)
        self.ref = cast(ref,self.PT)

def register_library(lib):
    lib.lwqq_msg_free.argtypes = [Msg.PT]

register_library(lib)
