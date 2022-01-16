find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(dsa_test dynamic_array.cpp vector.cpp list.cpp)

target_link_libraries(
	dsa_test
	PRIVATE project_options
		project_warnings
		dsa
		GTest::gtest
		GTest::gtest_main)

gtest_discover_tests(dsa_test)
