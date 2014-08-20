# Target Agent

## General

The target agent is the SCR interface to (shell) commands
of the target system.

## Paths

Target agent is attached to `.target` path. See all available `.target` commands below.
Target agent can be also invoked via WFM where it lives under `.local` root.
Difference between WFM `.local` and SCR `.target` is only after SCR switch,
which is used for example in installation, when `.local` always work always on
root `/` and `.target` work on SCR target.

## Commands For Execute

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

Example in ruby that creates temporary file and raises exception in case of failure.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.bash_output"), "mktemp")
    if result["exit"] != 0
      raise "Failed to create temporary file with #{result["stderr"]}"
    end
    file_path = result["stdout"]
```

Example in ruby passing ENV variable VERBOSE=1

```
  result = Yast::SCR.Execute(Yast::Path.new(".target.bash_output"), "mktemp", {"VERBOSE" => "1"})
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

Example in ruby that does time consuming job.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.bash_background"), "sleep 1000000")
    if result["exit"] != 0
      raise "I cannot sleep. Do not disturb me!"
    end
```

### `.symlink`
Creates symlink. _Returns_ boolean depending on success. _Arguments_ are two
strings, one with source path and second with target one.

Example in ruby create symlink `/tmp2` pointing to `/tmp`

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.symlink"), "/tmp", "/tmp2")
    raise "Creating symlink failed" unless result
```

### `.mkdir`

Creates directory and all its parents. _Returns_ boolean depending on success.
_Arguments_ are string with path and optional integer with mode. If mode is not
specified, 0755 is used.

Example in ruby create directory `/tmp/foo/bar` with mode 0700.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.mkdir"), "/tmp/foo/bar", 0700)
    raise "Creating directory failed" unless result
```

### `.remove`
Removes a file. _Returns_ boolean depending on success.
_Argument_ is string with path to file.
*note*: Cannot remove a directory

Example in ruby remove file `/tmp/foo`.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.remove"), "/tmp/foo")
    raise "Removing file failed" unless result
```

### `.mount`

Mounts a (block) device at a mountpoint.
_Arguments_ are array with device, mountpoint strings and optional third element
for logfile and optional options to pass to mount.
*note*: Deprecated use bash agent directly to run mount

The return value is true or false, depending of the success

Example in ruby how to mount floppy.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.mount"), ["/dev/floppy", "/floppy", "/var/log/y2mountlog"], "-t msdos")
    raise "Mounting floppy failed" unless result
```

### `.umount`

Unmounts a (block) device at a mountpoint.
_Argument_ is mountpoint
*note*: Deprecated use bash agent directly to run umount

The return value is true or false, depending of the success

Example in ruby how to umount floppy.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.umount"), "/floppy")
    raise "Unmounting floppy failed" unless result
```

### `.insmod`

Load module in target system.
_Arguments_ are module and options for it.
*note*: Deprecated use bash agent directly to run insmod

The return value is true or false, depending of the success

Example in ruby how to insert module.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.insmod"), "a_module", "an option")
    raise "Module insertion failed" unless result
```

### `.modprobe`

Load module in target system.
_Arguments_ are module and options for it.
*note*: Deprecated use bash agent directly to run modprobe

The return value is true or false, depending of the success

Example in ruby how to insert module.

```
    result = Yast::SCR.Execute(Yast::Path.new(".target.modprobe"), "a_module", "an option")
    raise "Module insertion failed" unless result
```

## Commands For Read/Write

### `.string`

Reads/writes file as a single string.
_Arguments for writing_ can have two types. The first is string filename and string value.
The second one is array with string filename and integer filemode and string content.
_Arguments for reading_ is only string filename.

The return value fore reading is content of file or nil in case of failure.
For writing it return true or false, depending of the success.

Example in ruby how to read file content.

```
    content = Yast::SCR.Read(Yast::Path.new(".target.string"), "/root/test")
    raise "Reading file failed" unless content
```

Example in ruby how to write file without specified mode.

```
    result = Yast::SCR.Write(Yast::Path.new(".target.string"), "/root/test", "test content")
    raise "Writing to file failed" unless result
```

Example in ruby how to write file with specified mode.

```
    result = Yast::SCR.Write(Yast::Path.new(".target.string"), ["/root/test", 0600], "test content")
    raise "Writing to file failed" unless result
```

### `.ycp` or `.yast2`

Reads/Writes programming data serialized as ycp to given file.
_Arguments for writing_ can have two types. The first is string filename and any value.
The second one is array with string filename and integer filemode and any value.
_Arguments for reading_ is string filename and optional default value if file not found.

Reading for `.yast2` have one speciality, that it search in y2path for data/_filename_.

The return value is data from file or nil if failed for reading and true or false, depending of
the success, for writing.

Example in ruby how to read structure from file.

```
    content = Yast::SCR.Read(Yast::Path.new(".target.ycp"), "/root/test.ycp")
    raise "Reading file failed" unless content
```

Example in ruby how to read structure from file relative to y2path with default value.

```
    content = Yast::SCR.Read(Yast::Path.new(".target.yast2"), "test.ycp", :missing)
```

Example in ruby how to write data.

```
    data = { :a => "test" }
    result = Yast::SCR.Write(Yast::Path.new(".target.yast2"), "/root/test.ycp", data)
    raise "Writing to file failed" unless result
```

### `.byte`
Reads/Writes bytes as byteblock from/to given file.
_Arguments for writing_ are filename and byteblock.
_Argument for reading_ is filename.

Example in ruby how to read/write byteblock.

```
    content = Yast::SCR.Read(Yast::Path.new(".target.byte"), "/root/test")
    raise "Reading byteblock failed" unless content
    res = Yast::SCR.Write(Yast::Path.new(".target.byte"), "/root/test2", content)
    raise "Writing byteblock failed" unless res
```

### `.passwd`
Write-only command to set or modify the encrypted password of already existing 
user in /etc/passwd and /etc/shadow.
_Argument_ is crypted password.
The return value is true or false, depending of the success

Example in ruby how to change root password.

```
    content = Yast::SCR.Write(Yast::Path.new(".target.passwd.root"), crypted_password)
    raise "Changing root password failed" unless content
```

### `.tmpdir`
Read-only command to return the (instance specific) directory for storing temporary
files. The directory (and its contents) will be removed by the SystemAgent
destructor (usually when yast2 exits)

Example in ruby how to read tmpdir.

```
  path = Yast::SCR.Read(Yast::Path.new(".target.tmpdir"))
```

### `.dir`
Read-only command to read a directory. Returns a list of strings, one string for
each file contained in the directory path is pointing to.
The entries '.' and '..' are NOT returned. Returns nil and
doesn't log an error, if path does not point to a readable directory.
If a default value is given, this is returned if path isn't accessible.

_Arguments_ are string path and optional default value.

Example in ruby how to get list of files in directory.

```
  files = Yast::SCR.Read(Yast::Path.new(".target.dir"), "/etc/sysconfig/network")
  raise "Reading directory content failed" unless files
```

### `.size`
Read-only command to read current size of file.
Returns -1 if the file does not exist

_Argument_ is string path to file.

Example in ruby how to get size of file.

```
  file_size = Yast::SCR.Read(Yast::Path.new(".target.size"), "/root/test")
```

### `.stat`
Read-only command to return a map with file information (see stat(2)). If
the file does not exist return an empty map.

_Argument_ is string path to file.

### `.lstat`
Read-only command to return a map with file information (see lstat(2)). If
the file does not exist return an empty map. Only difference to stat is that
lstat does not follow link(s) and returns info about link itself

_Argument_ is string path to file.

### `.symlink`
Read-only command to get the content of the symbolic link filename. If the filename
does not exist or is not symbolic link, nil is returned and an error logged.

_Argument_ is string path to symlink.
