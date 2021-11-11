# Enables the options for supported sanitizers and validates them
function(enable_sanitizers project)

	if(NOT (CPP_PROJECT_TEMPLATE_USING_CLANG OR CPP_PROJECT_TEMPLATE_USING_GCC))
		return()
	endif()

	option(CPP_TMPL_ANALYZE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
	option(CPP_TMPL_SANITIZER_ADDRESS "Enable address sanitizer" FALSE)
	option(CPP_TMPL_SANITIZER_LEAK "Enable leak sanitizer" FALSE)
	option(CPP_TMPL_SANITIZER_UB "Enable undefined behavior sanitizer" FALSE)
	option(CPP_TMPL_SANITIZER_THREAD "Enable thread sanitizer" FALSE)
	option(CPP_TMPL_SANITIZER_MEMORY "Enable memory sanitizer" FALSE)

	if(${CPP_TMPL_ANALYZE_COVERAGE})
		target_compile_options(${project} INTERFACE --coverage -O0 -g)
		target_link_libraries(${project} INTERFACE --coverage)
	endif()

	set(sanitizers "")
	if(${CPP_TMPL_SANITIZER_ADDRESS})
		list(APPEND sanitizers "address")
	endif()

	if(${CPP_TMPL_SANITIZER_LEAK})
		list(APPEND sanitizers "leak")
	endif()

	if(${CPP_TMPL_SANITIZER_UB})
		list(APPEND sanitizers "undefined")
	endif()

	if(${CPP_TMPL_SANITIZER_THREAD})
		if("address" IN_LIST sanitizers OR "leak" IN_LIST sanitizers)
			message(
				WARNING
					"Thread sanitizer does not work with Address and Leak sanitizer enabled"
			)
		else()
			list(APPEND sanitizers "thread")
		endif()
	endif()

	if(${CPP_TMPL_SANITIZER_MEMORY} AND CPP_PROJECT_TEMPLATE_USING_CLANG)
		if("address" IN_LIST sanitizers
		   OR "thread" IN_LIST sanitizers
		   OR "leak" IN_LIST sanitizers)
			message(
				WARNING
					"Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled"
			)
		else()
			list(APPEND sanitizers "memory")
		endif()
	endif()

	list(JOIN sanitizers "," list_of_sanitizers)

	if(list_of_sanitizers AND NOT ${list_of_sanitizers} STREQUAL "")
		target_compile_options(${project} INTERFACE -fsanitize=${LIST_OF_sanitizers})
		target_link_options(${project} INTERFACE -fsanitize=${LIST_OF_sanitizers})
	endif()

endfunction()
