# == Project setup ==
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(JSOG)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable code coverage if not defined
if(NOT DEFINED CODE_COVERAGE)
  set(CODE_COVERAGE ON)
endif()

# == Packages ==
# Add nlohmann json package
find_package(nlohmann_json 3.2.0 CONFIG REQUIRED)
find_package(uriparser 0.9.2 CONFIG REQUIRED char)

# == Executable ==
# Glob all .cpp files in the current directory
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "src/main.cpp")

add_library(JSOG_lib STATIC ${SOURCES})

# Add the include directory
target_include_directories(JSOG_lib PUBLIC include)

# Link the nlohmann json and uriparser libraries to the JSOG_lib target
target_link_libraries(JSOG_lib PUBLIC nlohmann_json::nlohmann_json uriparser::uriparser)

target_compile_options(JSOG_lib PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Create the executable

# Create the executable
add_executable(JSOG)

# Add the sources to the executable
target_sources(JSOG PRIVATE src/main.cpp)


# Add the include directory
target_include_directories(JSOG PRIVATE include)

# Link the nlohmann json library
target_link_libraries(JSOG PRIVATE JSOG_lib nlohmann_json::nlohmann_json uriparser::uriparser)

file(GLOB TEST_COMPILABLES ${CMAKE_SOURCE_DIR}/tests/schemas_to_compile/*.json)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/built_samples/*.h ${CMAKE_BINARY_DIR}/built_samples/*.cpp ${CMAKE_BINARY_DIR}/built_samples/*.dump.json
  COMMAND ${CMAKE_BINARY_DIR}/JSOG -d -o ${CMAKE_BINARY_DIR}/built_samples ${TEST_COMPILABLES}
  DEPENDS JSOG ${TEST_COMPILABLES}
  COMMENT "Building samples"
)

add_custom_target(full_test_samples ALL
    DEPENDS ${CMAKE_BINARY_DIR}/built_samples/*.h ${CMAKE_BINARY_DIR}/built_samples/*.cpp ${CMAKE_BINARY_DIR}/built_samples/*.dump.json)

# == Tests ==
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_custom_target(copy_samples ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/samples ${CMAKE_BINARY_DIR}/samples)

file(GLOB TEST_SOURCES "tests/*.cpp")


add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain JSOG_lib  nlohmann_json::nlohmann_json uriparser::uriparser)

# Make tests depend on copy_samples as we need to copy the samples before running the tests
add_dependencies(tests copy_samples)

file(GLOB OUTPUT_TEST_SOURCES tests/output_tests/*.cpp)
file(GLOB OUTPUT_TEST_SCHEMA_SOURCES ${CMAKE_BINARY_DIR}/built_samples/*.cpp)

add_executable(output_tests ${OUTPUT_TEST_SOURCES} ${OUTPUT_TEST_SCHEMA_SOURCES})
include_directories(output_tests PRIVATE ${CMAKE_BINARY_DIR}/built_samples)
target_link_libraries(output_tests PRIVATE Catch2::Catch2WithMain JSOG_lib  nlohmann_json::nlohmann_json uriparser::uriparser)
add_dependencies(output_tests copy_samples full_test_samples)


# Add coverage flags to the compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(JSOG_lib PRIVATE --coverage)
  target_link_libraries(JSOG_lib PRIVATE --coverage)
  target_compile_options(tests PRIVATE --coverage)
  target_link_libraries(tests PRIVATE --coverage)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)


# Add the tests
include(CTest)
include(Catch)
catch_discover_tests(tests)
catch_discover_tests(output_tests)

enable_testing()

# Code coverage target
if(CODE_COVERAGE)
  add_custom_target(coverage
  COMMAND lcov --capture --directory . --output-file lcov.info --rc branch_coverage=1 --rc geninfo_unexecuted_blocks=1 --ignore-errors mismatch
  COMMAND lcov --remove lcov.info '/usr/*' '*/catch2/*' '*/tests/*' --output-file lcov.info
  COMMAND genhtml lcov.info --output-directory coverage_report --rc branch_coverage=1 --rc geninfo_unexecuted_blocks=1
  COMMENT "Generating coverage report..."
  )
endif()
