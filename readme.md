A C++ implementation of *Fast Simulation of Mass-Spring Systems* [1], rendered with OpenGL.
The dynamic inverse procedure described in [2] was implemented to constrain spring deformations
and prevent the "super-elastic" effect when using large time-steps.

### Dependencies

* **OpenGL, freeGLUT, GLEW, GLM** for rendering.
* **OpenMesh** for computing normals.
* **Eigen** for sparse matrix algebra.

### Building

``` bash
mkdir build
cd build
cmake ..
cmake --build .
```

On Windows, you will likely need to specify the directories containing GLUT and GLEW in CMAKE_PREFIX_PATH so that cmake can find them.

``` bash
cmake .. -DCMAKE_PERFIX_PATH:PATH=/path/to/libs
```

You will also need to copy the DLLs to the build directory if they are not available globally.

### Demonstration

![curtain_hang](https://user-images.githubusercontent.com/24758349/79005907-97ad1100-7b60-11ea-9e27-90375461beaf.gif)

![curtain_ball](https://user-images.githubusercontent.com/24758349/79005924-9d0a5b80-7b60-11ea-8ce4-d9fc683441d7.gif)

### References

[1] Liu, T., Bargteil, A. W., Obrien, J. F., & Kavan, L. (2013). Fast simulation of mass-spring systems. *ACM Transactions on Graphics,32*(6), 1-7. doi:10.1145/2508363.2508406

[2] Provot, X. (1995). Deformation constraints in a mass-spring modelto describe rigid cloth behavior. *InGraphics Interface* 1995,147-154.
