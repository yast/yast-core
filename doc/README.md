# YaST Core

The core part of [YaST](http://yast.github.io) providing the components system,
the agents infrastructure and the implementation of the YCP language for
backward compatibility.

## Agents Infrastructure

The YaST agents are used to modify the underlying system. Their most important
feature is that they can be switched to a different target system, which is
heavily used during installation.

The agents for the target system are managed by a SCR instance. Each agent is
attached to its unique YaST path. Read more about SCR, YaST paths and their
relationship with the agents in the "YaST architecture" document at the
[YaST documentation page](http://yast.github.io/documentation.html).

Each agent is defined by its scrconf file which can contain either a base 
with some parameters either a path to a binary if the agent
is implemented by a script.

Agents that are implemented in scripts communicate via the YCP protocol and
have predefined methods. Agents using a base just configure such base. The
bases are libraries that are registered to SCR without a path and that usually
provide parsing funcionalities for different file types.

### Relevant agents and bases

- [System agent](systemagent.md)

### Hints

- The agents can be registered during runtime. See
[SCR#RegisterAgent](http://www.rubydoc.info/github/yast/yast-ruby-bindings/Yast/SCR#RegisterAgent-class_method)
- Ruby-bindings provide shortcut for constructing paths. See
[Yast.path](http://www.rubydoc.info/github/yast/yast-ruby-bindings/Yast#path-instance_method)

## Component System

The core provides a components system that allows language agnostic communication
between various parts of YaST. Code written in ruby and perl uses the respective
ruby and perl bindings to attach to such system.
There are also components written directly in C++ like libyui and the package
bindings.

## YCP implementation

Beside providing backward compatibility, YCP is still needed as a communication
protocol within the components system and with the agents. It is also used in
agents scrconf file to define agent and its path.

## Further Information

More information about YaST can be found on its [homepage](http://yast.github.io).
