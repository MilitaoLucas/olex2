CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
PROJECT(olex2)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXWIDGETS__ -D_UNICODE")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")

# Example to get it working with Anaconda or Miniforge. Should work similarly with other environments:
# -DPYTHON_LIBRARY=<conda_home>/envs/<python3.8 environment>/lib/libpython3.8.so.1.0
# -DPYTHON_INCLUDE_DIR=<conda_home>/envs/<python3.8 environment>/include/python3.8
# -DPYTHON_ROOT_DIR=<conda_home>/envs/<python3.8 environment>
# -DPython3_FIND_STRATEGY=LOCATION

# finding the python version
execute_process(
    COMMAND "${Python_EXECUTABLE}" -V
    OUTPUT_VARIABLE PYTHON_V_OUTPUT
    ERROR_VARIABLE PYTHON_V_ERROR
    RESULT_VARIABLE PYTHON_V_RESULT
)

if (PYTHON_V_RESULT EQUAL 0)
    # Python 2 often prints to stdout, Python 3 often to stderr
    if (PYTHON_V_OUTPUT)
        set(RAW_VERSION_STRING "${PYTHON_V_OUTPUT}")
    else()
        set(RAW_VERSION_STRING "${PYTHON_V_ERROR}")
    endif()

    string(REGEX MATCH "([0-9]+.[0-9]+)" Python_VERSION "${RAW_VERSION_STRING}")
    message(STATUS "Python version: ${Python_VERSION}")
else()
    message(WARNING "Failed to get Python version using '${PYTHON_EXECUTABLE} -V'. Result: ${PYTHON_V_RESULT}")
    message(WARNING "Output: ${PYTHON_V_OUTPUT}")
    message(WARNING "Error: ${PYTHON_V_ERROR}")
endif()
# end finding the python version

if (DEFINED Python_EXECUTABLE)
  get_filename_component(Python_HOME ${Python_EXECUTABLE} DIRECTORY)
  get_filename_component(Python_HOME ${Python_HOME} DIRECTORY)
  set(Python_LIBRARY "${Python_HOME}/lib/libpython${Python_VERSION}.so")
endif ()
# Add user provided compile options
if (DEFINED $ENV{OLEX2_DIR})
  set(OLEX2_GUI $ENV{OLEX2_DIR})
endif ()
message(WARNING "test ${Python_VERSION}")
if ("${Python_VERSION}" VERSION_GREATER 3.11)
    set(_PYGT310 "_PYGT310")
endif ()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(COPY_GUI_FILES "Copy the GUI files to the build location for testing" OFF)
  option(EXTERNAL_PYTHON_BINDINGS "Builds some bindings to use with external Python (experimental)" OFF)
  set(OLEX2_GUI_DIRECTORY ${OLEX2_GUI} CACHE PATH "Directory to copy from if COPY_GUI_FILES is set to ON")
endif ()

if (OLEX2_GUI_DIRECTORY STREQUAL "" AND COPY_GUI_FILES)
  message(FATAL_ERROR "COPY_GUI_FILES is only supported with OLEX2_GUI_DIRECTORY CMAKE variable or OLEX2_GUI env variable.")
endif ()
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
if (CMAKE_VERSION VERSION_LESS 3.18)
  set(DEV_MODULE Development)
else()
  set(DEV_MODULE Development.Module)
endif()
set(Python_FIND_VIRTUALENV "ONLY")
set(Python_FIND_STRATEGY "LOCATION")
find_package(Python ${Python_VERSION} EXACT COMPONENTS ${DEV_MODULE} Interpreter REQUIRED)

message(STATUS "Python include: ${Python_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${Python_INCLUDE_DIRS})
message(STATUS "Python version: ${Python_VERSION}")
if (NOT DEFINED ${Python_LIBRARIES})
  set(Python_LIBRARIES ${Python_LIBRARY})
endif ()
message(STATUS "Python libraries: ${Python_LIBRARIES}")

add_subdirectory(3rdparty EXCLUDE_FROM_ALL)
# additional modules
find_package(PkgConfig REQUIRED)
pkg_check_modules(PANGOFT2 REQUIRED pangoft2)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor) # Often needed for client apps
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})


# END user provided compile options

SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB xlib_SRCS "${src}/xlib/*.cpp")
FILE(GLOB xlibabsorb_SRCS "${src}/xlib/absorpc/*.cpp")
FILE(GLOB xlibhenke_SRCS "${src}/xlib/henke/*.cpp")
FILE(GLOB xlibmacro_SRCS "${src}/xlib/macro/*.cpp")


FILE(GLOB unirun_SRCS "${src}/unirun/*.cpp")
FILE(GLOB olex2_SRCS "${src}/olex/*.cpp")
LIST(REMOVE_ITEM olex2_SRCS "${src}/olex/langdict.cpp")
LIST(REMOVE_ITEM olex2_SRCS "${src}/olex/ctrls.cpp")
FILE(GLOB olex2_nui_SRCS "${src}/olex/nui/*.cpp")
FILE(GLOB olex2_ctrls_SRCS "${src}/olex/ctrls/*.cpp")
SET(all_SRCS
    ${olex2_nui_SRCS}
    ${xlib_SRCS}
    ${xlibabsorb_SRCS}
    ${xlibmacro_SRCS}
    ${xlibhenke_SRCS}
    ${olex2_ctrls_SRCS}
    ${olex2_extras}
)


INCLUDE_DIRECTORIES("${src}/olex")
INCLUDE_DIRECTORIES("${src}/olex/ctrls")
INCLUDE_DIRECTORIES("${src}/olex/nui")
INCLUDE_DIRECTORIES("${src}/xlib")
INCLUDE_DIRECTORIES("${src}/xlib/absorpc")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_2}")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_1}")
INCLUDE_DIRECTORIES(
    ${PANGOFT2_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_CURSOR_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
)
add_subdirectory(sdl)
add_subdirectory(glib)

INCLUDE_DIRECTORIES("${sdl_INCLUDE}")
add_subdirectory(repository)
INCLUDE_DIRECTORIES("${glib_INCLUDE}")
add_subdirectory(olex)
add_subdirectory(gxlib)
ADD_EXECUTABLE(unirun ${unirun_SRCS} ${all_SRCS})
ADD_EXECUTABLE(olex2 ${olex2_SRCS} ${all_SRCS})

TARGET_LINK_LIBRARIES(unirun PRIVATE
    wx-static
    sdl_core_lib
    glib
    gxlib
    html_core
    repo_core_lib
    ${WX_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${PANGOFT2_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    EGL
    ${WAYLAND_EGL_LIBRARIES}
    GLU
    ${Python_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)


TARGET_LINK_LIBRARIES(olex2 PRIVATE
    olex_extras
    wx-static
    sdl_core_lib
    glib
    html_core
    repo_core_lib
    gxlib
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${PANGOFT2_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    EGL
    ${WAYLAND_EGL_LIBRARIES}
    GLU
    ${Python_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

#target_include_directories(olex2 PRIVATE
#    ${glib_INCLUDE}
#)
message(WARNING "Python is ${_PYGT310}")
target_compile_definitions(olex2 PRIVATE _PYTHON _OPENSSL _PYGT310)
target_compile_definitions(unirun PRIVATE _PYTHON _OPENSSL _PYGT310)

if("${EXTERNAL_PYTHON_BINDINGS}")
    add_subdirectory(nanobind)
    add_dependencies(_olexpy olex2)

    TARGET_LINK_LIBRARIES(_olexpy PRIVATE
        html_core
        olex_extras
        repo_core_lib
        sdl_core_lib
        wx-static
        ${OPENSSL_LIBRARIES}
    )
endif ()
if (COPY_GUI_FILES)
  add_custom_command(TARGET olex2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
      "${OLEX2_GUI_DIRECTORY}"
      "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying files to build directory"
  )
endif ()
