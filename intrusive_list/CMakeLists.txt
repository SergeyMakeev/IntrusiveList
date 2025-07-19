cmake_minimum_required(VERSION 3.14)

# Create header-only interface library
add_library(intrusive_list INTERFACE)

# Set C++17 requirement
target_compile_features(intrusive_list INTERFACE cxx_std_17)

# Add the current directory to include path
target_include_directories(intrusive_list INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Optional: Set up installation (uncomment if you want to install the library)
# include(GNUInstallDirs)
# 
# # Install the header file
# install(FILES intrusive_list.hpp
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/intrusive
# )
# 
# # Install the target
# install(TARGETS intrusive_list
#     EXPORT intrusive_list_targets
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
# 
# # Install the export set
# install(EXPORT intrusive_list_targets
#     FILE intrusive_list-config.cmake
#     NAMESPACE intrusive::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/intrusive_list
# )