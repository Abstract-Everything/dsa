add_library(library STATIC ${CMAKE_CURRENT_LIST_DIR}/src/library.cpp)

target_include_directories(
	library
	INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/library)

target_link_libraries(library PRIVATE project_options project_warnings)
