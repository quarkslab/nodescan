nodescan
========

Nodescan is an asynchronous scanning library. It allows the building of custom
asynchronous scanners, from the target specification to the level 7 processing.

For now, it only supports one asynchronous engine with UNIX sockets.


Installation
============

These libraries needs to be installed:

* libleeloo: refer to https://github.com/quarkslab/libleeloo
* boost-python
* boost-random
* boost-log

Under debian based system, you can do:

  # apt-get install boost-{python,random,log}-dev


To build it, follow these instructions:

 $ cd /path/to/source
 $ mkdir build
 $ cd build
 $ cmake -DCMAKE_BUILD_TYPE=release ..
 $ make
 $ sudo make install


Usage
=====

There are usage C++ examples in the "tests" directory. Python bindings examples
can be found in bindings/python/examples.
