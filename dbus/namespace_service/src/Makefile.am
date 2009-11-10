#
# Makefile.am for dbus/service
#

EXTRA_DIST=org.opensuse.YaST.modules.service.in.in org.opensuse.YaST.modules.conf.in org.opensuse.yast.module-manager.policy.in \
    yast_dbus_names.h.in

AM_CXXFLAGS = -DY2LOG=\"DBus-service\" -DSUSEVERSION=\"${SUSEVERSION}\"

INCLUDES = ${AGENT_INCLUDES} -I$(includedir)

AM_LDFLAGS = -L${libdir}

ybin_PROGRAMS = yast_modules_dbus_server

yast_modules_dbus_server_SOURCES = yast_modules_dbus_server.cc DBusModulesServer.cc DBusModulesServer.h

yast_modules_dbus_server_CPPFLAGS = $(AM_CPPFLAGS) $(DBUS_CFLAGS) $(POLKIT_CFLAGS) -I$(top_builddir)/liby2dbus/src -I$(includedir) -I$(top_builddir)/wfm/src

yast_modules_dbus_server_LDADD = ${AGENT_LIBADD} $(top_builddir)/liby2dbus/src/liby2dbus.la $(top_builddir)/wfm/src/libpy2wfm.la

# PolicyKit defaults
polkit_policiesdir = `pkg-config --print-errors --variable policydir polkit`
polkit_policies_DATA = org.opensuse.yast.module-manager.policy

# validate the policy files in 'make check' target
check-local:
	polkit-policy-file-validate $(polkit_policies_DATA)


# service activation config

# FIXME?? dbus pkgconfig provides only 'session_bus_services_dir'
dbus_servicesdir = `pkg-config --print-errors --variable prefix dbus-1`/share/dbus-1/system-services

# Can't use autoconf only because @ybindir@ expands to '${prefix}/...'. 
# So must use %YBINDIR% and expand it via sed during make
# Ouch, ouch, ouch
dbus_services_in_files = org.opensuse.YaST.modules.service.in
dbus_services_DATA = $(dbus_services_in_files:.service.in=.service)
$(dbus_services_DATA): $(dbus_services_in_files)
	sed -e "s|\%YBINDIR\%|$(ybindir)|" $< > $@

# service config
dbus_confdir = `pkg-config --print-errors --variable sysconfdir dbus-1`/dbus-1/system.d
dbus_conf_DATA = org.opensuse.YaST.modules.conf

