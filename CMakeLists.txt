cmake_minimum_required(VERSION 2.6)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(CMAKE_MODULE_PATH "C:/Program Files/CMake/share/cmake-3.20/Modules")

project(edPlugin)
find_package(Maya REQUIRED)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../output/Maya${MAYA_VERSION})

set(CMAKE_ARCJOVE)

#set(OPENGL_INCLUDE_DIR "C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um")
#set(OPENGL_INCLUDE_DIR "C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um/GL/")

set(OPENGL_VERBOSE TRUE)

find_package(OpenGL REQUIRED)

set(GLEW_VERBOSE TRUE)
#set(GLEW_USE_STATIC_LIBS TRUE)

#set(GLEW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
#find_package(GLEW REQUIRED)

message(WARNING ${OPENGL_INCLUDE_DIR})
message(WARNING ${GLEW_INCLUDE_DIR})
message(WARNING ${OpenGL_FOUND})
message(WARNING ${GLEW_SHARED_LIBRARY_RELEASE})
message(WARNING ${GLEW_ROOT})
message(WARNING $ENV{GLEW_ROOT})

#set(GLEW_INCLUDE_DIRS "C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um")
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
link_libraries(${GLEW_LIBRARIES})

#option(LIBIGL_USE_STATIC_LIBRARY     "Use libIGL as static librarie" ON)
#find_package(LIBIGL REQUIRED QUIET)


file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h" "src/lib/*.cpp" "src/lib/*.h" 
	"src/deformer/*.h" "src/deformer/*.cpp" "src/*/*.cpp" "src/*/*.h")
#file(GLOB SOURCE_FILES "src/plugin.cpp")
#


 
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include/eigen-3.3.7")
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include/libigl-master/include")
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include/glm-master")
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include")
include_directories(${MAYA_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

link_directories(${MAYA_LIBRARY_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES} ${OPENGL_LIBRARY})

MAYA_PLUGIN(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
