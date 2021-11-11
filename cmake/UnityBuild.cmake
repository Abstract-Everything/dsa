option(CPP_TMPL_COMPILE_UNITYBUILD "Enable Unity builds of projects" OFF)

# Uses unity build for the given project
function(enable_unitybuild project)
	if(${CPP_TMPL_COMPILE_UNITYBUILD})
		# Add for any project you want to apply unity builds for
		set_target_properties(${project} PROPERTIES UNITY_BUILD ON)
	endif()
endfunction()
