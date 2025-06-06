SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB repo_SRCS "${src}/*.cpp")
FILE(GLOB repo_INCLUDE "${src}")
add_library(repo_core_lib STATIC
    ${repo_SRCS}
)

IF (NOT WIN32)
  LIST(REMOVE_ITEM repo_SRCS "${src}/fsindex.cpp")
  LIST(REMOVE_ITEM repo_SRCS "${src}/unzip.cpp")
  LIST(REMOVE_ITEM repo_SRCS "${src}/zip.cpp")
endif (NOT WIN32)

target_link_libraries(repo_core_lib PRIVATE
    sdl_core_lib
    wx-static
)

target_include_directories(repo_core_lib PUBLIC
    ${repo_INCLUDE}
)
#set(glib_INCLUDE ${glib_INCLUDE} PARENT_SCOPE)
set_target_properties(repo_core_lib PROPERTIES POSITION_INDEPENDENT_CODE ON) # Add this line
target_compile_definitions(repo_core_lib PRIVATE _OPENSSL _PYTHON)
