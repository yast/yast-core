#
# Makefile.am for core/agents-non-y2
#

SUBDIRS = conf

# liby2 only distinguishes between server and clients,
# since agents do a "service", they go to "servers_non_y2"
agentdir = $(execcompdir)/servers_non_y2
agent_SCRIPTS = ag_hostnames

EXTRA_DIST = $(agent_SCRIPTS)
