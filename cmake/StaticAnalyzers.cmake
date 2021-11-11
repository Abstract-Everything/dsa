# Enable static analyzers
function(enable_static_analyzers)
	option(CPP_TMPL_ANALYZE_CPPCHECK "Enable static analysis with cppcheck" OFF)
	option(CPP_TMPL_ANALYZE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
	option(CPP_TMPL_COMPILE_IWYU "Enable static analysis with include-what-you-use" OFF)

	if(${CPP_TMPL_ANALYZE_CPPCHECK})
		find_program(CPPCHECK cppcheck)
		if(CPPCHECK)
			set(CMAKE_CXX_CPPCHECK
			    ${CPPCHECK}
			    --suppress=missingInclude
			    --enable=all
			    --inline-suppr
			    --inconclusive
			    -i
			    ${CMAKE_SOURCE_DIR}/imgui/lib
			    PARENT_SCOPE)
		else()
			message(SEND_ERROR "cppcheck requested but executable not found")
		endif()
	endif()

	if(${CPP_TMPL_ANALYZE_CLANG_TIDY})
		find_program(CLANGTIDY clang-tidy)
		if(CLANGTIDY)
			set(CMAKE_CXX_CLANG_TIDY
			    ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option
			    PARENT_SCOPE)
		else()
			message(SEND_ERROR "clang-tidy requested but executable not found")
		endif()
	endif()

	if(${CPP_TMPL_COMPILE_IWYU})
		find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
		if(INCLUDE_WHAT_YOU_USE)
			set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
			    ${INCLUDE_WHAT_YOU_USE}
			    PARENT_SCOPE)
		else()
			message(
				SEND_ERROR "include-what-you-use requested but executable not found"
			)
		endif()
	endif()
endfunction()
