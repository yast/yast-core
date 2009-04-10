#! /usr/bin/python
# example-arch.py
# Test http://svn.opensuse.org/svn/yast/branches/tmp/lslezak/core/dbus/namespace_service

# By default nobody, not even root, can call anything.
# Let's grant us a permission for a call for which there is no policyconfig
# (in /usr/share/PolicyKit/policy/org.opensuse.yast.modules.*) yet:
# As root:
# # polkit-auth --user $USER --grant org.opensuse.yast.modules.arch.is-laptop

# Start the service from an xterm. If you let it autostart, it will not
# be able to ask for PolicyKit authorization for --explicit.
# (That's because whe have the polkit logic wrong and
# ask in the server instead of in the client. TODO.)
# As root:
# # /usr/lib/YaST2/bin/yast_modules_dbus_server --disable-timer &
# # polkit-auth --user $USER --revoke org.opensuse.yast.modules.arch.is-laptop

import dbus
bus = dbus.SystemBus()
O = bus.get_object              # shorthand
I = dbus.Interface              # shorthand
MOD_S = 'org.opensuse.YaST.modules'

import sys
if len(sys.argv) > 1 and sys.argv[1] == "--explicit":
    mm_o = O(MOD_S, '/')
    mm = I(mm_o, 'org.opensuse.YaST.modules.ModuleManager')
    # polkit will ask
    mm.Import('Arch')

Arch_o = O(MOD_S, '/org/opensuse/YaST/modules/Arch')
Arch = I(Arch_o, 'org.opensuse.YaST.Values')
# polkit will refuse
print "Is this a laptop:", Arch.is_laptop()
