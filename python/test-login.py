from lwqq.core import *
from lwqq.enumerations import *
from lwqq.vplist import *
from ctypes import c_voidp,cast,POINTER,c_int,byref,c_char_p,pointer

from gevent.core import loop
from gevent import with_timeout,spawn,joinall
import gevent as g

def start_login():
    print("===start_login===")

def load_complete():
    print("===load_complete===")
    print()

def start_login2():
    print("===start login 2===")

def need_verify():
    p

def init_listener(lc):
    print(type(lc.args.login_ec))
    lc.addListener(lc.events.login_complete,
            Command.make('void',load_complete))
    lc.addListener(lc.events.start_login,Command.make('void',start_login))
    lc.addListener(lc.events.start_login,Command.make('void',start_login2))
    lc.addListener(lc.events.need_verify,Command.make('2p',need_verify))

print(Lwqq.time())
Lwqq.log_level(3)
lc = Lwqq('2501542492','1234567890+12345')
init_listener(lc)
lc.sync(1)

def dispatch(cmd,delay):
    with_timeout(delay/1000,lambda : cmd.invoke())
    pass

lc.login(Status.ONLINE)
ev = lc.relink()
lc.setDispatcher(dispatch)
lc.logout()
