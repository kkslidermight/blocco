add_executable(test_math test_math.cpp)
set_project_warnings(test_math)
target_link_libraries(test_math PRIVATE blocco_engine)
add_test(NAME test_math COMMAND test_math)

add_executable(test_collision test_collision.cpp)
set_project_warnings(test_collision)
target_link_libraries(test_collision PRIVATE blocco_engine)
add_test(NAME test_collision COMMAND test_collision)
