cmake_minimum_required(VERSION 3.10)
project(Gimp-Plugin)

# enable auto-complete & debug with gdb & memory-leaks detection
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

# copy assets folder
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIMP REQUIRED gimp-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK REQUIRED gtk+-2.0)

# main executable
add_executable(hello_gtk src/hello_gtk.c)
target_include_directories(hello_gtk PUBLIC ${GTK_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GIMP_INCLUDE_DIRS} include)
target_link_libraries(hello_gtk ${GTK_LIBRARIES} ${GIO_LIBRARIES} ${GLIB_LIBRARIES} ${GIMP_LIBRARIES})
