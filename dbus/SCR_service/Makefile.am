#
# Makefile.am for dbus/service
#

EXTRA_DIST=org.opensuse.yast.SCR.service.in org.opensuse.yast.SCR.conf.in

AM_CXXFLAGS = -DY2LOG=\"SCR-service\" -DSUSEVERSION=\"${SUSEVERSION}\"

INCLUDES = ${AGENT_INCLUDES}

ybin_PROGRAMS = SCR_dbus_server

SCR_dbus_server_SOURCES = SCR_dbus_server.cc DBusServer.cc DBusServer.h

SCR_dbus_server_CPPFLAGS = $(AM_CPPFLAGS) $(DBUS_CFLAGS) $(POLKIT_CFLAGS) -I$(top_builddir)/liby2dbus/src -I$(srcdir)/../../scr/src -I$(top_builddir)/libscr/src/include

SCR_dbus_server_LDADD = ${AGENT_LIBADD} $(top_builddir)/liby2dbus/src/liby2dbus.la $(top_builddir)/scr/src/libpy2scr.la


# service activation config

# FIXME?? dbus pkgconfig provides only 'session_bus_services_dir'
dbus_servicesdir = `pkg-config --print-errors --variable prefix dbus-1`/share/dbus-1/system-services

dbus_services_in_files = org.opensuse.yast.SCR.service.in

dbus_services_DATA = $(dbus_services_in_files:.service.in=.service)

$(dbus_services_DATA): $(dbus_services_in_files)
	sed -e "s|\@YBINDIR\@|$(ybindir)|" $< > $@

# service config

dbus_confdir = `pkg-config --print-errors --variable sysconfdir dbus-1`/dbus-1/system.d

dbus_conf_in_files = org.opensuse.yast.SCR.conf.in

dbus_conf_DATA = $(dbus_conf_in_files:.conf.in=.conf)

# set access mode according to the PolicyKit status
# if policykit is disabled only root can access the service
if HAVE_POLKIT
dbus_access_mode=allow
else
dbus_access_mode=deny
endif

$(dbus_conf_DATA): $(dbus_conf_in_files)
	sed -e "s|\@ACCESS_MODE\@|$(dbus_access_mode)|" $< > $@

