cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_FLAGS "-Wall -pedantic")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen dia)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest)

project(Core)

add_library(Core STATIC
            src/ThreadPool.cpp
            include/ThreadPool.hpp
            include/MessageQueue.hpp
            include/TestUtil.hpp
)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#set_target_properties(Core
#        PROPERTIES
#            FRAMEWORK
#            VERSION
#                ${PROJECT_VERSION}
#            PUBLIC_HEADER
#                include/MessageQueue.hpp
#            SOVERSION
#                1.0.0)

configure_file(Core.pc.in Core.pc @ONLY)

target_include_directories(Core PRIVATE include)

install(TARGETS Core 
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(DOXYGEN_IN Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

message("-- Doxygen build started")
add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)


