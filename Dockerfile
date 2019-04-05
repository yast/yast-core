FROM registry.opensuse.org/yast/head/containers/yast-cpp:latest
# the tests require specific locale settings to pass
ENV LANG=POSIX LC_ALL=
# Remove the preinstalled yast2-core, it interferes with the built one
# when running the tests... (huh??)
RUN zypper --non-interactive rm yast2-core
RUN zypper --non-interactive in jemalloc-devel
COPY . /usr/src/app
