# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(
		DOWNLOAD
		"https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
		EXPECTED_HASH
			SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
		TLS_VERIFY ON)
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")

set(CONAN_EXTRA_OPTIONS "")

if(${CPP_TMPL_TESTING} STREQUAL "catch2")
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} catch2/2.13.3)
endif()

if(${CPP_TMPL_TESTING} STREQUAL "gtest")
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} gtest/1.11.0)
endif()

if(${CPP_TMPL_DOCS_DOXYGEN})
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} docopt.cpp/0.6.2)
endif()

if(${CPP_TMPL_GUI_IMGUI})
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} imgui-sfml/2.1@bincrafters/stable)

	# set(CONAN_EXTRA_OPTIONS ${CONAN_EXTRA_OPTIONS} sfml:shared=False sfml:graphics=True
	# sfml:audio=False sfml:window=True libalsa:disable_python=True)
endif()

conan_cmake_configure(
	REQUIRES ${CONAN_EXTRA_REQUIRES} fmt/6.2.0 spdlog/1.5.0
	OPTIONS ${CONAN_EXTRA_OPTIONS}
	GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(
	PATH_OR_REFERENCE .
	BUILD missing
	# This is a workaround until https://github.com/conan-io/cmake-conan/pull/343 is merged
	SETTINGS compiler.cppstd=${CPP_TMPL_STANDARD} ${settings})

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_BINARY_DIR}
    PARENT_SCOPE)
