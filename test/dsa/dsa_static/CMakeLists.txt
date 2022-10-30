find_package(Catch2 REQUIRED)
include(Catch)

set(DSA_STATIC_TEST_INCLUDES "${CMAKE_CURRENT_LIST_DIR}")
set(DSA_STATIC_TEST_FILES allocator_traits_tests.cpp)

if(${DSA_STATIC_TESTS})
	add_executable(dsa_static_tests ${DSA_STATIC_TEST_FILES})
	target_include_directories(dsa_static_tests PRIVATE ${DSA_STATIC_TEST_INCLUDES})
	target_link_libraries(dsa_static_tests PRIVATE dsa project_options project_warnings Catch2::Catch2WithMain)

	catch_discover_tests(
		dsa_static_tests
		TEST_PREFIX "unittests."
		REPORTER XML
		OUTPUT_DIR .
		OUTPUT_PREFIX "unittests."
		OUTPUT_SUFFIX .xml)
endif()

if(${DSA_STATIC_DEBUG_TESTS})
	# Create an executable with conxtexpr tests executed at runtime in order to allow debugging
	add_executable(dsa_runtime_tests ${DSA_STATIC_TEST_FILES})
	target_link_libraries(dsa_runtime_tests PRIVATE dsa project_options project_warnings Catch2::Catch2WithMain)
	target_compile_definitions(dsa_runtime_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

	catch_discover_tests(
		dsa_runtime_tests
		TEST_PREFIX "debug.unittests."
		REPORTER XML
		OUTPUT_DIR .
		OUTPUT_PREFIX "debug.unittests."
		OUTPUT_SUFFIX .xml)
endif()
