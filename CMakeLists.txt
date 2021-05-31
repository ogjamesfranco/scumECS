cmake_minimum_required(VERSION 3.20)
project(scumECS VERSION 1.0)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(test_search ${PROJECT_SOURCE_DIR}/tests/test_search.cpp)
set_property(TARGET test_search PROPERTY CXX_STANDARD 17)

enable_testing()
add_test("Search FizzBuzz" test_search)
