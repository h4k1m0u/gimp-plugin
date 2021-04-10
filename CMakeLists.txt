cmake_minimum_required(VERSION 3.10)
project(Gimp-Plugin)

# enable auto-complete & debug with gdb & memory-leaks detection
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

# required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIMP REQUIRED gimp-2.0)

# executable
add_executable(box_blur
  src/fill.c
  src/box_blur.c
  src/blur.c)
target_include_directories(box_blur PUBLIC ${GIMP_INCLUDE_DIRS} include)
target_link_libraries(box_blur ${GIMP_LIBRARIES})
