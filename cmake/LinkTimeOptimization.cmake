# Enables Link time optimization setting
macro(ENABLE_IPO)
	option(CPP_TMPL_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)"
	       OFF)

	if(${CPP_TMPL_IPO})
		include(CheckIPOSupported)
		check_ipo_supported(RESULT result OUTPUT output)
		if(result)
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
		else()
			message(SEND_ERROR "IPO is not supported: ${output}")
		endif()
	endif()
endmacro()
