DatABriCxx Examples
===================

Random Spectrum Generation and ROOT I/O
---------------------------------------

This is a typical particle/nuclear physics example. It generates random events
for a bogus spectrum (landau background plus a gaussian peak), and writes them
to a ROOT TTree in a TFile `out-mc.root`. It then read the tree, produces a
histogram and writes it to a new file `out-ana.root`.

The examples simply consists of a DatABriCxx JSON configuration file
[rndgen-ttree-io-hist.json](rndgen-ttree-io-hist.json), configuring the
brics involved in the data flow.

Run the example like this:

    # dbrx run rndgen-ttree-io-hist.json

To run the example with debugging output and with the HTTP server (on port
8080) enabled, and to keep the program running (so you can browse to
[http://localhost:8080/](http://localhost:8080/)), run

    # dbrx run -l debug -k -w -p 8080 rndgen-ttree-io-hist.json


Parameter Groups
----------------

This example shows how to nest parameters in groups using the class
`ParameterGroup`. It also show how to write parameter values to JSON filesand
how read them again. The example consists of the ROOT script
[param_group_example.C](param_group_example.C).

Run the example like this:

    # root param_group_example.C
