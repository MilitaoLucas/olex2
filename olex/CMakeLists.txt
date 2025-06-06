SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
SET(olex_extras_SRCS "${src}/langdict.cpp" "${src}/ctrls.cpp")
set(olex_INCLUDE "${src}")
add_subdirectory(html)
add_library(olex_extras STATIC
    ${olex_extras_SRCS}
)

target_link_libraries(olex_extras PRIVATE
    glib
    wx-static
)

target_include_directories(olex_extras PUBLIC
    ${olex_INCLUDE}
)
#set(glib_INCLUDE ${glib_INCLUDE} PARENT_SCOPE)
set_target_properties(olex_extras PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
