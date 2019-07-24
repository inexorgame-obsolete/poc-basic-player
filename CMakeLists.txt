cmake_minimum_required (VERSION 3.4)

project(inexor-poc-basic-player)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(BINARY_NAME "inexor-poc-basic-player")

# Dependency setup via conan

# Download conan executer in case it does not exists
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.13/conan.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

if(TARGET_WEB)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/toolchains/modules/")
    include(UseEmscripten)
    add_definitions("-DDEMANGLE_SUPPORT=1" "-DTARGET_WEB")
endif(TARGET_WEB)


# Required for conan cmake for handling an empty CMAKE_BUILD_TYPE
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

# Required conan remotes for different dependencies
conan_check(VERSION 1.4.0 REQUIRED)
conan_add_remote(NAME bincrafters INDEX 1
            URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_add_remote(NAME inexor INDEX 1
            URL https://api.bintray.com/conan/inexorgame/inexor-conan)
conan_add_remote(NAME helmesjo INDEX 1
            URL https://api.bintray.com/conan/helmesjo/public-conan)

if(UNIX AND NOT APPLE)
    set(LIBCXX libstdc++11)
	conan_cmake_run(CONANFILE conanfile.txt
        	        BASIC_SETUP
                	BUILD outdated
                	PROFILE default
                	PROFILE_AUTO build_type
			SETTINGS compiler.libcxx=${LIBCXX})
else()
	conan_cmake_run(CONANFILE conanfile.txt
			BASIC_SETUP
			BUILD outdated
			PROFILE default
			PROFILE_AUTO build_type)
endif()

include_directories("${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE NORMAL_SOURCE_FILES
  "src/*.hpp"
  "src/*.cpp"
)

message(STATUS "NORMAL SOURCE FILES: ${NORMAL_SOURCE_FILES}")
message(STATUS "CONAN_LIBS: ${CONAN_LIBS}")
 
add_executable(${BINARY_NAME} ${NORMAL_SOURCE_FILES})
target_link_libraries(${BINARY_NAME} ${CONAN_LIBS})

# Use folder structure in Visual Studio
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)
assign_source_group(${NORMAL_SOURCE_FILES})
