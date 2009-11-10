#! /usr/bin/python
# example-arch.py

# By default nobody, not even root, can call anything.
# Let's grant us a permission for a call for which there is no policyconfig
# (in /usr/share/PolicyKit/policy/org.opensuse.yast.modules.*) yet:
# As root:
# # polkit-auth --user $USER --grant org.opensuse.yast.modules.arch.is-laptop

# Revoke the permission (as root):
# # polkit-auth --user $USER --revoke org.opensuse.yast.modules.arch.is-laptop

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
    mm.Import('Arch')

Arch_o = O(MOD_S, '/org/opensuse/YaST/modules/Arch')
Arch = I(Arch_o, 'org.opensuse.YaST.Values')
# polkit will refuse
print "Is this a laptop:", Arch.is_laptop()
