import dbus
import re
import os

def retry(alambda):
    """Calls a function, obtaining authorization if required.

    Instead of RET = FOO(BAR, BAZ)
    Call RET = retry(lambda: FOO(BAR, BAZ))
    """
    try:
        result = alambda()
    except dbus.exceptions.DBusException, e:
        if e.get_dbus_name() == 'org.freedesktop.PolicyKit.Error.NotAuthorized':
            message = e.get_dbus_message()

            # get result and action name
            parts = re.split(' +', message)

            if len(parts) == 2 and re.match("auth_.*", parts[1]):
                print 'Authorization ' + parts[0] + ' is needed'
                polkit_res = obtain_authorization(parts[0])
                print 'Authorization obtained: ' + str(polkit_res)

                result = alambda()
            else:
                print 'PolicyKit error: ' + message
                raise
        else:
            raise
    return result

def obtain_authorization(action_id):
    session_bus = dbus.SessionBus()
    window_id = 0
    polkit_obj = session_bus.get_object('org.freedesktop.PolicyKit.AuthenticationAgent', '/');
    polkit_res = polkit_obj.ObtainAuthorization(action_id, window_id, os.getpid(), dbus_interface='org.freedesktop.PolicyKit.AuthenticationAgent')
    return polkit_res
