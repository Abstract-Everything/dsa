# Enables compilation with ccache or sccache
function(enable_ccache)
	set(CPP_TMPL_COMPILE_CACHE
	    "off"
	    CACHE STRING "Compiler cache to be used")

	set_property(CACHE CPP_TMPL_COMPILE_CACHE PROPERTY STRINGS "off" "ccache" "sccache")

	if(${CPP_TMPL_COMPILE_CACHE} STREQUAL "off")
		return()
	endif()

	find_program(CACHE_BINARY ${CPP_TMPL_COMPILE_CACHE})
	if(NOT CACHE_BINARY)
		message(FATAL_ERROR "${CPP_TMPL_COMPILE_CACHE} is enabled but was not found")
		return()
	endif()

	message(STATUS "${CPP_TMPL_COMPILE_CACHE} found and enabled")
	set(CMAKE_CXX_COMPILER_LAUNCHER
	    ${CACHE_BINARY}
	    PARENT_SCOPE)
endfunction()
