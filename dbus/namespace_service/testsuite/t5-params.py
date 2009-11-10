#! /usr/bin/python
import dbus
import unittest

# define representative values of various types, named after their signatures
b42 = True
y42 =                   42
n42 =                 4200
q42 =                42000
i42 =           2100000000
u42 =           4200000000
x42 =  4200000000000000000
t42 = 18000000000000000000
#   ^   E  P  T  G  M  K
d42 = 4.2
s42 = "fortytwo"
o42 = "/Forty/Two"
g42 = "tatastavi"

b = True
y = dbus.Byte(y42)
n = dbus.Int16(n42)
q = dbus.UInt16(q42)
i = dbus.Int32(i42)
# TODO make them big enough
u = dbus.UInt32(u42)
x = dbus.Int64(x42)
t = dbus.UInt64(t42)        # too big for YCP
d = dbus.Double(d42)
s = dbus.String(s42)
o = dbus.ObjectPath(o42)
g = dbus.Signature(g42)
#v, a

class ParamPassing(unittest.TestCase):

  def setUp(self):
    T_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/org/opensuse/YaST/modules/TEST')
    self.T  = dbus.Interface(T_o, 'org.opensuse.YaST.Values')
#    self.YT = dbus.Interface(T_o, 'org.opensuse.YaST.YCPValues')

  def testBoolTrue(self):
    self.assertEqual(self.T.ParamBoolean(True), True)

  def testBoolFalse(self):
    self.assertEqual(self.T.ParamBoolean(False), False)

  def testByte(self):
    self.assertEqual(self.T.ParamInteger(y), y42)

  def testInt16(self):
    self.assertEqual(self.T.ParamInteger(n), n42)

  def testUInt16(self):
    self.assertEqual(self.T.ParamInteger(q), q42)

  def testInt32(self):
    self.assertEqual(self.T.ParamInteger(i), i42)

  def testUInt32(self):
    self.assertEqual(self.T.ParamInteger(u), u42)

  def testInt64(self):
    self.assertEqual(self.T.ParamInteger(x), x42)

# python says: OverflowError: long too big to convert
#  def testUInt64(self):
#    self.assertEqual(self.T.ParamInteger(t), t42)

  def testDouble(self):
    self.assertEqual(self.T.ParamFloat(d), d42)

  def testString(self):
    self.assertEqual(self.T.ParamString(s), s42)

  def testList(self):
    lst = [1, 2, 3]
    self.assertEqual(self.T.ParamList(lst), lst)

#Term is not exported
#  def testTerm(self):
#    term = ["name", "two", "three"]
#    self.assertEqual(self.T.ParamTerm(term), term)

if __name__ == '__main__':
    unittest.main()
