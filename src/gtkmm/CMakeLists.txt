find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
add_executable(test_gtkmm hello_world.cpp main.cpp)
target_link_libraries(test_gtkmm PRIVATE project_warnings project_options PkgConfig::GTKMM)
