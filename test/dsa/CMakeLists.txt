if (${DSA_RUNTIME_TESTS})
	add_subdirectory(dsa_runtime)
endif()

if (${DSA_STATIC_TESTS} OR ${DSA_STATIC_DEBUG_TESTS})
	add_subdirectory(dsa_static)
endif()
