add_executable(unit_tests test_math.cpp test_collision.cpp)
set_project_warnings(unit_tests)
target_link_libraries(unit_tests PRIVATE blocco_engine)
add_test(NAME unit COMMAND unit_tests)
