DatABriCxx - Modular data analysis with C++11 and CERN ROOT
===========================================================

DatABriCxx is a modular C++11 data analysis framework. It uses so-called
"brics" as building blocks.

Note: This project is still under heavy development. The API is subject to
change at any time, documentation (as yet) mostly non-existent. Use at your
own risk! Proper conceptual and API documentation are most definitely on the
to do list - however, features and bug-fixes required by the initial users
currently have take priority.

DatABriCxx uses the [CERN ROOT framework](http://root.cern.ch/) internally
for class loading, reflection, and related tasks. However DatABriCxx is
written I/O- and task-agnostic. However, while support for ROOT files,
histograms and similar is included, you are free to use other I/O formats and
not to use any ROOT classes in your DatABriCxx applications.


Installation
------------

Dependencies: DatABriCxx requires

* A C++11 compatible C++ compiler (GCC >= v4.8 or Clang >= v3.4 will do)

* The [CERN ROOT framework](http://root.cern.ch/) >= v6.0.0. ROOT
  must be built with the `--enable-http` option. The program `root-config`
  must be on your `$PATH`.

DatABriCxx uses the GNU Autotools/Automake for build and installation, so just
use the usual

    ./autogen.sh && ./configure && ./make install

to build and install. DatABriCxx supports parallel compilation, you may want
to use `make -j8` or so to speed things up.

To install DatABriCxx in your home directory (instead of `/usr/local`), e.g.
in `$HOME/my/sw/dir`, set the installation prefix as desired and modify your
paths:

    ./autogen.sh
    ./configure --prefix="$HOME/my/sw/dir"
    export PATH="$HOME/my/sw/dir/bin:${PATH}"
    export LD_LIBRARY_PATH="$HOME/my/sw/dir/lib:$LD_LIBRARY_PATH"
    make -j8 install

On OS-X, use `$DYLD_LIBRARY_PATH` instead of `$LD_LIBRARY_PATH`.


Examples
--------

To try DatABriCxx out, have a look at the [examples](doc/examples/examples.md).
