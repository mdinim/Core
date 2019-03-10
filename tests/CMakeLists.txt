set(CMAKE_CXX_FLAGS "-Wall -pedantic")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    target_include_directories(${TESTNAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endmacro()

package_add_test(Test_MessageQueue MessageQueue_test.cpp)
target_link_libraries(Test_MessageQueue Core)

package_add_test(Test_ThreadPool ThreadPool_test.cpp)
target_link_libraries(Test_ThreadPool Core)
