from lwqq.types import *
from lwqq.vplist import *
from lwqq.lwqq import *
from lwqq.core import Event
from lwqq import core
from lwqq import lwjs
from lwqq.msg import *
from ctypes import c_voidp,cast,POINTER,c_int,byref,c_char_p,pointer,CFUNCTYPE,py_object
from tornado.ioloop import IOLoop
import urllib.request

loop = IOLoop.instance()

def start_login(p_login_ec):
    login_ec = cast(p_login_ec,POINTER(c_int))[0]
    print("login_ec",login_ec)
    print("===start_login===")

def need_verify(p_vf_image):
    vf = core.VerifyCode(cast(p_vf_image,POINTER(core.VerifyCode.PT))[0])
    path = "/tmp/"+lc.username.decode("utf-8")+".jpg"
    vf.save(path)
    print("need verify image, see: "+path)
    code = input("Input:")
    vf.input(code.encode("utf-8"))
    pass

def message_cb():
    for msg in lc.msg_list.read():
        print(msg.trycast(BuddyMessage))
        print(msg.typeid)
        if msg.trycast(Message):
            m = Message(msg.ref)
            print(m.sender)
            print(str(m))
        msg.destroy()

def message_lost():
    lc.logout()
    exit(0)

def init_listener(lc):
    lc.addListener(lc.events.start_login,Command.make('p',start_login,lc.args.login_ec))
    lc.addListener(lc.events.need_verify,Command.make('p',need_verify,lc.args.vf_image))
    lc.addListener(lc.events.login_complete,load_info)
    lc.addListener(lc.events.poll_msg,message_cb)
    lc.addListener(lc.events.poll_lost,message_lost)

def load_info():
    if not core.has_feature(Features.WITH_MOZJS):
        print("not support js")
        exit(-1)
    else:
        js = lwjs.Lwjs()
        hashjs = urllib.request.urlopen("http://pidginlwqq.sinaapp.com/hash.js")
        js.load(hashjs.read())
        lc.get_friends_info(js.hashfunc,js.js).addListener(poll_msg)
    pass

def poll_msg():
    lc.msg_list.poll(0)

def local_thread(cmd):
    cmd.invoke()
    pass

def dispatch(cmd,timeout):
    loop.add_callback(local_thread,cmd)
    pass

def main():
    lc.login(Status.ONLINE)
    pass


print(Lwqq.time())
Lwqq.log_level(3)
lc = Lwqq(b'2501542492',b'1234567890+12345')
lc.setDispatcher(dispatch)
init_listener(lc)
loop.add_callback(main)
loop.start()

