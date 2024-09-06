# Multicellular Simulator
A personal playground to test various models of multicellular developmental dynamics.

The simulation, chemical reaction networks, force fields, and other cell-based settings can be customized easily by editing the `test_lua_code.lua` file, no recompilation necessary.

## Demonstration
Click the image below to see a simple video demonstration of dividing, growing, and dying cells confined to an invisible 3D forcefield box. This simulation shows a multicellular blob of up to 1 000 000 cells, running on a consumer-grade graphics card (NVidia GTX 1070).

[![Video demonstration of the simulator](https://img.youtube.com/vi/jIlXnYCDbYs/0.jpg)](https://www.youtube.com/watch?v=jIlXnYCDbYs)



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
