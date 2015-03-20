DatABriCxx Examples
===================

Random MCA Events Calibration with ROOT I/O
-------------------------------------------

This is a typical particle/nuclear physics example. It simulates random events
from a multi-channel analyser (MCA) for a bogus spectrum (landau background
plus a Gaussian peak), and writes them to a ROOT TTree in a TFile
`out-mc.root`, also including the MCA spectrum as a `TH1D`. It then reads the
tree, calibrates the events using the custom linear calibration bric
`LinCalibBric` and writes the calibrated values and the calibrated
spectrum to a new file `out-ana.root`.

The examples consists of the DatABriCxx configuration file
[mca-calib.json](mca-calib.json), and the ROOT script
[LinCalibBric.C](LinCalibBric.C) (containing the implementation of the
custom class `LinCalibBric`).

Run the example like this:

    # dbrx run mca-calib.json

To run the example with debugging output and with the HTTP server (on port
8080) enabled, and to keep the program running (so you can browse to
[http://localhost:8080/](http://localhost:8080/)), run

    # dbrx run -l debug -k -w -p 8080 mca-calib.json

Note that you can also run the example from within a different directory:

    # dbrx run doc/examples/mca-calib.json

as [mca-calib.json](mca-calib.json) uses

    "requires": [ "$_/LinCalibBric.C" ]

to refer to [LinCalibBric.C](LinCalibBric.C). The special variable `$_` is
automatically substituted with the path to the configuration file,
allowing to specify file names relative it's location.

The example also include an additional configuration file
[mca-calib-vars.json](mca-calib-vars.json), that overrides the fixed
calibration values in [mca-calib.json](mca-calib.json) with variables. Run

    # offset=-19.157 dbrx run -Vslope=0.10403 mca-calib.json mca-calib-vars.json

to see the effect of cascading multiple configuration files and variable
substitution. Note that you can supply variable values both as environment
variables (used above to set `offset`) or via the `dbrx -V` option (used
above to set `slope`).

You can also print the effective configuration after cascading and with
variable substitution using

    # offset=-19.157 dbrx get-config -Vslope=0.10403 mca-calib.json mca-calib-vars.json

and also without variable substitution using

    # dbrx get-config -s mca-calib.json mca-calib-vars.json


Parameter Groups
----------------

This example shows how to nest parameters in groups using the class
`ParameterGroup`. It also show how to write parameter values to JSON files and
how read them again. The example consists of the ROOT script
[param_group_example.C](param_group_example.C).

Run the example like this:

    # root param_groups.C
