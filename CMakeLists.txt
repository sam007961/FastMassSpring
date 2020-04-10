cmake_minimum_required(VERSION 3.10.0)

project(fast-mass-spring)

set(Sources
    ClothApp/app.cpp
    ClothApp/MassSpringSolver.cpp
    ClothApp/Mesh.cpp
    ClothApp/Renderer.cpp
    ClothApp/Shader.cpp
    ClothApp/UserInteraction.cpp
)

# find OpenGL, GLUT, GLEW
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})

# add Eigen, OpenMesh, glm
include(FetchContent)
FetchContent_Declare(
  openmesh
  GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git 
)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY  https://gitlab.com/libeigen/eigen.git
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY  https://github.com/g-truc/glm
)

FetchContent_GetProperties(openmesh)
if(NOT openmesh_POPULATED)
  FetchContent_Populate(openmesh)
  add_subdirectory(${openmesh_SOURCE_DIR} ${openmesh_BINARY_DIR})
endif()

FetchContent_GetProperties(eigen)
if(NOT eigen_POPULATED)
  FetchContent_Populate(eigen)
  add_subdirectory(${eigen_SOURCE_DIR} ${eigen_BINARY_DIR})
endif()


FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
  FetchContent_Populate(glm)
  add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

# needed for OpenMesh on Windows
if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
endif()

# copy shaders to binary directory
file(INSTALL ClothApp/shaders/ DESTINATION shaders/)

# create executable
add_executable(fast-mass-spring ${Sources})
target_link_libraries(fast-mass-spring ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} OpenMeshCore eigen glm)
