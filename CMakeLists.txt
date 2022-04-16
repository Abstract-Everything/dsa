cmake_minimum_required(VERSION 3.15)

project(
	cpp_data_structures_and_algorithms_visualisation
	VERSION 0.0.0.1
	DESCRIPTION "A visualisation of data structures and algorihtms using C++"
	LANGUAGES CXX)

include(cmake/Constants.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/StandardProjectSettings.cmake)
include(cmake/LinkTimeOptimization.cmake)
include(cmake/TimeTrace.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Cache.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Doxygen.cmake)
include(cmake/PrecompileHeaders.cmake)
include(cmake/Tests.cmake)
include(cmake/UnityBuild.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
set_standard(project_options)
enable_time_trace(project_options)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

enable_ipo()
enable_pch(project_options)
option(CPP_TMPL_BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)

enable_doxygen()
enable_ccache()
enable_sanitizers(project_options)
enable_static_analyzers()

enable_tests()

add_subdirectory(dependencies)
add_subdirectory(dsa)
add_subdirectory(visual)
add_subdirectory(test)
