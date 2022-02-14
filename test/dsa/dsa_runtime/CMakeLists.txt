find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(dsa_runtime_test dynamic_array.cpp vector.cpp list.cpp binary_tree.cpp heap.cpp algorithms.cpp)

target_link_libraries(
	dsa_runtime_test
	PRIVATE project_options
		project_warnings
		dsa
		GTest::gtest
		GTest::gtest_main)

gtest_discover_tests(dsa_runtime_test)
