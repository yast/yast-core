FROM yastdevel/cpp-tw
# the tests require specific locale settings to pass
ENV LANG=POSIX LC_ALL=
COPY . /tmp/sources
# Remove the preinstalled yast2-core, it interferes with the built one
# when running the tests... (huh??)
RUN zypper --non-interactive rm yast2-core
