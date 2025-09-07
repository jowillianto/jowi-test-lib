jowi_add_test(
  ${PROJECT_NAME}_asserts
  ${CMAKE_CURRENT_LIST_DIR}/asserts.cc
  SANITIZERS thread undefined address
)

jowi_add_test(
  ${PROJECT_NAME}_tests
  ${CMAKE_CURRENT_LIST_DIR}/tests.cc
  SANITIZERS thread undefined address
)