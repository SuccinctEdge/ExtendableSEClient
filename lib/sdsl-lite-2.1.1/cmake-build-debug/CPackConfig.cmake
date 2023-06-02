# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1;/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/cmake-build-debug")
set(CPACK_CMAKE_GENERATOR "Ninja")
set(CPACK_COMPONENTS_ALL "")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/home/joffrey/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/221.5080.224/bin/cmake/linux/share/cmake-3.22/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "sdsl built using CMake")
set(CPACK_GENERATOR "TGZ;TBZ2;ZIP")
set(CPACK_INSTALL_CMAKE_PROJECTS "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/cmake-build-debug;sdsl;ALL;/")
set(CPACK_INSTALL_PREFIX "/usr/local")
set(CPACK_MODULE_PATH "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/external/libdivsufsort/CMakeModules")
set(CPACK_NSIS_DISPLAY_NAME "sdsl 2.0.2")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "sdsl 2.0.2")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OUTPUT_CONFIG_FILE "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/cmake-build-debug/CPackConfig.cmake")
set(CPACK_PACKAGE_CONTACT "yuta.256@gmail.com")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/external/libdivsufsort/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A lightweight suffix sorting library")
set(CPACK_PACKAGE_FILE_NAME "libdivsufsort-2.0.2-Linux-x86_64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "sdsl 2.0.2")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "sdsl 2.0.2")
set(CPACK_PACKAGE_NAME "sdsl")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "Yuta Mori")
set(CPACK_PACKAGE_VERSION "2.0.2")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "2")
set(CPACK_RESOURCE_FILE_LICENSE "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/external/libdivsufsort/LICENSE")
set(CPACK_RESOURCE_FILE_README "/home/joffrey/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/221.5080.224/bin/cmake/linux/share/cmake-3.22/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "/home/joffrey/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/221.5080.224/bin/cmake/linux/share/cmake-3.22/Templates/CPack.GenericWelcome.txt")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;ZIP")
set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/build/;/\\.build/;/\\.svn/;~$")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/cmake-build-debug/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "libdivsufsort-2.0.2")
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_STRIP_FILES "")
set(CPACK_SYSTEM_NAME "Linux-x86_64")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Linux-x86_64")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/home/joffrey/Documents/Repos/SEPrivate/lib/sdsl-lite-2.1.1/cmake-build-debug/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()