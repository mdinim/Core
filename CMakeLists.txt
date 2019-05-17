cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Core)
set(COMMON_COMPILE_OPTIONS  -Wall -pedantic -Werror -Wextra)
set(COMMON_COMPILE_FEATURES cxx_std_17)

# Project name
project(Core VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

### Target Core::Utils
add_library(Utils INTERFACE)

# Until this has more files this is installed as a single header
list(APPEND CoreUtils_FILES
        include/Utils/Utils.hpp
)

# We want this to be included with the #include <Core/{...}>
target_include_directories(Utils INTERFACE
        $<INSTALL_INTERFACE:include>
)

### Target Core::Json
add_library(Json STATIC
        include/Json/Json.hpp
        src/Json/Json.cpp
        src/Json/ValueWrapper.hpp
)

# Compile options
target_compile_options(Json PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_features(Json PUBLIC ${COMMON_COMPILE_FEATURES})

set_target_properties(Json PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
)

# Include dirs
target_include_directories(Json
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Json>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Utils>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/Json
)

target_link_libraries(Json PRIVATE Utils)

### Target Core::ThreadPool
add_library(ThreadPool INTERFACE)

# Until this has more files this is installed as a single header
list(APPEND CoreThreadPool_FILES
        include/ThreadPool/ThreadPool.hpp
)

target_include_directories(ThreadPool
        INTERFACE
            $<INSTALL_INTERFACE:include>
)

### Target Core::MessageQueue
add_library(MessageQueue INTERFACE)

# Until this has more files this is installed as a single header
list(APPEND CoreMessagePool_FILES
        include/MessageQueue/MessageQueue.hpp
)

target_include_directories(MessageQueue
        INTERFACE
            $<INSTALL_INTERFACE:include>
)

# Install steps
install(TARGETS Utils Json ThreadPool MessageQueue
        EXPORT core-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/Json DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(FILES ${CoreUtils_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(FILES ${CoreThreadPool_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(FILES ${CoreMessageQueue_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)

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

export(PACKAGE Json)
export(PACKAGE Utils)
export(PACKAGE ThreadPool)
export(PACKAGE MessageQueue)

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


