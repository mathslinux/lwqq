from ..core import *
from ..enumerations import *
from ctypes import c_voidp

from gevent.core import loop
from gevent import with_timeout,spawn,joinall
import gevent as g

print(Lwqq.time())
Lwqq.log_level(3)
a = 3

lc = Lwqq('2501542492','1234567890')

def dispatch(cmd,delay):
    with_timeout(delay/1000,lambda : cmd.invoke())
    pass

def main():
    lc.login(Status.ONLINE)
    ev = lc.relink()
    #ev.addListener(a)

lc.setDispatcher(dispatch)
g.get_hub().run()


lc.logout()
