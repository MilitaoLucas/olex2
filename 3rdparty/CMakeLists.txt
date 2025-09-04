set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(wxVERSION_FAMILY "3.2")
set(wxVERSION "${wxVERSION_FAMILY}.5")
CPMAddPackage(
    NAME wxWidgets
    GIT_REPOSITORY "https://github.com/wxWidgets/wxWidgets.git"
    VERSION "${wxVERSION}"
    OPTIONS
        "wxUSE_REGEX builtin"
        "wxUSE_ZLIB builtin"
        "wxUSE_EXPAT builtin"
        "wxUSE_LIBJPEG builtin"
        "wxUSE_LIBPNG builtin"
        "wxUSE_LIBTIFF builtin"
        "wxUSE_LIBLZMA builtin"
        "wxBUILD_SHARED off"
)
message(WARNING "Install: ${wxWidgets_INSTALL_DIR}")

if(wxWidgets_ADDED)
    message(STATUS "wxWidgets added successfully via CPM")
    add_library(wxWidgets::wxWidgets INTERFACE IMPORTED GLOBAL)
    set(wxWidgets_include1)
    target_include_directories(
        wxWidgets::wxWidgets INTERFACE
        "${wxWidgets_BINARY_DIR}/lib/wx/include/gtk3-unicode-static-${wxVERSION_FAMILY}"
        "${wxWidgets_SOURCE_DIR}/include/wx"
    )

    set(wxWidgets_INCLUDE_DIR_1
      ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY}
      PARENT_SCOPE
    )
    set(wxWidgets_INCLUDE_DIR_2
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      PARENT_SCOPE
    )
else()
    message(FATAL_ERROR "Failed to add wxWidgets via CPM")
endif()

