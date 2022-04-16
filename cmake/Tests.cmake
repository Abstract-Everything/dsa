# Setup options for the supported test frameworks
set(CPP_PROJECT_TEMPLATE_USING_GTEST FALSE)
set(CPP_PROJECT_TEMPLATE_USING_CATCH FALSE)

macro(ENABLE_TESTS)
	option(DSA_RUNTIME_TESTS "Compile dsa runtime tests" FALSE)
	option(DSA_STATIC_TESTS "Compile dsa static tests" FALSE)
	option(DSA_STATIC_DEBUG_TESTS "Compile dsa static tests into an executable for debugging" FALSE)

	set(CPP_PROJECT_TEMPLATE_USING_GTEST ${DSA_RUNTIME_TESTS})
	set(CPP_PROJECT_TEMPLATE_USING_CATCH ${DSA_STATIC_TESTS} OR ${DSA_STATIC_DEBUG_TESTS})

	if(${CPP_PROJECT_TEMPLATE_USING_GTEST} OR ${CPP_PROJECT_TEMPLATE_USING_CATCH})
		enable_testing()
	endif()
endmacro()
