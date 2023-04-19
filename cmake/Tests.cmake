# Setup options for the supported test frameworks
set(CPP_PROJECT_TEMPLATE_USING_CATCH FALSE)

macro(ENABLE_TESTS)
	option(DSA_STATIC_TESTS "Compile dsa static tests" FALSE)
	option(DSA_STATIC_DEBUG_TESTS "Compile dsa static tests into an executable for debugging" FALSE)

	set(CPP_PROJECT_TEMPLATE_USING_CATCH ${DSA_STATIC_TESTS} OR ${DSA_STATIC_DEBUG_TESTS})

	if(${CPP_PROJECT_TEMPLATE_USING_CATCH})
		enable_testing()
	endif()
endmacro()
