find_package(ImGui-SFML REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_executable(
	visual
	gui/user_interface.cpp
	gui/components/memory.cpp
	main.cpp
	main_window.cpp
	viewport.cpp
	enable_event_registration_scope.cpp)

target_include_directories(
	visual
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/gui
		${CMAKE_CURRENT_LIST_DIR}/gui/components
		${CMAKE_CURRENT_LIST_DIR}/utilities)

target_link_libraries(
	visual
	PRIVATE project_warnings
		project_options
		dsa
		fmt::fmt
		spdlog::spdlog
		ImGui-SFML::ImGui-SFML)

add_custom_target(
	fonts
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/fonts
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/fonts
		${CMAKE_BINARY_DIR}/fonts
	COMMENT "copying ${CMAKE_CURRENT_LIST_DIR}/fonts to ${CMAKE_BINARY_DIR}/fonts"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_dependencies(visual fonts)
