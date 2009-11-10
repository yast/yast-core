#!/usr/bin/env python

# a simple example showing how to integrate Yast Dbuse service into KDE desktop

from polkit_helper import retry # in .
import dbus
import sys

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
	samba = bus.get_object('org.opensuse.YaST.modules', '/org/opensuse/YaST/modules/YaPI/Samba')

	call = lambda: samba.AddShare(share_name, {'path':directory, 'comment':'Exported directory ' + directory, 'read only':'Yes'},
				      dbus_interface='org.opensuse.YaST.Values')
	result = retry(call)
	print "Result:", result

if __name__ == "__main__":
    main()
