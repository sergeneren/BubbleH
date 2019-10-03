# Bubble<sup>H</sup>

Bubble<sup>H</sup> is an implementation of siggraph paper [Double Bubbles Sans Toil and Trouble: Discrete Circulation-Preserving Vortex Sheets for Soap Films and Foams](http://www.cs.columbia.edu/cg/doublebubbles/) in Sidefx Houdini. It solves soap film surfaces in a SOP node by a conversion between Houdini geometry and LosTopos mesh. 
 

### Release Notes

v1.0 Initial Public Release

## Installation

Either download the source as a zip file or right click to a desired location and use the command below with git bash
```
git clone https://github.com/sergeneren/BubbleH
```

### Required Libraries 

The following libraries are required for Bubble<sup>H</sup> to be compiled. You can install them easily using vcpkg install command. 

- Eigen (http://eigen.tuxfamily.org)
- OpenGL and GLUT (http://www.opengl.org/resources/libraries/glut/)
- CLAPACK (http://icl.cs.utk.edu/lapack-for-windows/clapack/)
- BLAS (http://www.netlib.org/blas/)
- zLib (https://www.zlib.net/)

### Compilation
This repository is only tested under Windows 10 and Visual Studio 2017. Compilation for Linux or MAC OS requires testing.    

Bubble<sup>H</sup> expects [vcpkg](https://github.com/Microsoft/vcpkg), Visual Studio 2017 and CMake to be installed. For compilateion use CMake gui to create a build directory and use x64 platform. vcpkg toolchain file must be specified during configuration. If you see a missing library, first install it using vcpkg and configure again. 

The houdini directory is hard coded for Houdini 17.5.327. You can change the "houdini dir" variable in cmake to find houdini cmake file. 

After configuring, generate the visual studio solution. Build the solution with release configuration. Houdini cmake will assure the resulting dll's are installed under your %HOME%/%HOUDINI_VERSION%/dso folder. You can open houdini and start using SOP node "Soap Film". An icon for the node will also be placed automatically under %HOME%/%HOUDINI_VERSION%/config/Icons  

Please take a look at the provided .hiplc files under "scenes" folder for examples. 


## Author

* **Sergen Eren** - [My website](https://sergeneren.com)

## License
This project is licensed under Clear BSD License 
