cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Core)

# Project name
project(Core VERSION 0.0.1 LANGUAGES CXX)

# Single target currently
add_library(Core STATIC
        include/Json/Json.hpp
        src/Json/Json.cpp
        src/Json/ValueWrapper.hpp

        include/ThreadPool.hpp
        include/Utils.hpp
        include/TestUtil.hpp

        include/MessageQueue.hpp
)

# Compile options
target_compile_options(Core PRIVATE -Wall -pedantic -Werror -Wextra)
target_compile_features(Core PUBLIC cxx_std_17)

# Include dirs
target_include_directories(Core
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

# Install steps
install(TARGETS Core
        EXPORT core-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets
install(EXPORT core-targets
        FILE
        CoreTargets.cmake
        NAMESPACE
        Core::
        DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/Core
        )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/CoreConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/CoreConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/CoreConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT core-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/CoreTargets.cmake NAMESPACE Core::)

export(PACKAGE Core)

configure_file(Core.pc.in Core.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/CoreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/CoreConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest)

    enable_testing()
    add_subdirectory(tests)
endif()

if(CREATE_DOCS)
    find_package(Doxygen
            REQUIRED dot
            OPTIONAL_COMPONENTS mscgen dia)

    set(DOXYGEN_IN Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    message("-- Doxygen build started")
    add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
endif()


