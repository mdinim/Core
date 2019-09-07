set(CMAKE_CXX_FLAGS "-Wall -pedantic")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})

    add_common_compiler_options(${TESTNAME})

    target_link_libraries(${TESTNAME} gtest gmock gtest_main
            $<$<AND:$<BOOL:${CREATE_COVERAGE_REPORT}>,$<CXX_COMPILER_ID:GNU>>:gcov>)

    target_link_options(${TESTNAME} PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fprofile-instr-generate>)

    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    target_include_directories(${TESTNAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endmacro()

function(generate_overall_coverage_report)
    set(COVERAGE_NAME Overall_coverage)
    set(TEST_TARGET_NAME Test_Core)
    set(COVERAGE_LOCATION ${CMAKE_BINARY_DIR}/${COVERAGE_NAME})

    if (${COMPILER_IS_CLANG})
        set(COVERAGE_PROFRAW_FILE_LOC ${CMAKE_BINARY_DIR}/${COVERAGE_NAME}.profraw)
        set(COVERAGE_PROFDATA_FILE_LOC ${CMAKE_BINARY_DIR}/${COVERAGE_NAME}.profdata)

        add_custom_target(${COVERAGE_NAME}_preproc
                COMMAND LLVM_PROFILE_FILE=${COVERAGE_NAME}.profraw $<TARGET_FILE:${TEST_TARGET_NAME}>
                COMMAND llvm-profdata merge -sparse ${COVERAGE_PROFRAW_FILE_LOC} -o ${COVERAGE_NAME}.profdata
                DEPENDS ${TEST_TARGET_NAME}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                )

        add_custom_target(${COVERAGE_NAME}_show
                COMMAND llvm-cov show
                $<TARGET_FILE:${TEST_TARGET_NAME}>
                -instr-profile=${COVERAGE_PROFDATA_FILE_LOC}
                -show-line-counts-or-regions
                -Xdemangler c++filt
                -Xdemangler -n
                ${ARGN}
                DEPENDS ${TEST_TARGET_NAME}_preproc
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )

        add_custom_target(${COVERAGE_NAME}_report
                COMMAND llvm-cov report
                $<TARGET_FILE:${TEST_TARGET_NAME}>
                -instr-profile=${COVERAGE_PROFDATA_FILE_LOC}
                -Xdemangler c++filt
                -Xdemangler -n
                ${ARGN}
                DEPENDS ${COVERAGE_NAME}_preproc
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )

        add_custom_target(${COVERAGE_NAME}
                COMMAND llvm-cov show
                $<TARGET_FILE:${TEST_TARGET_NAME}>
                -instr-profile=${COVERAGE_PROFDATA_FILE_LOC}
                -show-line-counts-or-regions
                -output-dir=${COVERAGE_LOCATION}
                -format="html"
                -Xdemangler c++filt
                -Xdemangler -n
                ${ARGN}
                DEPENDS ${COVERAGE_NAME}_preproc
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
    elseif (${CMAKE_COMPILER_IS_GNUCXX})
        add_custom_target(${COVERAGE_NAME}_preproc
                COMMAND mkdir -p ${COVERAGE_LOCATION}
                COMMAND $<TARGET_FILE:${TEST_TARGET_NAME}>
                COMMAND lcov -c
                --base-directory .
                --directory $<TARGET_FILE_DIR:${TEST_TARGET_NAME}>
                --directory $<TARGET_FILE_DIR:Json>
                --directory $<TARGET_FILE_DIR:DateTime>
                --output-file ${COVERAGE_LOCATION}/coverage.info --rc lcov_branch_coverage=1
                COMMAND lcov --remove ${COVERAGE_LOCATION}/coverage.info --output-file ${COVERAGE_LOCATION}/coverage.info */extern/* *8.3.0* *MacOSX.sdk* tests/*
                DEPENDS ${TEST_TARGET_NAME}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                )

        add_custom_target(${COVERAGE_NAME}_report
                COMMAND lcov --summary ${COVERAGE_LOCATION}/coverage.info --rc lcov_branch_coverage=1
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${COVERAGE_NAME}_preproc
                )

        add_custom_target(${COVERAGE_NAME}
                COMMAND genhtml ${COVERAGE_LOCATION}/coverage.info --demangle-cpp --output-directory ${COVERAGE_LOCATION}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${COVERAGE_NAME}_preproc
                )
    endif ()
endfunction()

package_add_test(Test_MessageQueue MessageQueue_test.cpp)
target_link_libraries(Test_MessageQueue MessageQueue Utils)

package_add_test(Test_Utils Utils_test.cpp)
target_link_libraries(Test_Utils Utils)

package_add_test(Test_ThreadPool ThreadPool_test.cpp)
target_link_libraries(Test_ThreadPool ThreadPool)

package_add_test(Test_Json Json_test.cpp)
target_link_libraries(Test_Json Json)

package_add_test(Test_DateTime Time_test.cpp Duration_test.cpp)
target_link_libraries(Test_DateTime DateTime)

package_add_test(Test_FileManager FileManager_test.cpp)
target_link_libraries(Test_FileManager FileManager)

package_add_test(Test_Graph BinaryTree_test.cpp BinarySearchTree_test.cpp)
target_link_libraries(Test_Graph Graph)

package_add_test(Test_Core Utils_test.cpp MessageQueue_test.cpp ThreadPool_test.cpp Json_test.cpp Time_test.cpp Duration_test.cpp FileManager_test.cpp BinaryTree_test.cpp BinarySearchTree_test.cpp)
target_link_libraries(Test_Core Json Utils ThreadPool MessageQueue Utils DateTime FileManager Graph)

if (${CREATE_COVERAGE_REPORT})
    if ("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        set(COMPILER_IS_CLANG 1)
    else ()
        set(COMPILER_IS_CLANG 0)
    endif ()

    if (${COMPILER_IS_CLANG})
        message("-- Coverage with llvm Code Coverage Tools")
    elseif (${CMAKE_COMPILER_IS_GNUCXX})
        message("-- Coverage with GNU Code Coverage Tools")
    endif ()

    generate_overall_coverage_report(
            ${Json_FILES} ${Json_SRC_FILES}
            ${DateTime_FILES} ${DateTime_SRC_FILES}
            ${MessageQueue_FILES}
            ${ThreadPool_FILES}
            ${Utils_FILES}
            ${FileManager_FILES} ${FileManager_SRC_FILES}
    )
endif ()

