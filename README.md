Biped Footstep Planner
======================

Author:   Keliang He <khe1@swarthmore.edu>

GUI code: Matt Zucker <mzucker1@swarthmore.edu>

Building
========

This project requires cmake, libpng, and GLUT.  To build:

    mkdir build
    cd build
    ln -s ../maps maps
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make

Running bipedPlan
=================

The `bipedPlan` executable is a standalone command-line
application. Example queries are in file query.txt. For instance:

    ./bipedPlan ./maps/circles.png 8 24 1.57 120 110 3 1000 40 1 1

The output file `DebugImage.svg` will show the planned steps,
`Heuristic.svg` will show the heuristic used.

