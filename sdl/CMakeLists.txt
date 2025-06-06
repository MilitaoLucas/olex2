SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB sdl_SRCS "${src}/*.cpp")
FILE(GLOB sdlsmart_SRCS "${src}/smart/*.cpp")
FILE(GLOB sdlexp_SRCS "${src}/exparse/*.cpp")
FILE(GLOB sdl_math_SRCS "${src}/math/*.cpp")
set(sdl_INCLUDE "${src}")
add_library(sdl_core_lib STATIC
    ${sdl_SRCS}
    ${sdlsmart_SRCS}
    ${sdlexp_SRCS}
    ${sdl_math_SRCS}
)
target_link_libraries(sdl_core_lib PRIVATE
    wx-static
)

target_include_directories(sdl_core_lib PUBLIC
    ${sdl_INCLUDE}
)
set_target_properties(sdl_core_lib PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
set(sdl_INCLUDE ${sdl_INCLUDE} PARENT_SCOPE)
