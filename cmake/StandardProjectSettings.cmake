# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")

	set(CMAKE_BUILD_TYPE
	    RelWithDebInfo
	    CACHE STRING "Choose the type of build." FORCE)

	# Set the possible values of build type for cmake-gui, ccmake
	set_property(
		CACHE CMAKE_BUILD_TYPE
		PROPERTY STRINGS
			 "Debug"
			 "Release"
			 "MinSizeRel"
			 "RelWithDebInfo")
endif()

# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(${CPP_PROJECT_TEMPLATE_USING_CLANG})
	add_compile_options(-fcolor-diagnostics)
elseif(${CPP_PROJECT_TEMPLATE_USING_GCC})
	add_compile_options(-fdiagnostics-color=always)
else()
	message(
		STATUS "No colored compiler diagnostic set for '${CMAKE_CXX_COMPILER_ID}' compiler."
	)
endif()

set(CPP_TMPL_STANDARD
    20
    CACHE STRING "Choose the c++ standard.")

set_property(
	CACHE CPP_TMPL_STANDARD
	PROPERTY STRINGS
		 "11"
		 "14"
		 "17"
		 "20")

# Sets c++ target standard
function(set_standard project)
	if(${CPP_TMPL_STANDARD} STREQUAL "11")
		set(standard cxx_std_11)
	elseif(${CPP_TMPL_STANDARD} STREQUAL "14")
		set(standard cxx_std_14)
	elseif(${CPP_TMPL_STANDARD} STREQUAL "17")
		set(standard cxx_std_17)
	elseif(${CPP_TMPL_STANDARD} STREQUAL "20")
		set(standard cxx_std_20)
	endif()

	target_compile_features(${project} INTERFACE ${standard})
endfunction()
