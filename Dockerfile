FROM yastdevel/cpp-tw
# the tests require specific locale settings to pass
ENV LANG=POSIX LC_ALL=
COPY . /tmp/sources

