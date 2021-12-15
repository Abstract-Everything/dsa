find_package(ImGui-SFML REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_executable(
	visual
	events/allocated_array_event.cpp
	events/copy_assignment_event.cpp
	events/deallocated_array_event.cpp
	events/event.cpp
	events/move_assignment_event.cpp
	gui/user_interface.cpp
	main.cpp
	main_window.cpp
	memory.cpp
	memory_allocation.cpp
	memory_value.cpp
	viewport.cpp)

target_include_directories(
	visual
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/events
		${CMAKE_CURRENT_LIST_DIR}/gui
		${CMAKE_CURRENT_LIST_DIR}/monitors)

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
