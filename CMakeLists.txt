cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

project(edPlugin)
find_package(Maya REQUIRED)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../output/Maya${MAYA_VERSION})

set(CMAKE_ARCJOVE)




file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h" "src/lib/*.cpp" "src/lib/*.h" 
	"src/deformer/*.h" "src/deformer/*.cpp")

 
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include/eigen-3.3.7")
include_directories("F:/all_projects_desktop/common/edCode/edPlugin/include/libigl-master/include")
include_directories(${MAYA_INCLUDE_DIR})

link_directories(${MAYA_LIBRARY_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES} ${OPENGL_LIBRARY})

MAYA_PLUGIN(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
