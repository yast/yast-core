#! /usr/bin/python
# test various kinds of parameter errors
# -bad param type
#TODO for other cases:
# - unknown namespace
# object not found
# etc, check y2errors
import dbus
import unittest
import dbustest

class BadParams(dbustest.DBusTestCase):
  def setUp(self):
    T_o = dbus.SessionBus().get_object('org.opensuse.YaST.modules',
                                       '/org/opensuse/YaST/modules/TEST')
    self.T  = dbus.Interface(T_o, 'org.opensuse.YaST.Values')
    self.YT = dbus.Interface(T_o, 'org.opensuse.YaST.YCPValues')

  # dbus-Python checks these itself because of the introspection.
  # TODO work around it and do send such invalid calls
  def testRawMissingParam(self):
    self.assertRaises(Exception, self.T.ParamMap)

  def testRawTypeMismatch(self):
    self.assertRaises(Exception, self.T.ParamMap, "I am a string, not a map")

  def testRawSuperfluousParam(self):
    self.assertRaises(Exception, self.T.ParamMap, {"foo":"bar"}, "I am superfluous")

#Empty type means string
#  def testBsvNil(self):
#    "type is required even for nil, dubious"
#    self.assertRaises(Exception, self.YT.ParamMap, (True, "", ""))

#  def testBsvTypeRequired(self):
#    self.assertRaises(Exception, self.YT.ParamMap, (False, "", ""))

  def testBsvUnknownType(self):
    self.assertRaisesDBus("InvalidSignature",
                          self.YT.ParamMap, (False, "ufo", 0))

  def testBsvUnimplementedType(self):
    self.assertRaisesDBus("InvalidSignature",
                          self.YT.ParamMap, (False, "byteblock", 0))

  def testBsvTypeMismatch(self):
    self.assertRaises(Exception, self.YT.ParamMap, (False, "string", 42))

  def testBsvBadNilFlag(self):
    badBsv = (42,)  # (42) is a plain number
    # wrap it otherwise the method sig would refuse it
    self.assertRaisesDBus("InvalidArgs",
                          self.YT.ParamAny, (False, "list", [badBsv]))

  def testBsvBadTypeField(self):
    badBsv = (False, 42, 42)
    # wrap it otherwise the method sig would refuse it
    self.assertRaisesDBus("InvalidArgs",
                          self.YT.ParamAny, (False, "list", [badBsv]))

  def testBsvBadPayload(self):
    badBsv = (False, "", "Not A Variant")
    # wrap it otherwise the method sig would refuse it
    self.assertRaisesDBus("InvalidArgs",
                          self.YT.ParamAny, (False, "list", [badBsv]))

  def testUnhandledDBusType(self):
    struct = ("a", "struct")
    # actually we could convert structs to ycp lists (or terms)
    # but that would be spoiling the user too much
    self.assertRaisesDBus("InvalidArgs", self.T.ParamAny, struct)

  def testTermInvalid(self):
    "Terms are passed as lists where first item must be string"
    term = [1 , 2, 3]
    self.assertRaises(Exception, self.T.ParamTerm, term)


if __name__ == '__main__':
    unittest.main()
