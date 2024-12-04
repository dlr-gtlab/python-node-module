# GtPythonGraph Module

 Module to combine the abilities of python and the intelli graph system.
 The nodes can be used flexibly to script processes in individual process steps. 
 Data forwarding is possible for a large number of data classes in the system. 
 A matplotlib interface is also integrated, which makes it possible to display plots based on the scripts directly within the graph in the node.
  
    <figure class="image">
        <img src="/images/Example.png" alt="Example Workflow">
        <figcaption> <i>Fig. 1: Example workflow using python nodes</i></figcaption>
    </figure>
  
 ## License

The largest portion of the code is licensed under the `Apache 2.0 License`.

Smaller thirdparty party code part of code base uses other permissive licenses, such as the
`BSD` and `MIT` licenses. Please review the directory [LICENSES](https://github.com/dlr-gtlab/python-node-module/tree/master/LICENSES) and [.reuse](https://github.com/dlr-gtlab/python-module/tree/master/.reuse)
for a full overview on all licensed used.


## Compiling from Source

### Prerequisites

A working GTlab installation is required. This includes GTlab and the GTlab Logging library.
In addition the python module and the intelligraph-module of the GTlab eco-system are 
required including thier dependencies.

| Library              |  Version  | Bundled | Where to get                                     |
| -------------------- | --------- | ------- | -------------------------------------------------|
| Qt                   |  5.15.x   | No      | https://download.qt.io/official_releases/qt/     |
| GTlab Core + Logging |  >= 2.0   | No      | https://github.com/dlr-gtlab/gtlab-core          |
| Python               |  3.9.x    | No      | https://www.python.org/downloads/                |
| PythonQt             | >= 3.5.0  | Yes     | https://github.com/MeVisLab/pythonqt             |
| intelligraph-module  | >= 0.12.0 | No      | https://github.com/dlr-gtlab/intelligraph-module |
| python-module        | >= v1.6.1 | No      | https://github.com/dlr-gtlab/python-module       |

### Building

The python node module requires a recent `CMake` (>3.15) to build. The configuration and build process is
similar to other CMake builds:

```
cmake -S . -B build_dir -DQt5_DIR=<path/to/cmake/Qt5> -DCMAKE_PREFIX_PATH=<path/to/gtlab_install> -DCMAKE_INSTALL_PREFIX=<path/to/gtlab_install>
cmake --build build_dir
cmake --build build_dir --target install
```

