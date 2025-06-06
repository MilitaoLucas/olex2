SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB gxlib_SRCS "${src}/*.cpp")
FILE(GLOB gxlib_INCLUDE "${src}")
add_library(gxlib STATIC
    ${gxlib_SRCS}
)
target_link_libraries(gxlib PRIVATE
    wx-static
    glib
    repo_core_lib
    sdl_core_lib
)

target_include_directories(gxlib PUBLIC
    ${gxlib_INCLUDE}
)
#set(glib_INCLUDE ${glib_INCLUDE} PARENT_SCOPE)
set_target_properties(gxlib PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
target_compile_definitions(gxlib PRIVATE _PYTHON)
