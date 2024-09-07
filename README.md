# Multicellular Simulator
A personal playground to test various models of multicellular developmental dynamics.

## Demonstration
[![Video demonstration of the simulator](https://img.youtube.com/vi/jIlXnYCDbYs/0.jpg)](https://www.youtube.com/watch?v=jIlXnYCDbYs)

Click the image above to see a simple video demonstration of dividing, growing, and dying cells confined to an invisible 3D forcefield box. This simulation shows a multicellular blob of up to 1 000 000 cells, running in real-time on a consumer-grade graphics card (NVidia GTX 1070). More complex simulations can be made by defining more complex CRNs.

## Description
This was a solo hobby project that I worked on in 2018, but which I haven't updated since.

This system can simulate multicellular systems in a particle-based manner, including various intercellular forces, chemical reaction networks (CRNs), cell division driven by CRN signals, cell death (including apoptosis driven by CRN signals), external force fields, and cell polarity. The simulation, chemical reaction networks, force fields, and other cell-based settings can be easily customized by editing the `test_lua_code.lua` file, without requiring recompilation.

The software runs most of its simulation code on an available OpenCL-capable graphics card, currently tested with an NVidia GTX 1070, for high performance simulations. The software dynamically compiles code for the graphics card based on the simulation setup.







## How to compile
Make sure that all dependencies are installed. This includes:
- epoxy/gl.h              (For Debian-based systems: `sudo apt-get install libepoxy-dev`)
- gtkmm                   (For Debian-based systems: `sudo apt-get install libgtkmm-3.0-dev`)
- glm                     (For Debian-based systems: `sudo apt-get install libglm-dev`)
- CL/cl.hpp or CL/cl2.hpp (For Debian-based systems: `sudo apt-get install opencl-headers`)
- libOpenCL.so            (For Debian-based systems: `sudo apt-get install ocl-icd-opencl-dev`)
- lGLEW                   (For Debian-based systems: `sudo apt-get install libglew-dev`)
- LuaJIT 5.1

- pybind11/pybind11.h		  (`usr/bin/python3 -m pip install pybind11` (requires pip))


Once these are all installed, simply run `make` to compile the project. Use `make clean` if previous builds cause interference.
