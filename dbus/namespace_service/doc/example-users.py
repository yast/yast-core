#! /usr/bin/python
# example-users.py
# see example-arch.py
#
# As root:
# # polkit-auth --user $USER --grant org.opensuse.yast.modules.yapi-users.usersget
# work around a bug in yast2-core-2.18.6:
# # polkit-auth --user $USER --grant org.opensuse.yast.module-manager.modules.import

from polkit_helper import retry # in .
import dbus

bus = dbus.SystemBus()
O = bus.get_object              # shorthand
I = dbus.Interface              # shorthand
MOD_S = 'org.opensuse.YaST.modules'

import sys
if len(sys.argv) > 1 and sys.argv[1] == "--explicit":
    mm_o = O(MOD_S, '/org/opensuse/YaST/modules')
    mm = I(mm_o, 'org.opensuse.YaST.modules.ModuleManager')
    # polkit will ask
    retry(lambda: mm.Import('YaPI::USERS'))

Users_o = O(MOD_S, '/org/opensuse/YaST/modules/YaPI__USERS')
Users = I(Users_o, 'org.opensuse.YaST.Values')
cfg = {
    "type": "system",
    "index": "uid",
    }
call = lambda: Users.UsersGet(cfg)
print "System users:", retry(call)
# it returns an empty hash, weird. but it works through, yay.
