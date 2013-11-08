# Target Agent

## General

The target agent is the SCR interface to (shell) commands
of the target system.

## Paths

Target agent are attached to `.target` under which lives commands.
Target agent can be also invoked via WFM where it lives under `.local` root.
Difference between WFM `.local` and SCR `.target` is only after SCR switch,
which is used for example in installation, when `.local` always work always on
root `/` and `.target` work on SCR target.

## Commands

### `.bash`
Executes command in bash. _Returns_ exit code of command. _Arguments_ are command
as string and optional map of environment variables.

Example in ruby that calls `halt -p`

```
    exit_code = SCR.Execute(Yast::Path.new(".target.bash"), "halt -p")
```

### `.bash_output`
Executes command in bash. _Returns_ map including `"exit"` for exit code,
`"stdout"` for command stdout and `"stderr"` for command stderr. _Arguments_ are
command as string and optional map of environment variables.

Example in ruby that create temporary file and raise exception if not succeed.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.bash_output"), "mktemp")
    if result["exit"] != 0
      raise "Failed to create temporary file with #{result["stderr"]}"
    end
    file_path = result["stdout"]
```

### `.bash_input`
Executes command in bash and pass string on stdin. _Returns_ exit code.
_Arguments_ are command and string that is given to stdin.

Example in ruby that change password to pwd.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.bash_input"), "passwd", "pwd\npwd\n")
    if result["exit"] != 0
      raise "Failed to change password"
    end
```

### `.bash_background`
Executes command in bash in background. _Returns_ zero if succeed and -1 if
failed. _Arguments_ are command as string and optional map of environment
variables.

Example in ruby that do time consuming job.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.bash_background"), "sleep 1000000")
    if result["exit"] != 0
      raise "I cannot sleep. Do not disturb me!"
    end
```

### `.symlink`
Creates symlink. _Returns_ boolean depending on success. _Arguments_ are two
strings one with source path and second with target one.

Example in ruby create symlink `/tmp2` pointing to `/tmp`

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.symlink"), "/tmp", "/tmp2")
    raise "Creating symlink failed" unless result
```

### `.mkdir`

Creates directory and all its parents. _Returns_ boolean depending on success.
_Arguments_ are string with path and optional integer with mode. If mode is not
specified 0755 is used.

Example in ruby create directory `/tmp/foo/bar` with mode 0700.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.mkdir"), "/tmp/foo/bar", 0700)
    raise "Creating directory failed" unless result
```

### `.remove`
Removes a file. _Returns_ boolean depending on success.
_Argument_ is string with path to file.
+note+: Cannot remove a directory

Example in ruby remove file `/tmp/foo`.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.remove"), "/tmp/foo")
    raise "Removing file failed" unless result
```


Implemented paths are

Execute (.target.bash, string command [, map env])	execute command in bash
Execute (.target.bash_output, string command [, map env]) execute command in bash, return output
Execute (.target.bash_background, string command [, map env]) execute command in bash, don't wait for return
Execute (.target.symlink, string old, string new)	create symbolic link
Execute (.target.mkdir, string dir [, integer mode])	create directory (with mode)
Execute (.target.remove, string file)			remove file
Execute (.target.inject, string file, string path)	inject file to target system
Execute (.target.control.printer_reset, string device)	reset a printer device

Execute (.target.mount, ...)
Execute (.target.umount, ...)
Execute (.target.smbmount, ...)
Execute (.target.insmod, ...)
Execute (.target.modprobe, ...)

The Target agent implements the Execute() interface of SCR,
so a typical call for the target agent would be

SCR (`Execute (.target.bash, "command_to_run_in_bash", $[ "env":"value"] ));
SCR (`Execute (.target.symlink, "oldpath", "newpath" );
SCR (`Execute (.target.mkdir, "path" );	// default mode is 0755
SCR (`Execute (.target.control.printer_reset, "/dev/lp1");
SCR (`Execute (.target.bash, "command_to_run_in_bash", $[ "env":"value"] ));

Further interfaces are Read() and Write() which are used for file access.

(Replaces WFM Read/Write)
Read(.target.file, ...)
Write(.target.file, ...)

(Replaces WFM ReadY2/WriteY2)
Read(.target.ycpfile, ...)
Write(.target.ycpfile, ...)

Read(.target.root)
Read(.target.tmpdir)
Read(.target.string, ...)
Read(.target.dir, ...)
Read(.target.size, ...)

Details
-------
return from .target.bash_output:
      $[ "exit"   : return_value,            // integer
         "stdout" : "stdout_from_command",   // string
         "stderr" : "stderr_from_command"    // string
      ]

Logging
-------
The logging is controled by Y2DEBUG environment variable.
Set it to "1" and all messages will go to the y2log file.

You can also control the logging with ~/.yast2./logcontrol.ycp
file. Use "bash" as component name.
