cmake_minimum_required(VERSION 3.30)
project(jowi_test_lib VERSION 1.0.0)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)
set (CMAKE_CXX_COMPILE_FEATURES "${CMAKE_CXX_COMPILE_FEATURES} -Wall -Werror")
option (JOWI_TEST_LIB_BUILD_TESTS "Build tests" OFF)
include (CTest)

if (NOT TARGET ${PROJECT_NAME})
  add_library(${PROJECT_NAME})
  add_library(jowi::test_lib ALIAS ${PROJECT_NAME})
  target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES 
        FILES
          ${CMAKE_CURRENT_LIST_DIR}/src/assert.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/exception.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/randomizer.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/reflection.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/test_context.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/test_entry.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/test_lib.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/test_runner.cc
          ${CMAKE_CURRENT_LIST_DIR}/src/test_suite.cc
      FILE_SET HEADERS 
        BASE_DIRS 
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:include>
        FILES 
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/jowi/test_lib.hpp>
          $<INSTALL_INTERFACE:include/jowi/test_lib.hpp>
  )
endif()

if (NOT TARGET jowi::cli)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/jowi-cli)
endif()

set(JOWI_TEST_LIB_NAME ${PROJECT_NAME} CACHE INTERNAL "The name of the jowi testing library")
set (JOWI_TEST_LIB_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/main.cc" CACHE INTERNAL "The path to the jowi test library main function")
# Function to add a test into the suite. 
function(jowi_add_test target_name)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs 
    TARGETS
    LIBRARIES
    DIRECTORIES
    SANITIZERS
    COMPILE_DEFINITIONS
    COMPILE_OPTIONS
    COMPILE_FEATURES
    LINK_OPTIONS
  )
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  # This adds the non sanitized test
  include(CTest)
  list(APPEND ARG_TARGETS ${JOWI_TEST_LIB_MAIN})
  list(APPEND ARG_TARGETS ${ARG_UNPARSED_ARGUMENTS})

  function (add_sanitizer target_name)
    set(oneValueArgs SANITIZER)
    add_executable(${target_name} ${ARG_TARGETS})
    if (ARG_LIBRARIES)
      target_link_libraries(${target_name} 
        PRIVATE 
          ${ARG_LIBRARIES} 
          jowi::test_lib
          jowi::cli
          jowi::cli::ui
      )
    else()
      target_link_libraries(${target_name} 
        PRIVATE 
          jowi::test_lib
          jowi::cli
          jowi::cli::ui
      )
    endif()
    if(ARG_COMPILE_DEFINITIONS)
      target_compile_definitions(${target_name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()
    if (ARG_DIRECTORIES)
      target_include_directories(${target_name} PRIVATE ${ARG_DIRECTORIES})
    endif()
    if (ARG_LINK_OPTIONS)
      target_link_options(${target_name} PRIVATE "${ARG_LINK_OPTIONS}")
    endif()
    if (SANITIZER)
      target_compile_options(${target_name} PRIVATE "-fsanitize=${SANITIZER}")
      target_link_options(${target_name} PRIVATE "-fsanitize=${SANITIZER}")
    endif()
    if (ARG_COMPILE_OPTIONS)
      target_compile_options(${target_name} PRIVATE ${ARG_COMPILE_OPTIONS})
    endif()
    if (ARG_COMPILE_FEATURES)
      target_compile_features(${target_name} PRIVATE ${ARG_COMPILE_FEATURES})
    endif()
    add_test(NAME ${target_name} COMMAND ${target_name})
  endfunction()

  # Add executables for testing
  add_sanitizer(${target_name})
  if ("all" IN_LIST ARG_SANITIZERS)
    set (ARG_SANITIZERS "address;thread;undefined;memory")
  endif()
  if ("address" IN_LIST ARG_SANITIZERS)
    add_sanitizer(${target_name}_asan SANITIZER "address")
  endif()
  if ("thread" IN_LIST ARG_SANITIZERS)
    add_sanitizer(${target_name}_tsan SANITIZER "thread")
  endif()
  if ("undefined" IN_LIST ARG_SANITIZERS)
    add_sanitizer(${target_name}_ubsan SANITIZER "undefined")
  endif()
  if ("memory" IN_LIST ARG_SANITIZERS AND NOT APPLE)
    add_sanitizer(${target_name}_msan SANITIZER "memory")
  endif()
endfunction()

if (JOWI_TEST_LIB_BUILD_TESTS)
  jowi_add_test(
    ${PROJECT_NAME}_asserts
    ${CMAKE_CURRENT_LIST_DIR}/tests/asserts.cc
    SANITIZERS thread undefined address
  )

  jowi_add_test(
    ${PROJECT_NAME}_tests
    ${CMAKE_CURRENT_LIST_DIR}/tests/tests.cc
    SANITIZERS thread undefined address
  )
endif()

set (JOWI_COMPONENT_NAME "test_lib")
include(GNUInstallDirs)
install (
  TARGETS ${PROJECT_NAME}
  EXPORT jowi
  FILE_SET
    HEADERS
    COMPONENT ${JOWI_COMPONENT_NAME}_header
  FILE_SET 
    CXX_MODULES 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cxx_modules/jowi/${JOWI_COMPONENT_NAME}
    COMPONENT ${JOWI_COMPONENT_NAME}_file_set
  CXX_MODULES_BMI 
    COMPONENT ${JOWI_COMPONENT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cxx_modules/jowi/${JOWI_COMPONENT_NAME}
)

if (PROJECT_IS_TOP_LEVEL)
  install (
    EXPORT jowi
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/jowi
    FILE jowi-config.cmake
    NAMESPACE jowi::
    CXX_MODULES_DIRECTORY cxx_modules/jowi
  )
endif()