from ..core import *
from ..enumerations import *

print(Lwqq.time())
Lwqq.log_level(3)
a = 3

def relink():
    print(a)

lc = Lwqq('2501542492','1234567890')
lc.relink().addListener(relink)
lc.logout()
