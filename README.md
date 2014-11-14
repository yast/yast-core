# YaST Core

Travis:  [![Build Status](https://travis-ci.org/yast/yast-core.svg?branch=master)](https://travis-ci.org/yast/yast-core)
Jenkins: [![Jenkins Build](http://img.shields.io/jenkins/s/https/ci.opensuse.org/yast-core-master.svg)](https://ci.opensuse.org/view/Yast/job/yast-core-master/)

The core part of [YaST](http://yast.github.io) provides component system, agent infrastructure and for backward compatibility implementation of YCP language.

## Agents Infrastructure
Agents are used to modify the underlaying system. Their most important feature is that it can be switched to different target which is heavily used during installation.
Agents for target system are managed by SCR instance. Each agent is attached to its unique path. The path is special YaST data type(TODO link). An agent itself is defined
by its scrconf file where is specified if some base together with its parameters that should be used or path to a binary if the agent is implemented by script.
Agents that is implemented in script communicate via YCP protocol and have predefined methods. Agents using base just configure such base. The base are
library that is registered also to SCR, but without path. It is usually parser, that allows easier reading and writing files. For example of such library see [system agent documentation](doc/systemagent).

### Hints
- Agents can be registered during runtime. See [SCR#RegisterAgent](http://www.rubydoc.info/github/yast/yast-ruby-bindings/Yast/SCR#RegisterAgent-class_method)
- Ruby-bindings provide shortcut for constructing paths. See [Yast.path](http://www.rubydoc.info/github/yast/yast-ruby-bindings/Yast#path-instance_method)

## Component System
The core provides component system that allows language agnostic communication between various parts of YaST. Code written in ruby and perl use the respective ruby and perl bindings to attach to such system.
There is also component written directly in C++ like libyui and package bindings.

## YCP implementation
Beside providing backward compatibility YCP is still needed as communication protocol in component system and for agents. It is also used in agents scrconf file to
define agent and its path.

## Further Information

More information about YaST can be found on its [homepage](http://yast.github.io).
