#! /usr/bin/python
# test passing an empty map as an argument, bnc#516492
import dbus
T_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                   '/org/opensuse/YaST/modules/TEST')

# A) explicit typing
YT = dbus.Interface(T_o, 'org.opensuse.YaST.YCPValues')
yp = (False, "map", dbus.Dictionary(signature="sv", variant_level=1))
yrp = YT.ParamMap(yp)
print "Explicit returned:", yrp
assert yrp[0] == False
assert yrp[1] == "map"
rp = yrp[2]
assert isinstance(rp, dict)
assert len(rp.values()) == 0

# B) implicit typing
T = dbus.Interface(T_o, 'org.opensuse.YaST.Values')
# p = {} # ValueError: Unable to guess signature from an empty dict
p = dbus.Dictionary(signature="sv")
rp = T.ParamMap(p)
print "Implicit returned:", rp
assert isinstance(rp, dict)
assert len(rp.values()) == 0
