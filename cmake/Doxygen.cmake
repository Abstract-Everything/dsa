# Enables documentation generation with DOxygen
function(enable_doxygen)
	option(CPP_TMPL_DOCS_DOXYGEN "Enable doxygen doc builds of source" OFF)
	if(${CPP_TMPL_DOCS_DOXYGEN})
		set(DOXYGEN_CALLER_GRAPH
		    YES
		    PARENT_SCOPE)

		set(DOXYGEN_CALL_GRAPH
		    YES
		    PARENT_SCOPE)

		set(DOXYGEN_EXTRACT_ALL
		    YES
		    PARENT_SCOPE)

		find_package(Doxygen)
		doxygen_add_docs(docs ${PROJECT_SOURCE_DIR})
	endif()
endfunction()
