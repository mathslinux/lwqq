from lwqq.enumerations import *
from lwqq.vplist import *
from lwqq.lwqq import *
from ctypes import c_voidp,cast,POINTER,c_int,byref,c_char_p,pointer

def start_login(p_login_ec):
    login_ec = cast(p_login_ec,POINTER(c_int))[0]
    print("login_ec",login_ec)
    print("===start_login===")

def load_complete():
    print("===load_complete===")
    print()

def start_login2():
    print("===start login 2===")

def need_verify():
    pass

def init_listener(lc):
    lc.addListener(lc.events.start_login,Command.make('p',start_login,lc.args.login_ec))
    lc.addListener(lc.events.need_verify,Command.make('2p',need_verify))
    lc.addListener(lc.events.login_complete,Command.make('void',load_complete))

print(Lwqq.time())
Lwqq.log_level(3)
lc = Lwqq(b'2501542492',b'1234567890+12345')
init_listener(lc)
lc.sync(1)


lc.login(Status.ONLINE)
ev = lc.relink()
lc.logout()
