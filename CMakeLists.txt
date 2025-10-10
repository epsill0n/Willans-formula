cmake_minimum_required(VERSION 3.16)
project(WillansFormula LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgmpxx -lgmp -lmpfr -O3 -march=native -flto")

add_executable(all_until_nth_prime cpp/all_until_nth_prime.cpp)
target_link_libraries(all_until_nth_prime PRIVATE gmpxx gmp mpfr)

add_executable(nth_prime cpp/nth_prime.cpp)
target_link_libraries(nth_prime PRIVATE gmpxx gmp mpfr)

set_target_properties(all_until_nth_prime PROPERTIES SUFFIX ".exe")
set_target_properties(nth_prime PROPERTIES SUFFIX ".exe")

set_target_properties(all_until_nth_prime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/cpp)
set_target_properties(nth_prime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/cpp)
