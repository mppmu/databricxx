DatABriCxx Examples
===================

Random Spectrum Generation and ROOT I/O
---------------------------------------

This is a typical particle/nuclear physics example. It simulates random events
from a multi-channel analyser (MCA) for a bogus spectrum (landau background
plus a gaussian peak), and writes them to a ROOT TTree in a TFile
`out-mc.root`, also including the MCA spectrum as a `TH1D`. It then reads the
tree, calibrates the events using the custom linear calibration bric
`CalibBricExample` and writes the calibrated values and the calibrated
spectrum to a new file `out-ana.root`.

The examples consists of the DatABriCxx configuration file
[mca-calib-example.json](mca-calib-example.json), and the ROOT script
[CalibBricExample.C](CalibBricExample.C) (containing the implementation of the
custom class `CalibBricExample`).

Run the example like this:

    # dbrx run mca-calib-example.json

To run the example with debugging output and with the HTTP server (on port
8080) enabled, and to keep the program running (so you can browse to
[http://localhost:8080/](http://localhost:8080/)), run

    # dbrx run -l debug -k -w -p 8080 mca-calib-example.json


Parameter Groups
----------------

This example shows how to nest parameters in groups using the class
`ParameterGroup`. It also show how to write parameter values to JSON filesand
how read them again. The example consists of the ROOT script
[param_group_example.C](param_group_example.C).

Run the example like this:

    # root param_group_example.C
