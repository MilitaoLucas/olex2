CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
PROJECT(olex2)
#-DPython3_LIBRARY=/home/lucas/miniforge3/envs/olex2/lib/libpython3.so -DPython3_INCLUDE_DIR=/home/lucas/miniforge3/envs/olex2/include/python3.8
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXWIDGETS__ -D_UNICODE")

#SET(wxWidgets_USE_UNICODE ON)
#SET(wxWidgets_CONFIG_OPTIONS --version=3.0)
#FIND_PACKAGE(wxWidgets 3.0.5 EXACT REQUIRED COMPONENTS base core gl html net aui adv)
#message(STATUS "WX lib: ${wxWidgets_LIB_DIR}")
#SET(wxWidgets_CONFIG_EXECUTABLE "/usr/local/bin/wx-config")
#INCLUDE(${wxWidgets_USE_FILE})

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")
add_subdirectory(3rdparty EXCLUDE_FROM_ALL)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
FIND_PACKAGE(PythonLibs 3.8 COMPONENTS Development REQUIRED)
message(STATUS "Python include: ${PYTHON_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
message(STATUS "Python version: ${PYTHONLIBS_VERSION_STRING}")
message(STATUS "Python libraries: ${PYTHON_LIBRARIES}")

#find_package(JPEG REQUIRED)
#include_directories(${JPEG_INCLUDE_DIR})
#
## Find PNG
#find_package(PNG REQUIRED)
#include_directories(${PNG_INCLUDE_DIRS})
#
#FIND_PACKAGE(Fontconfig REQUIRED)
#INCLUDE_DIRECTORIES(${Fontconfig_INCLUDE_DIRS})
#
## Find OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
#
## Find ZLIB (required by PNG and possibly wxWidgets)
#find_package(ZLIB REQUIRED)
#
## Find Expat (possibly required by wxWidgets)
#find_package(EXPAT REQUIRED)


SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB alglib_SRCS "${src}/alglib/*.cpp")
FILE(GLOB sdl_SRCS "${src}/sdl/*.cpp")
FILE(GLOB sdlsmart_SRCS "${src}/sdl/smart/*.cpp")
FILE(GLOB sdlexp_SRCS "${src}/sdl/exparse/*.cpp")
FILE(GLOB esdl_math_SRCS "${src}/sdl/math/*.cpp")
FILE(GLOB xlib_SRCS "${src}/xlib/*.cpp")
FILE(GLOB xlibabsorb_SRCS "${src}/xlib/absorpc/*.cpp")
FILE(GLOB xlibhenke_SRCS "${src}/xlib/henke/*.cpp")
FILE(GLOB xlibmacro_SRCS "${src}/xlib/macro/*.cpp")
FILE(GLOB glib_SRCS "${src}/glib/*.cpp")
FILE(GLOB gxlib_SRCS "${src}/gxlib/*.cpp")

FILE(GLOB repo_SRCS "${src}/repository/*.cpp")

IF (NOT WIN32)
        LIST(REMOVE_ITEM repo_SRCS "${src}/repository/fsindex.cpp")
        LIST(REMOVE_ITEM repo_SRCS "${src}/repository/unzip.cpp")
        LIST(REMOVE_ITEM repo_SRCS "${src}/repository/zip.cpp")
endif (NOT WIN32)

FILE(GLOB unirun_SRCS "${src}/unirun/*.cpp")
FILE(GLOB olex2_SRCS "${src}/olex/*.cpp")
FILE(GLOB olex2_nui_SRCS "${src}/olex/nui/*.cpp")
FILE(GLOB olex2_html_SRCS "${src}/olex/html/*.cpp")
FILE(GLOB olex2_ctrls_SRCS "${src}/olex/ctrls/*.cpp")
SET(olex2_extras "${src}/olex/langdict.cpp" "${src}/olex/ctrls.cpp")
SET(all_SRCS
        ${alglib_SRCS}
        ${sdl_SRCS}
        ${sdlsmart_SRCS}
        ${sdlexp_SRCS}
        ${esdl_math_SRCS}
        ${olex2_nui_SRCS}
        ${xlib_SRCS}
        ${xlibabsorb_SRCS}
        ${xlibmacro_SRCS}
        ${xlibhenke_SRCS}
        ${glib_SRCS}
        ${gxlib_SRCS}
        ${repo_SRCS}
        ${olex2_html_SRCS}
        ${olex2_ctrls_SRCS}
        ${olex2_extras}
)


INCLUDE_DIRECTORIES("${src}/alglib")
INCLUDE_DIRECTORIES("${src}/glib")
INCLUDE_DIRECTORIES("${src}/gxlib")
INCLUDE_DIRECTORIES("${src}/olex")
INCLUDE_DIRECTORIES("${src}/olex/html")
INCLUDE_DIRECTORIES("${src}/olex/ctrls")
INCLUDE_DIRECTORIES("${src}/olex/nui")
INCLUDE_DIRECTORIES("${src}/repository")
INCLUDE_DIRECTORIES("${src}/sdl")
INCLUDE_DIRECTORIES("${src}/xlib")
INCLUDE_DIRECTORIES("${src}/xlib/absorpc")
INCLUDE_DIRECTORIES("${src}/sdl/math")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_2}")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_1}")
ADD_EXECUTABLE(unirun ${unirun_SRCS} ${all_SRCS})
ADD_EXECUTABLE(olex2 ${olex2_SRCS} ${all_SRCS} )
message(WARNING "SYSTEM LIBS: ${WX_LIBRARIES}")

TARGET_LINK_LIBRARIES(unirun PRIVATE # Apply similar changes to the unirun target if needed
    wx-static
    ${WX_LIBRARIES}
#    wxWidgets_LIB_DIR
    # Add others if unirun uses wx components requiring them
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
#    ${JPEG_LIBRARIES}
    ${EXPAT_LIBRARIES}
    GLU
    ${PYTHON_LIBRARIES}

    ${OPENSSL_LIBRARIES}
)



TARGET_LINK_LIBRARIES(olex2 PRIVATE
    wx-static
    ${WX_LIBRARIES}
    # Explicitly link wxWidgets internal dependency libraries
    # Ensure these files exist in WXWIDGETS_LIB_DIR. Names might differ slightly
    # (e.g., libwxpngd-3.0.a for debug builds).
#    "${WXWIDGETS_LIB_DIR}/libwxpng-3.0.a"
#    "${WXWIDGETS_LIB_DIR}/libwxzlib-3.0.a"
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
#    ${JPEG_LIBRARIES} # System JPEG
    ${EXPAT_LIBRARIES}# System Expat
    GLU
    ${PYTHON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)


target_compile_definitions(olex2 PRIVATE _PYTHON)
target_compile_definitions(unirun PRIVATE _PYTHON)
add_custom_command(TARGET olex2 POST_BUILD
    COMMAND rsync -rv "/home/lucas/olex2dep/"
            "${CMAKE_CURRENT_BINARY_DIR}/" > /dev/null
    COMMENT "Syncing files to build directory"
)