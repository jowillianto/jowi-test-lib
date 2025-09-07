cmake_minimum_required(VERSION 3.30)
project(jowi_test_lib)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)
set (CMAKE_CXX_COMPILE_FEATURES "-Wall -Werror")
option (JOWI_TEST_LIB_BUILD_TESTS "Build tests" OFF)
include (CTest)

if (NOT TARGET ${PROJECT_NAME})
  add_library(${PROJECT_NAME})
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
  add_executable(${target_name} ${ARG_TARGETS})

  function (add_sanitizer target_name sanitizer abbr)
    add_executable(${target_name}_${abbr} ${ARG_TARGETS})
    
    if(ARG_LIBRARIES)
      target_link_libraries(${target_name}_${abbr} PRIVATE ${ARG_LIBRARIES} ${JOWI_TEST_LIB_NAME})
    else()
      target_link_libraries(${target_name}_${abbr} PRIVATE ${JOWI_TEST_LIB_NAME})
    endif()
    if(ARG_COMPILE_DEFINITIONS)
      target_compile_definitions(${target_name}_${abbr} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()
    if (ARG_DIRECTORIES)
      target_include_directories(${target_name}_${abbr} PRIVATE ${ARG_DIRECTORIES})
    endif()
    if (ARG_LINK_OPTIONS)
      target_link_options(${target_name}_${abbr} PRIVATE "-fsanitize=${sanitizer}" ${ARG_LINK_OPTIONS})
    else()
      target_link_options(${target_name}_${abbr} PRIVATE "-fsanitize=${sanitizer}")
    endif()
    if (ARG_COMPILE_OPTIONS)
      target_compile_options(${target_name}_${abbr} PRIVATE "-fsanitize=${sanitizer}" ${ARG_COMPILE_OPTIONS})
    else()
      target_compile_options(${target_name}_${abbr} PRIVATE "-fsanitize=${sanitizer}")
    endif()
    if (ARG_COMPILE_FEATURES)
      target_compile_features(${target_name}_${abbr} PRIVATE ${ARG_COMPILE_FEATURES})
    endif()
    add_test(NAME ${target_name}_${abbr} COMMAND ${target_name}_${abbr})
  endfunction()

  # Normal Addition
  if (ARG_LIBRARIES)
    target_link_libraries(${target_name} PRIVATE ${ARG_LIBRARIES} ${JOWI_TEST_LIB_NAME})
  else()
    target_link_libraries(${target_name} PRIVATE ${JOWI_TEST_LIB_NAME})
  endif()
  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions(${target_name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
  endif()
  if (ARG_DIRECTORIES)
    target_include_directories(${target_name} PRIVATE ${ARG_DIRECTORIES})
  endif()
  if (ARG_LINK_OPTIONS)
    target_link_options(${target_name} PRIVATE ${ARG_LINK_OPTIONS})
  endif()
  if (ARG_COMPILE_OPTIONS)
    target_compile_options(${target_name} PRIVATE ${ARG_COMPILE_OPTIONS})
  endif()
  if (ARG_COMPILE_FEATURES)
    target_compile_features(${target_name} PRIVATE ${ARG_COMPILE_FEATURES})
  endif()
  add_test(NAME ${target_name} COMMAND ${target_name})

  if ("all" IN_LIST ARG_SANITIZERS)
    add_sanitizer(${target_name} "thread" "tsan")
    add_sanitizer(${target_name} "undefined" "ubsan")
    add_sanitizer(${target_name} "address" "asan")
    if (NOT APPLE)
      add_sanitizer(${target_name} "memory" "msan")
    endif()
  else()
    if ("address" IN_LIST ARG_SANITIZERS)
      add_sanitizer(${target_name} "address" "asan")
    endif()
    if ("thread" IN_LIST ARG_SANITIZERS)
      add_sanitizer(${target_name} "thread" "tsan")
    endif()
    if ("undefined" IN_LIST ARG_SANITIZERS)
      add_sanitizer(${target_name} "undefined" "ubsan")
    endif()
    if ("memory" IN_LIST ARG_SANITIZERS AND NOT APPLE)
      add_sanitizer(${target_name} "memory" "msan")
    endif()
  endif()
endfunction()

if (JOWI_TEST_LIB_BUILD_TESTS)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
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