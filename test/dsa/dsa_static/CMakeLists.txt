find_package(Catch2 REQUIRED)
include(Catch)

add_library(dsa_tests STATIC main.cpp)

target_link_libraries(
	dsa_tests
	PUBLIC Catch2::Catch2 dsa
	PRIVATE project_options project_warnings)

if(${DSA_STATIC_TESTS})
	add_executable(dsa_static_tests allocator_traits.cpp)
	target_link_libraries(dsa_static_tests PRIVATE project_options project_warnings dsa_tests)

	catch_discover_tests(
		dsa_static_tests
		TEST_PREFIX "unittests."
		REPORTER xml
		OUTPUT_DIR .
		OUTPUT_PREFIX "unittests."
		OUTPUT_SUFFIX .xml)
endif()

if(${DSA_STATIC_DEBUG_TESTS})
	# Create an executable with conxtexpr tests executed at runtime in order to allow debugging
	add_executable(dsa_runtime_tests allocator_traits.cpp)
	target_link_libraries(dsa_runtime_tests PRIVATE project_options project_warnings dsa_tests)
	target_compile_definitions(dsa_runtime_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

	catch_discover_tests(
		dsa_runtime_tests
		TEST_PREFIX "debug.unittests."
		REPORTER xml
		OUTPUT_DIR .
		OUTPUT_PREFIX "debug.unittests."
		OUTPUT_SUFFIX .xml)
endif()
