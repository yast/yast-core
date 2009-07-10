import dbus
import unittest

class DBusTestCase(unittest.TestCase):
  def failUnlessRaisesDBus(self, dbusErrName, callableObj, *args, **kwargs):
    """Fail unless a DBusException whose name contains dbusErrName is thrown
    by callableObj when invoked with arguments args and keyword
    arguments kwargs. If a different type of exception is
    thrown, it will not be caught, and the test case will be
    deemed to have suffered an error, exactly as for an
    unexpected exception.
    """
    excClass = dbus.exceptions.DBusException
    try:
      callableObj(*args, **kwargs)
    except excClass, e:
      got = e.get_dbus_name()
      if got.find(dbusErrName) != -1:
        return
      raise self.failureException, "Expected DBus exception matching %s but got %s" % (dbusErrName, got)
    else:
      if hasattr(excClass,'__name__'): excName = excClass.__name__
      else: excName = str(excClass)
      raise self.failureException, "%s not raised" % excName

  assertRaisesDBus = failUnlessRaisesDBus
