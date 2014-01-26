from ..core import *
from ..enumerations import *
from ctypes import c_voidp

print(Lwqq.time())
Lwqq.log_level(3)
a = 3

lc = Lwqq('2501542492','1234567890')
lc.login(Status.ONLINE)
def a():
    print("hi")
ev = lc.relink().addListener(a)
ev.addListener(a)
Event.new(x).addListener(a).finish()
lc.logout()
