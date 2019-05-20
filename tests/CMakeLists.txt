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

macro(generate_coverage_report TARGET_NAME)
    if(${COMPILER_IS_CLANG})
        set(TARGET_PROFRAW_FILE_LOC ${CMAKE_BINARY_DIR}/${TARGET_NAME}.profraw)
        set(TARGET_PROFDATA_FILE_LOC ${CMAKE_BINARY_DIR}/${TARGET_NAME}.profdata)

        add_custom_target(${TARGET_NAME}_coverage_preproc
            COMMAND mkdir -p coverage
            COMMAND LLVM_PROFILE_FILE=${TARGET_NAME}.profraw $<TARGET_FILE:Test_${TARGET_NAME}>
            COMMAND llvm-profdata merge -sparse ${TARGET_PROFRAW_FILE_LOC} -o ${TARGET_NAME}.profdata
            DEPENDS Test_${TARGET_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(${TARGET_NAME}_coverage_show
            COMMAND llvm-cov show
                $<TARGET_FILE:Test_${TARGET_NAME}>
                    -instr-profile=${TARGET_PROFDATA_FILE_LOC}
                    -show-line-counts-or-regions
                    -Xdemangler c++filt
                    -Xdemangler -n
                ${ARGN}
            DEPENDS ${TARGET_NAME}_coverage_preproc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )

        add_custom_target(${TARGET_NAME}_coverage_report
            COMMAND llvm-cov report
                $<TARGET_FILE:Test_${TARGET_NAME}>
                    -instr-profile=${TARGET_PROFDATA_FILE_LOC}
                    -Xdemangler c++filt
                    -Xdemangler -n
                ${ARGN}
            DEPENDS ${TARGET_NAME}_coverage_preproc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )

        add_custom_target(${TARGET_NAME}_coverage
            COMMAND llvm-cov show
                $<TARGET_FILE:Test_${TARGET_NAME}>
                    -instr-profile=${TARGET_PROFDATA_FILE_LOC}
                    -show-line-counts-or-regions
                    -output-dir=${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-coverage
                    -format="html"
                    -Xdemangler c++filt
                    -Xdemangler -n
                ${ARGN}
            DEPENDS ${TARGET_NAME}_coverage_preproc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    elseif(${CMAKE_COMPILER_IS_GNUCXX})
        string(REGEX REPLACE "/([a-zA-Z]|-)*" "/" COUNT_OF_SEPARATORS ${CMAKE_CURRENT_BINARY_DIR})
        string(LENGTH ${COUNT_OF_SEPARATORS} COUNT_OF_SEPARATORS)

        add_custom_target(${TARGET_NAME}_coverage_preproc
                COMMAND mkdir -p coverage
                COMMAND GCOV_PREFIX=${CMAKE_BINARY_DIR}/${TARGET_NAME}-coverage GCOV_PREFIX_STRIP=${COUNT_OF_SEPARATORS} $<TARGET_FILE:Test_${TARGET_NAME}>
                COMMAND mv ${TARGET_NAME}-coverage/*/**/*.gcda ${TARGET_NAME}-coverage/
                COMMAND cd ${TARGET_NAME}-coverage
                COMMAND ls | grep -v .gcda | xargs rm -r
                DEPENDS Test_${TARGET_NAME}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(gcov-show
                COMMAND gcov-8 -b ${ARGN} -o ${CMAKE_BINARY_DIR}/${TARGET_NAME}-coverage/*/**/*.gcda
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${TARGET_NAME}_coverage_preproc
        )
    endif()
endmacro()

package_add_test(Test_MessageQueue MessageQueue_test.cpp)
target_link_libraries(Test_MessageQueue MessageQueue Utils)

package_add_test(Test_ThreadPool ThreadPool_test.cpp)
target_link_libraries(Test_ThreadPool ThreadPool)

package_add_test(Test_Json Json_test.cpp)
target_link_libraries(Test_Json Json)

if(${CREATE_COVERAGE_REPORT})
    if("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        set(COMPILER_IS_CLANG 1)
    else()
        set(COMPILER_IS_CLANG 0)
    endif()

    if(${COMPILER_IS_CLANG})
        message(" ==== COVERAGE WITH llvm Code Coverage Tools ====")
    elseif(${CMAKE_COMPILER_IS_GNUCXX})
        message(" ==== COVERAGE WITH GNU Code Coverage Tools ====")
    endif()

    generate_coverage_report(Json ${Json_FILES} ${Json_SRC_FILES})

endif()
