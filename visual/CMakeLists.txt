find_package(ImGui-SFML REQUIRED)
find_package(fmt REQUIRED)

add_executable(visual main.cpp array.cpp main_window.cpp)
target_link_libraries(visual PRIVATE project_warnings project_options fmt::fmt
				     ImGui-SFML::ImGui-SFML)

add_custom_target(
	fonts
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/fonts
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/fonts
		${CMAKE_BINARY_DIR}/fonts
	COMMENT "copying ${CMAKE_CURRENT_LIST_DIR}/fonts to ${CMAKE_BINARY_DIR}/fonts"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_dependencies(visual fonts)
