from .common import lib
from .queue import *
from .types import MsgType,ContentType,FontStyle,ServiceType
import ctypes

__all__ = [ 'Msg', 'MsgSeq', 'MsgContent', 'Face', 'Text', 'Img', 'CFace',
    'Message', 'BuddyMessage', 'GroupMessage', 'GroupWebMessage',
    'SessMessage', 'DiscuMessage' ]

c_time_t = ctypes.c_long


class Msg(ctypes.Structure):
    _fields_ = [('_typeid',MsgType)]

    def destroy(self):
        lib.lwqq_msg_free(ctypes.byref(self))
    @property
    def typeid(self): return self._typeid.value

    def trycast(self,dest_type):
        return self.typeid == dest_type.TypeID


class MsgSeq(Msg):
    _fields_ = [
            ('_super',Msg),
            ('_from',ctypes.c_char_p),
            ('_to',ctypes.c_char_p),
            ('_msg_id',ctypes.c_int),
            ('_msg_id2',ctypes.c_int)]
    @property
    def super(self): return self._super
    @property
    def sender(self): return self._from
    @property
    def to(self): return self._to
    @property
    def msg_id(self): return self._msg_id
    @property
    def msg_id2(self): return self._msg_id2

class MsgContent(ctypes.Structure):
    _fields_ = [
            ('typeid',ContentType)
            ]

class Face(MsgContent):
    _fields_ = MsgContent._fields_ + [
        ('face',ctypes.c_int),
        ]

class Text(MsgContent):
    _fields_ = MsgContent._fields_ + [
        ('text',ctypes.c_char_p)
        ]

class Img(MsgContent):
    _fields_ = MsgContent._fields_+ [
            ('name',ctypes.c_char_p),
            ('data',ctypes.c_voidp),
            ('size',ctypes.c_size_t),
            ('success',ctypes.c_int),
            ('file_path',ctypes.c_char_p),
            ('url',ctypes.c_char_p)
            ]

class CFace(MsgContent):
    _fields_ = MsgContent._fields_ + [
            ('name',ctypes.c_char_p),
            ('data',ctypes.c_voidp),
            ('size',ctypes.c_size_t),
            ('file_id',ctypes.c_char_p),
            ('key',ctypes.c_char_p),
            ('serv_ip',ctypes.c_char * 24),
            ('serv_port',ctypes.c_char * 8),
            ('url',ctypes.c_char_p)
            ]

class Message(MsgSeq):
    _fields_ = [
            ('super',MsgSeq),
            ('time',c_time_t),
            ('upload_retry',ctypes.c_int),

            ('f_name',ctypes.c_char_p),
            ('f_size',ctypes.c_int),
            ('f_style',FontStyle),
            ('f_color',ctypes.c_char * 7),
            ('content',TAILQ_HEAD),
            ]

class BuddyMessage(Message):
    _fields_ = Message._fields_ + [
            ('from',ctypes.c_void_p)
            ]
    TypeID = MsgType.MS_BUDDY_MSG

class GroupMessage(Message):
    _fields_ = Message._fields_ + [
            ('send',ctypes.c_char_p),
            ('group_code',ctypes.c_char_p)
            ]

GroupWebMessage = GroupMessage

class SessMessage(Message):
    _fields_ = Message._fields_ + [
            ('id',ctypes.c_char_p),
            ('group_sig',ctypes.c_char_p),
            ('service_type',ServiceType)
            ]

class DiscuMessage(GroupMessage):
    _fields_ = GroupMessage._fields_

def register_library(lib):
    lib.lwqq_msg_free.argtypes = [ctypes.POINTER(Msg)]

register_library(lib)
