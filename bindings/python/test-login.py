from lwqq.core import *
from lwqq.enumerations import *
from lwqq.vplist import *
from ctypes import c_voidp,cast,POINTER,c_int,byref,c_char_p,pointer

from gevent.core import loop
from gevent import with_timeout,spawn,joinall
import gevent as g

def load_complete(err):
    print("===load_complete===")
    args = cast(err,POINTER(Arguments))
    print(args[0])
    print()

def init_listener(lc):
    print(type(lc.args.login_ec))
    lc.addListener(lc.events.login_complete,
            Command.make('p',load_complete,byref(lc.args)))
    #lc.addListener(lc.events.contents.start_login,Command.make(load_complete))

print(Lwqq.time())
Lwqq.log_level(3)
lc = Lwqq('2501542492','1234567890')
init_listener(lc)
lc.sync(1)

def dispatch(cmd,delay):
    with_timeout(delay/1000,lambda : cmd.invoke())
    pass

lc.login(Status.ONLINE)
ev = lc.relink()
lc.setDispatcher(dispatch)
lc.logout()
