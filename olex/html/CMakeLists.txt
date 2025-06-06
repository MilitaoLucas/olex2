SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB html_SRCS "${src}/*.cpp")
set(html_INCLUDE "${src}")

add_library(html_core STATIC
    ${html_SRCS}
)

target_link_libraries(html_core PRIVATE
    sdl_core_lib
    repo_core_lib
    gxlib
    wx-static
)

target_include_directories(html_core PUBLIC
    ${html_INCLUDE}
)
#set(glib_INCLUDE ${glib_INCLUDE} PARENT_SCOPE)
set_target_properties(html_core PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
