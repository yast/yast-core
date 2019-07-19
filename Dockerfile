FROM yastdevel/cpp:sle12-sp5


# the tests require specific locale settings to pass
ENV LANG=POSIX LC_ALL=
# Remove the preinstalled yast2-core, it interferes with the built one
# when running the tests... (huh??)
RUN zypper --non-interactive rm yast2-core
COPY . /usr/src/app
