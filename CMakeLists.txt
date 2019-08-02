cmake_minimum_required(VERSION 3.8)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Core)
set(COMMON_COMPILE_OPTIONS  -Wall -pedantic -pedantic-errors -Werror -Wextra)
set(COMMON_COMPILE_FEATURES cxx_std_17)

macro(add_common_compiler_options TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:DEBUG>:-O0>
            $<$<CONFIG:RELEASE>:-O3>
            $<$<BOOL:${CREATE_COVERAGE_REPORT}>:-O0>
            $<$<AND:$<BOOL:${CREATE_COVERAGE_REPORT}>,$<CXX_COMPILER_ID:GNU>>:--coverage>
            $<$<AND:$<BOOL:${CREATE_COVERAGE_REPORT}>,$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>>:-fprofile-instr-generate -fcoverage-mapping>
    )
endmacro()

# Project name
project(Core VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

### Target Core::Utils
add_library(Utils INTERFACE)

# We want this to be included with the #include <Core/{...}>
target_include_directories(Utils INTERFACE
        $<INSTALL_INTERFACE:include>
)

list(APPEND Utils_FILES
        include/Utils/TestUtil.hpp
        include/Utils/Utils.hpp
        include/Utils/ValueWrapper.hpp
)

# Until this has more files this is installed as a single header
list(APPEND Json_FILES
        include/Json/Json.hpp
)

list(APPEND Json_SRC_FILES
        src/Json/Json.cpp
)

### Target Core::Json
add_library(Json STATIC
        ${Json_FILES}
        ${Json_SRC_FILES}
)

# Compile options
add_common_compiler_options(Json)

target_compile_features(Json PUBLIC ${COMMON_COMPILE_FEATURES})

set_target_properties(Json PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
)

# Include dirs
target_include_directories(Json
        INTERFACE
            $<INSTALL_INTERFACE:include>
            $<INSTALL_INTERFACE:include/Core>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Json>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# gcov works for clang as well
target_link_libraries(Json PUBLIC Utils
    $<$<AND:$<BOOL:${CREATE_COVERAGE_REPORT}>,$<CXX_COMPILER_ID:GNU>>:gcov>)

### Target Core::DateTime
list(APPEND DateTime_FILES
        include/DateTime/Date.hpp
        include/DateTime/DateTime.hpp
        include/DateTime/Time.hpp
        include/DateTime/Duration.hpp
)

list(APPEND DateTime_SRC_FILES
        src/DateTime/Time.cpp
)

add_library(DateTime STATIC
        ${DateTime_FILES}
        ${DateTime_SRC_FILES}
)

add_common_compiler_options(DateTime)

target_compile_features(DateTime PUBLIC ${COMMON_COMPILE_FEATURES})

set_target_properties(DateTime PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
)

# Include dirs
target_include_directories(DateTime
        INTERFACE
            $<INSTALL_INTERFACE:include>
            $<INSTALL_INTERFACE:include/Core>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/DateTime>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

### Target Core::FileManager
list(APPEND FileManager_FILES
        include/FileManager/FileManager.hpp
        include/FileManager/Exceptions.hpp
        include/FileManager/FileBase.hpp
        include/FileManager/TextFile.hpp
        include/FileManager/BinaryFile.hpp
)

list(APPEND FileManager_SRC_FILES
        src/FileManager/FileManager.cpp
        src/FileManager/FileBase.cpp
        src/FileManager/TextFile.cpp
        src/FileManager/BinaryFile.cpp
)

add_library(FileManager STATIC
        ${FileManager_FILES}
        ${FileManager_SRC_FILES}
)

add_common_compiler_options(FileManager)

target_compile_features(FileManager PUBLIC ${COMMON_COMPILE_FEATURES})

target_link_libraries(FileManager PUBLIC $<$<CXX_COMPILER_ID:GNU>:stdc++fs>)

set_target_properties(FileManager PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
)

target_include_directories(FileManager
        INTERFACE
            $<INSTALL_INTERFACE:include/Core>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/FileManager>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

### Target Core::ThreadPool
add_library(ThreadPool INTERFACE)

# Until this has more files this is installed as a single header
list(APPEND ThreadPool_FILES
        include/ThreadPool/ThreadPool.hpp
)

target_include_directories(ThreadPool
        INTERFACE
            $<INSTALL_INTERFACE:include>
)

### Target Core::MessageQueue
add_library(MessageQueue INTERFACE)

# Until this has more files this is installed as a single header
list(APPEND MessageQueue_FILES
        include/MessageQueue/MessageQueue.hpp
)

target_include_directories(MessageQueue
        INTERFACE
            $<INSTALL_INTERFACE:include>
)

# Install steps
install(TARGETS Utils Json ThreadPool MessageQueue FileManager
        EXPORT core-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${Json_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(DIRECTORY include/Utils  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(FILES ${ThreadPool_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)
install(FILES ${MessageQueue_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Core)

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

#export(PACKAGE Json)
#export(PACKAGE Utils)
#export(PACKAGE ThreadPool)
#export(PACKAGE MessageQueue)
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

