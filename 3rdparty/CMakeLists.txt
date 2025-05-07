CPMAddPackage(
    NAME wxWidgetsSource
    VERSION "3.1.5"
    URL "https://github.com/wxWidgets/wxWidgets/releases/download/v3.0.5/wxWidgets-3.0.5.tar.bz2"
    DOWNLOAD_ONLY YES
)

include(ExternalProject)
if(wxWidgetsSource_ADDED)
    message(STATUS "wxWidgets source downloaded to: ${wxWidgetsSource_SOURCE_DIR}")

    # Define install location within the build directory
    set(wxWidgets_INSTALL_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets)
    # Define build location (out-of-source build is cleaner)
    set(wxWidgets_BUILD_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets_build)

    # List the components needed (matches your original find_package request)
    # Note: 'base' and 'core' are often implicitly built. Check wxWidgets configure options.
    #       'std' often covers base, core, net, xml. We'll enable others explicitly.
    set(WX_COMPONENTS_ARGS --enable-unicode --enable-gl --enable-html --enable-aui --enable-adv)
    # Add --enable-debug for debug builds if desired
    # Add --with-gtk=2 or --with-gtk=3 if needed, otherwise defaults are used

    ExternalProject_Add(
        wxWidgetsBuild # Name of the external project target itself
        SOURCE_DIR ${wxWidgetsSource_SOURCE_DIR}
        BINARY_DIR ${wxWidgetsSource_SOURCE_DIR}/build

        CONFIGURE_COMMAND ../configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
                            --disable-shared
                            --with-gtk2
                            --prefix=${CMAKE_BINARY_DIR}/3rdparty/wxwidgets
                              ${WX_COMPONENTS_ARGS}
                            --disable-sys-libs
                            --with-libjpeg=builtin
                            --with-libtiff=builtin
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON

        # List key output files to help CMake track build success.
        # These MUST exist after 'make install' in the INSTALL_DIR.
        # Include wx-config and at least one representative library from *each* component.
        # **This part is brittle and requires knowing the exact output names!**
        # Example names for GTK2, Unicode, Static 3.0.5 - VERIFY THESE!
        BUILD_BYPRODUCTS
            ${wxWidgets_INSTALL_DIR}/bin/wx-config
            # GTK Components (depend on base)
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_gl-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_core-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_aui-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_adv-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_html-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_propgrid-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_qa-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_ribbon-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_richtext-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_stc-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_xrc-3.0.a
            # Other components
            ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-3.0.a
            # Built-in dependencies (often needed by base/core/adv)
            ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a
             # Base libraries last (provide symbols for others)
            ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-3.0.a
            ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-3.0.a

    )
    set(wxWidgets_LIB_DIR ${wxWidgets_INSTALL_DIR}/lib PARENT_SCOPE)
    # Create an INTERFACE library target to represent the built wxWidgets
    # This target doesn't correspond to a single file but propagates usage requirements.
    add_library(wx-static INTERFACE IMPORTED GLOBAL)
    add_dependencies(wx-static wxWidgetsBuild) # Crucial: wxWidgets interface target depends on the build finishing

    # Define include directories based on the install location
    # The second path is for the platform-specific setup.h
    # ** Verify the setup header path (e.g., gtk2-unicode-static-3.0)! **
    # You might find it by inspecting the INSTALL_DIR after a successful build.
    set(WXWIDGETS_SETUP_H_INCLUDE_DIR ${wxWidgets_INSTALL_DIR}/lib/wx/include/gtk2-unicode-static-3.0) #<- EXAMPLE PATH! VERIFY!
    set(wxWidgets_INCLUDE_DIR_1
        ${wxWidgets_INSTALL_DIR}/include/wx-3.0
        PARENT_SCOPE
    )
    set(wxWidgets_INCLUDE_DIR_2
        ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
        PARENT_SCOPE
    )
    file(MAKE_DIRECTORY ${wxWidgets_INSTALL_DIR}/include/wx-3.0)
    file(MAKE_DIRECTORY ${WXWIDGETS_SETUP_H_INCLUDE_DIR})
    target_include_directories(wx-static INTERFACE
        ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
        ${wxWidgets_INSTALL_DIR}/include/wx-3.0
    )

    # Define compile definitions (like WXUSINGDLL=0 for static builds)
    # wx-config --cppflags usually provides these. We guess the common ones.

    # Define compiler flags (like -pthread or GTK flags)
    # wx-config --cxxflags usually provides these. We guess common ones.
    # ** This is highly platform/build dependent! **
    # Example flags often needed for static GTK builds:
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GTK2 REQUIRED gtk+-2.0) # Or gtk+-3.0 if you used --with-gtk=3
        pkg_check_modules(X11 REQUIRED x11)
        # Add checks for other dependencies if needed (e.g., opengl, webkit)
    else()
        message(WARNING "pkg-config not found. Compiler flags for wxWidgets dependencies might be incomplete.")
    endif()
    target_compile_options(wx-static INTERFACE
        # Flags from wx-config --cxxflags, like:
        "-pthread"                     # Common for GTK/X11 apps
        ${GTK2_CFLAGS_OTHER}          # Other flags from pkg-config
        ${X11_CFLAGS_OTHER}
        "-D__WXGTK__"                # Define the toolkit
        # Add other flags as needed
    )
     target_include_directories(wx-static INTERFACE
        ${GTK2_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
     )


    # Define libraries to link against
    # This is the most complex part without wx-config at configure time.
    # We need the full paths to the built wxWidgets static libraries AND system dependencies.
    # ** Library names (baseu, gtk2u_core) depend on build config (unicode, gtk version, static)! VERIFY! **
    # ** System libraries depend on platform and wxWidgets components used! **
    # Example for static GTK2 build on Linux:
    set(WX_LIBRARIES
        # GTK Components (depend on base)
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_gl-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_core-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_aui-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_adv-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_html-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_propgrid-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_qa-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_ribbon-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_richtext-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_stc-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_xrc-3.0.a
        # Other components
        ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-3.0.a
        # Built-in dependencies (often needed by base/core/adv)
        ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a
         # Base libraries last (provide symbols for others)
        ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-3.0.a

    )
    set(WX_LIBRARIES ${WX_LIBRARIES} PARENT_SCOPE)
    # System libs (obtained via 'wx-config --libs' on a similar system, or pkg-config)
    # These are VERY LIKELY INCOMPLETE or specific to one Linux setup
    set(WX_SYSTEM_LIBS
        ${GTK2_LIBRARIES}   # From pkg-config
        X11
        SM                  # X11 Session Management
        dl                  # Dynamic loading
        m                   # Math library
#        jpeg                # Add back for image support
#        tiff                # Add back for image support
#        expat               # Add back for XML support (often used by base)
        pthread             # Threading
#        z
      lzma
        )
    set(WX_SYSTEM_LIBS ${WX_SYSTEM_LIBS} PARENT_SCOPE)
    target_link_libraries(wx-static INTERFACE
        ${WX_LIBRARIES}
        ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
        ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a # Add built-in deps explicitly
        # Potentially add built-in libwxtiff, libwxjpeg, libwxexpat if they exist and are needed. Check wx install dir.
        ${WX_SYSTEM_LIBS})


    message(STATUS "wxWidgets INTERFACE target created.")
    message(STATUS "  Include Dirs: ${wxWidgets_INSTALL_DIR}/include/wx-3.0;${WXWIDGETS_SETUP_H_INCLUDE_DIR}")
    message(STATUS "  Libraries: <See target_link_libraries call above>")
    message(WARNING "wxWidgets linking flags and system dependencies are hardcoded guesses based on common static GTK builds. Verify these for your platform and build options!")

else()
    message(FATAL_ERROR "Failed to download wxWidgets source using CPM.")
endif()
