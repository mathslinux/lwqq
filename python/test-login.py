from lwqq.enumerations import *
from lwqq.vplist import *
from lwqq.lwqq import *
from lwqq.core import Event
from lwqq import core
from lwqq import lwjs
from ctypes import c_voidp,cast,POINTER,c_int,byref,c_char_p,pointer
import urllib.request 

def start_login(p_login_ec):
    login_ec = cast(p_login_ec,POINTER(c_int))[0]
    print("login_ec",login_ec)
    print("===start_login===")

def start_login2():
    print("===start login 2===")

def need_verify(p_vf_image):
    vf = core.VerifyCode(cast(p_vf_image,POINTER(core.VerifyCode.PT))[0])
    path = "/tmp/"+lc.username.decode("utf-8")+".jpg"
    vf.save(path)
    print("need verify image, see: "+path)
    code = input("Input:")
    vf.input(code.encode("utf-8"))
    pass


def login_complete():
    load_info(lc)
    ev = lc.relink()
    lc.logout()

def load_complete():
    print("===load complete===")

def init_listener(lc):
    lc.addListener(lc.events.start_login,Command.make('p',start_login,lc.args.login_ec))
    lc.addListener(lc.events.need_verify,Command.make('p',need_verify,lc.args.vf_image))
    lc.addListener(lc.events.login_complete,Command.make('void',login_complete))

def load_info(lc):
    if not core.has_feature(Features.WITH_MOZJS):
        print("not support js")
        exit(-1)
    else:
        js = lwjs.Lwjs()
        hashjs = urllib.request.urlopen("http://pidginlwqq.sinaapp.com/hash.js")
        js.load(hashjs.read())
        ev = Event(lc.get_friends_info(js.hashfunc,js.js))
        ev.addListener(load_complete)
    pass



print(Lwqq.time())
Lwqq.log_level(3)
lc = Lwqq(b'2501542492',b'1234567890+12345')
init_listener(lc)
lc.sync(1)


lc.login(Status.ONLINE)
