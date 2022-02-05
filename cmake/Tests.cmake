# Setup options for the supported test frameworks
set(CPP_PROJECT_TEMPLATE_USING_GTEST FALSE)

macro(ENABLE_TESTS)
	option(DSA_RUNTIME_TESTS "Compile dsa runtime tests" FALSE)
	set(CPP_PROJECT_TEMPLATE_USING_GTEST ${DSA_RUNTIME_TESTS})

	if(${CPP_PROJECT_TEMPLATE_USING_GTEST})
		enable_testing()
	endif()
endmacro()
