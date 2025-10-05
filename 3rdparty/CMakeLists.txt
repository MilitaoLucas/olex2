# Make wxWidgets version configurable with default value
set(wxVERSION "3.2.5" CACHE STRING "wxWidgets version to use")

find_package(PkgConfig REQUIRED)
# Extract the family version (major.minor) from the full version
string(REGEX MATCH "^([0-9]+\\.[0-9]+)" wxVERSION_FAMILY "${wxVERSION}")
if(NOT wxVERSION_FAMILY)
    message(FATAL_ERROR "Invalid wxVERSION format: ${wxVERSION}. Expected format: X.Y.Z")
endif()
CPMAddPackage(
  NAME Ccache.cmake
  GITHUB_REPOSITORY TheLartians/Ccache.cmake
  VERSION 1.2.5
)
CPMAddPackage(
    NAME wxWidgetsSource
    VERSION "${wxVERSION}"
    URL "https://github.com/wxWidgets/wxWidgets/releases/download/v${wxVERSION}/wxWidgets-${wxVERSION}.tar.bz2"
    DOWNLOAD_ONLY YES
)

pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
include(ExternalProject)
if(wxWidgetsSource_ADDED)
  message(STATUS "wxWidgets source downloaded to: ${wxWidgetsSource_SOURCE_DIR}")
  set(wxWidgets_INSTALL_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets)
  set(wxWidgets_BUILD_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets_build)

  ExternalProject_Add(
      wxWidgetsBuild
      SOURCE_DIR ${wxWidgetsSource_SOURCE_DIR}
      BINARY_DIR ${wxWidgetsSource_SOURCE_DIR}/buildgtk

      CONFIGURE_COMMAND ../configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
      --disable-shared
      --prefix=${CMAKE_BINARY_DIR}/3rdparty/wxwidgets
      --enable-aui
      --enable-unicode
      --enable-html
      --enable-webview
      --disable-sys-libs
      --with-gtk=3
      --with-libjpeg=builtin
      --with-libtiff=builtin
      BUILD_COMMAND make -j${EXTERNAL_JOBS}
      INSTALL_COMMAND make install
      LOG_CONFIGURE ON
      LOG_BUILD ON
      LOG_INSTALL ON

      BUILD_BYPRODUCTS
      ${wxWidgets_INSTALL_DIR}/bin/wx-config
      # GTK Components (depend on base)
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_gl-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_core-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_aui-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_adv-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_html-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_webview-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_propgrid-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_qa-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_ribbon-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_richtext-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_stc-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_xrc-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-${wxVERSION_FAMILY}.a

  )
  message(WARNING "libwx_gtk3u_webview-${wxVERSION_FAMILY}.a")
  set(wxWidgets_LIB_DIR ${wxWidgets_INSTALL_DIR}/lib PARENT_SCOPE)
  add_library(wx-static INTERFACE IMPORTED GLOBAL)
  add_dependencies(wx-static wxWidgetsBuild)

  set(WXWIDGETS_SETUP_H_INCLUDE_DIR ${wxWidgets_INSTALL_DIR}/lib/wx/include/gtk3-unicode-static-${wxVERSION_FAMILY})
  set(wxWidgets_INCLUDE_DIR_1
      ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY}
      PARENT_SCOPE
  )
  set(wxWidgets_INCLUDE_DIR_2
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      PARENT_SCOPE
  )
  file(MAKE_DIRECTORY ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY})
  file(MAKE_DIRECTORY ${WXWIDGETS_SETUP_H_INCLUDE_DIR})
  target_include_directories(wx-static INTERFACE
      ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY}
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
  )
  target_link_directories(wx-static INTERFACE
      ${wxWidgets_INSTALL_DIR}/lib
  )
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(X11 REQUIRED x11)
  else()
    message(WARNING "pkg-config not found. Compiler flags for wxWidgets dependencies might be incomplete.")
  endif()
  target_compile_options(wx-static INTERFACE
      "-pthread"                     # Common for GTK/X11 apps
      ${WEBKIT2GTK_CFLAGS_OTHER}
      ${GTK3_CFLAGS_OTHER}          # Other flags from pkg-config
      ${X11_CFLAGS_OTHER}
      "-D__WXGTK__"                # Define the toolkit
  )
  target_include_directories(wx-static INTERFACE
      ${GTK3_INCLUDE_DIRS}
      ${X11_INCLUDE_DIRS}
  )


  set(WX_LIBRARIES
      # Order matters - more specific libraries first, then base libraries
      -L/home/lucas/Documents/olex2/cmake-build-debug-gcc/3rdparty/wxwidgets/lib -pthread
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_xrc-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_webview-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_stc-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_richtext-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_ribbon-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_propgrid-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_aui-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_gl-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_qa-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_html-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_core-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-3.2.a
${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-3.2.a
-lwebkit2gtk-4.0
-lsoup-2.4
-Wl,--export-dynamic
-lgmodule-2.0
-pthread
-ljavascriptcoregtk-4.0
-lGL
-lGLU
-lEGL
-lwayland-egl
-lwayland-client
-lgthread-2.0
-pthread
-lX11
-lSM
-lxkbcommon
-lgtk-3
-lgdk-3
-lz
-lpangocairo-1.0
-latk-1.0
-lcairo-gobject
-lcairo
-lgdk_pixbuf-2.0
-lgio-2.0
-lXtst
-lpangoft2-1.0
-lpango-1.0
-lgobject-2.0
-lglib-2.0
-lharfbuzz
-lfontconfig
-lfreetype
-lwxtiff-3.2
-lwxjpeg-3.2
-lwxpng-3.2
-lwxregexu-3.2
-lwxscintilla-3.2
-lwxexpat-3.2
-lwxzlib-3.2
-lcurl
-lm



  )
  set(WX_LIBRARIES ${WX_LIBRARIES} PARENT_SCOPE)
  set(WX_SYSTEM_LIBS
      ${GTK3_LIBRARIES}
      ${WEBKIT2GTK_LIBRARIES}  # Add this line
      X11
      SM
      dl
      m
      pthread
      lzma
  )
  set(WX_SYSTEM_LIBS ${WX_SYSTEM_LIBS} PARENT_SCOPE)
  target_link_libraries(wx-static INTERFACE
      ${WX_LIBRARIES}
      webkit2gtk-4.0
      ${WEBKIT2GTK_LIBRARIES}
      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-${wxVERSION_FAMILY}.a
      ${WX_SYSTEM_LIBS})

  add_custom_target(wxwidgets-static-build DEPENDS wx-static)
  message(STATUS "wxWidgets INTERFACE target created.")
  message(STATUS "  Include Dirs: ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY};${WXWIDGETS_SETUP_H_INCLUDE_DIR}")
  message(STATUS "  Libraries: <See target_link_libraries call above>")
  message(WARNING "wxWidgets linking flags and system dependencies are hardcoded guesses based on common static GTK builds. Verify these for your platform and build options!")

else()
  message(FATAL_ERROR "Failed to download wxWidgets source using CPM.")
endif()
