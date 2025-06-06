SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB glib_SRCS "${src}/*.cpp")
FILE(GLOB glib_INCLUDE "${src}")
add_library(glib STATIC
    ${glib_SRCS}
)
target_include_directories(glib PRIVATE sdl_INCLUDE)
target_link_libraries(glib PRIVATE
    wx-static
    sdl_core_lib
)


set(glib_INCLUDE ${glib_INCLUDE} PARENT_SCOPE)
set_target_properties(glib PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
