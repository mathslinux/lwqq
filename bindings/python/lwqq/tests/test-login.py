from ..core import *
from ..enumerations import *
from ctypes import c_voidp

print(Lwqq.time())
Lwqq.log_level(3)
a = 3

lc = Lwqq('2501542492','1234567890')
lc.sync(1)
lc.login(Status.ONLINE)
ev = lc.relink()
def a():
    print(ev.raw.result)
ev.addListener(a)
lc.logout()
