find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(gtest gtest_main.cpp)

target_link_libraries(
	gtest
	PRIVATE project_options
		project_warnings
		library
		GTest::gtest
		GTest::gtest_main)

gtest_discover_tests(gtest)
