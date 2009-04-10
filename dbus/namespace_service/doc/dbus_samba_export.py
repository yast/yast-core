#!/usr/bin/env python

# a simple example showing how to integrate Yast Dbuse service into KDE desktop

import dbus
import sys
import os
import re

def main():

    if len(sys.argv) > 1:
	
	directory = sys.argv[1]

	if len(directory) == 0:
	    print 'Error: empty argument'
	    exit(1)

	print 'Exporting directory ' + directory
	export_dir(directory)

    else:
	print 'Error: missing argument'
	exit(1)

def export_dir(directory):
	share_name = directory.replace('/','_')

	if share_name[0] == '_':
	    share_name = share_name[1:]

        bus = dbus.SystemBus()
	samba = bus.get_object('org.opensuse.YaST.modules', '/org/opensuse/YaST/modules/YaPI__Samba')

	call = lambda: samba.AddShare(share_name, {'path':directory, 'comment':'Exported directory ' + directory, 'read only':'Yes'},
				      dbus_interface='org.opensuse.YaST.Values')
	result = polkit_retry(call)
	print "Result:", result

def polkit_retry(alambda):
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

if __name__ == "__main__":
    main()
