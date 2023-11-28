## libycp testsuite

### Overview

The ycp testsuite tests the libycp by running all .ycp files in
the ycp.test directory (tests/*.ycp) through runycp.

Every run results in a stdout and stderr output which is
checked against .out and .err respectively.

The file lib/ycp.exp defines a function which runs all tests
contained in tests (i.e. makes a 'glob' on tests/*.ycp)

For every .ycp file you put in tests, you must create the proper
.out and .err file with the following command

runycp < file.ycp > file.out 2> file.err



### Running all Tests

```
make check
```


### Fixing a Failed Test

After running the tests, the `testsuite` directory contains the _actual_ stdout
and stderr files for each individual test, e.g.

- `tmp.out.Builtin-List2`
- `tmp.err.Builtin-List2`

For a failing test, use `diff` to compare the _expected_ stdout and stderr file
in the subdirectory that contains the test against those _actual_ files.

If you are sure that the difference is legitimate, copy the actual stdout file
to the expected stdout file and the actual stderr fil to the expected stderr
file and commit the file(s) to Git.

Check the diff:

```
cd tests/builtin

diff ../../tmp.out.Builtin-List2 Builtin-List2.out
diff ../../tmp.err.Builtin-List2 Builtin-List2.err
```

Looks okay? Then:

```
cp ../../tmp.out.Builtin-List2 Builtin-List2.out
cp ../../tmp.err.Builtin-List2 Builtin-List2.err
```

Run the tests again to make sure it's now really okay:

```
cd ../..
make check
```

Okay now? Check the changes in:

```
cd tests/builtin

git checkin -a -m "Fixed tests"
git push ...
```


