#! /usr/bin/python
# test calling an unknown method
import dbus
import unittest
import dbustest

class Unknown(dbustest.DBusTestCase):
  def setUp(self):
    T_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/org/opensuse/YaST/modules/TEST')
    self.T  = dbus.Interface(T_o, 'org.opensuse.YaST.Values')
    self.YT = dbus.Interface(T_o, 'org.opensuse.YaST.YCPValues')

  def testUnknownMethodNoParams(self):
    self.assertRaisesDBus("UnknownMethod", self.T.NoSuchMethod)

  def testUnknownMethodWithParams(self):
    self.assertRaisesDBus("UnknownMethod", self.T.NoSuchMethod, 42)

  def testUnknownObject(self):
    U_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/borg/resistance/futile')

  def testUnknownModuleImplicit(self):
    U_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/org/opensuse/YaST/modules/UNKNOWN')
    # ^ HUH, no exception?
    U  = dbus.Interface(U_o, 'org.opensuse.YaST.Values')

  def testUnknownModuleExplicit(self):
    MM_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/org/opensuse/YaST/modules')
    MM = dbus.Interface(MM_o, 'org.opensuse.YaST.modules.ModuleManager')
    self.assertFalse(MM.Import("UNKNOWN"))
# TODO more sense to raise on error than return false
#    self.assertRaisesDBus("", MM.Import, "UNKNOWN")

  def testUnknownMethodBsvNoParams(self):
    self.assertRaisesDBus("UnknownMethod", self.YT.NoSuchMethod)

  def testUnknownMethodBsvWithParams(self):
    self.assertRaisesDBus("UnknownMethod", self.YT.NoSuchMethod, (False, "", ""))

if __name__ == '__main__':
    unittest.main()
