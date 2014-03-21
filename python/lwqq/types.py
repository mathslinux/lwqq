from ctypes import c_int

__all__ = [ 'Status', 'Features', 'ErrorCode', 'PollFlags', 'MsgType',
        'ContentType', 'FontStyle', 'ServiceType', 'MsgSystemType',
        'MsgSysGType' ]

class Features(c_int):
    WITH_LIBEV  = 1<<0
    WITH_LIBUV  = 1<<1
    WITH_SQLITE = 1<<2
    WITH_MOZJS  = 1<<3

class Status(c_int):
    LOGOUT  = 0
    ONLINE  = 10
    OFFLINE = 20
    AWAY    = 30
    HIDDEN  = 40
    BUSY    = 50
    CALLME  = 60
    SLIENT  = 70

class ErrorCode(c_int):
    DB_EXEC_FAILED      = -50
    NOT_JSON_FORMAT     = -30
    #upload error code
    UPLOAD_OVERSIZE     = -21
    UPLOAD_OVERRETRY    = -20
    #network error code
    HTTP_ERROR          = -11
    NETWORK_ERROR       = -10
    #system error code
    FILE_NOT_EXIST      = -6
    NULL_POINTER        = -5
    CANCELED            = -4
    TIMEOUT_OVER        = -3
    NO_RESULT           = -2
    ERROR               = -1

    #webqq error code
    OK                  = 0
    LOGIN_NEED_VC       = 10
    HASH_WRONG          = 50
    LOGIN_ABNORMAL      = 60
    NO_MESSAGE          = 102
    COOKIE_WRONG        = 103
    PTWEBQQ             = 116
    LOST_CONN           = 121

class PollFlags(c_int):
    AUTO_DOWN_GROUP_PIC   = 1<<0
    AUTO_DOWN_BUDDY_PIC   = 1<<1
    AUTO_DOWN_DISCU_PIC   = 1<<2
    REMOVE_DUPLICATED_MSG = 1<<3

class MsgType(c_int):
    MF_SEQ = 1<<1
    MT_MESSAGE = 1<<3|MF_SEQ
    MS_BUDDY_MSG = MT_MESSAGE|(1<<8)
    MS_GROUP_MSG = MT_MESSAGE|(2<<8)
    MS_DISCU_MSG = MT_MESSAGE|(3<<8)
    MS_SESS_MSG = MT_MESSAGE|(4<<8)
    MS_GROUP_WEB_MSG = MT_MESSAGE|(5<<8)

    MT_STATUS_CHANGE = 2<<3
    MT_KICK_MESSAGE = 3<<3
    MT_SYSTEM = MF_SEQ|(4<<3)
    #MS_ADD_BUDDY = MT_SYSTEM|(1<<8)
    #MS_VERIFY_PASS = MT_SYSTEM|(2<<8)
    #MS_VERIFY_PASS_ADD = MT_SYSTEM|(3<<8)
    #MS_VERIFY_REQUIRE = MT_SYSTEM|(4<<8)

    MT_BLIST_CHANGE = 5<<3
    MT_SYS_G_MSG = MF_SEQ|(6<<3)
    #MS_G_CREATE = MT_SYS_G_MSG|(1<<8)
    #MS_G_JOIN = MT_SYS_G_MSG|(2<<8)
    #MS_G_LEAVE = MT_SYS_G_MSG|(3<<8)
    #MS_G_REQUIRE = MT_SYS_G_MSG|(4<<8)

    MT_OFFFILE = MF_SEQ|(7<<3)
    MT_FILETRANS = MF_SEQ|(8<<3)
    MT_FILE_MSG = MF_SEQ|(9<<3)
    MT_NOTIFY_OFFFILE = MF_SEQ|(10<<3)
    MT_INPUT_NOTIFY = 11<<3
    MT_SHAKE_MESSAGE = MF_SEQ|(12<<3)
    MT_UNKNOW = MT_SHAKE_MESSAGE+1
    @classmethod
    def mt(self,typeid): return typeid&~(-1<<8)

class ContentType(c_int):
    TEXT   = 0
    FACE   = 1
    OFFPIC = 2
    CFACE  = 3

class FontStyle(c_int):
    BOLD      = 1<<2
    ITALIC    = 1<<1
    UNDERLINE = 1<<0

class ServiceType(c_int):
    GROUP = 0
    DISCU = 1

class MsgSystemType(c_int):
    VERIFY_REQUIRED    = 0
    VERIFY_PASS_ADD    = 1
    VERIFY_PASS        = 2
    ADDED_BUDDY_SIG    = 3
    SYSTEM_TYPE_UNKNOW = 4

class MsgSysGType(c_int):
    GROUP_CREATE             = 0
    GROUP_JOIN               = 1  #admin or member(when admin operate) get this msg
    GROUP_LEAVE              = 2  #admin or member(when admin operate) get this msg
    GROUP_REQUEST_JOIN       = 3  #only admin get this msg
    GROUP_REQUEST_JOIN_AGREE = 4  #only member get this msg
    GROUP_REQUEST_JOIN_DENY  = 5  #only member get this msg
    GROUP_UNKNOW             = 6
