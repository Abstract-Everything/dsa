# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(
		DOWNLOAD
		"https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
		EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
		TLS_VERIFY ON)
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")

set(CONAN_EXTRA_OPTIONS "")

if(${CPP_TMPL_TESTING} STREQUAL "gtest")
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} gtest/1.11.0)
endif()

if(${CPP_TMPL_DOCS_DOXYGEN})
	set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} docopt.cpp/0.6.2)
endif()

set(CONAN_EXTRA_OPTIONS ${CONAN_EXTRA_OPTIONS} sfml:audio=False)

conan_add_remote(
	NAME bincrafters
	INDEX 1
	URL https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
	VERIFY_SSL True)

conan_cmake_configure(
	REQUIRES ${CONAN_EXTRA_REQUIRES}
		 fmt/6.2.0
		 spdlog/1.5.0
		 # We use an older version because v2.11 has an issue on MSVC
		 # https://github.com/microsoft/vcpkg/pull/20674
		 freetype/2.10.4
		 imgui-sfml/2.3@bincrafters/stable
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
