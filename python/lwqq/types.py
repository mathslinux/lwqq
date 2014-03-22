from ctypes import c_int

__all__ = [ 'Status', 'Features', 'ErrorCode', 'PollFlags', 'MsgType',
        'ContentType', 'FontStyle', 'GroupType', 'MsgSystemType',
        'MsgSysGType', 'Constel', 'BloodType', 'ShengXiao', 'Gender',
        'ClientType', 'MemberFlag', 'MaskType', 'DelType', 'Answer' ]

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

class GroupType(c_int):
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

class Constel(c_int):
    AQUARIUS    = 1
    PISCES      = 2
    ARIES       = 3
    TAURUS      = 4
    GEMINI      = 5
    CANCER      = 6
    LEO         = 7
    VIRGO       = 8
    LIBRA       = 9
    SCORPIO     = 10
    SAGITTARIUS = 11
    CAPRICORNUS = 12

class BloodType(c_int):
    A     = 1
    B     = 2
    O     = 3
    AB    = 4
    OTHER = 5

class ShengXiao(c_int):
    MOUTH  = 1
    CATTLE = 2
    TIGER  = 3
    RABBIT = 4
    DRAGON = 5
    SNACK  = 6
    HORSE  = 7
    SHEEP  = 8
    MONKEY = 9
    CHOOK  = 10
    DOG    = 11
    PIG    = 12

class Gender(c_int):
    FEMALE = 1
    MALE   = 2

class ClientType(c_int):
    PC       = 1  # /*1-10*/
    MOBILE   = 21 # /*21-24*/
    WEBQQ    = 41
    QQFORPAD = 42

class MemberFlag(c_int):
    IS_ADMIN = 0x1

class MaskType(c_int):
    NONE = 0
    ONE  = 1
    ALL  = 2

class DelType(c_int):
    KEEP_OTHER = 1 #/* delete buddy and keep myself from other buddy list */
    FROM_OTHER = 2 #/* delete buddy and remove myself from other buddy list */

class Answer(c_int):
    NO            = 0
    YES           = 1
    EXTRA_ANSWER  = 2
    IGNORE        = 3
    ALLOW_ANS_ADD = EXTRA_ANSWER
