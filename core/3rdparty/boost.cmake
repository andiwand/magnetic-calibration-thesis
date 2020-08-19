include(FetchContent)

FetchContent_Declare(
        boost
        URL https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.bz2
)

FetchContent_GetProperties(boost)
if(NOT Boost_FOUND AND NOT boost_POPULATED)
    FetchContent_Populate(boost)

    # The following needs to be run after initial checkout or
    # after the dependency details of boost are changed.
    # The following option is provided to prevent having to
    # keep reconfiguring and rerunning the boost build each
    # time CMake runs once it has been successfully performed
    # for the version specified.
    option(ENABLE_BOOST_BUILD "Enable reconfiguring and rerunning the boost build" ON)
    if(ENABLE_BOOST_BUILD)
        # This file comes from the following location:
        #   https://github.com/pfultz2/cget/blob/master/cget/cmake/boost.cmake
        configure_file(boost_extern.cmake
                ${boost_SOURCE_DIR}/CMakeLists.txt
                COPYONLY
                )

        unset(generatorArgs)
        set(cacheArgs
                "-DCMAKE_INSTALL_PREFIX:STRING=${boost_BINARY_DIR}/install"
                "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=YES"
                "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
                )
        if(CMAKE_TOOLCHAIN_FILE)
            list(APPEND cacheArgs "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}")
        else()
            list(APPEND cacheArgs "-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}"
                    "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
                    )
        endif()

        get_property(isMulti GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(NOT isMulti)
            list(APPEND cacheArgs "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
        endif()

        if(CMAKE_GENERATOR_PLATFORM)
            list(APPEND generatorArgs
                    --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
                    )
        endif()
        if(CMAKE_GENERATOR_TOOLSET)
            list(APPEND generatorArgs
                    --build-generator-toolset  "${CMAKE_GENERATOR_TOOLSET}"
                    )
        endif()

        set(BOOST_LIBS headers system thread atomic chrono date_time regex serialization random exception)
        foreach(lib IN LISTS BOOST_LIBS)
            list(APPEND cacheArgs "-DBOOST_WITH_${lib}:STRING=--with-${lib}")
        endforeach()

        message(STATUS "Configuring and building boost immediately")
        execute_process(
                COMMAND ${CMAKE_CTEST_COMMAND}
                --build-and-test  ${boost_SOURCE_DIR} ${boost_BINARY_DIR}
                --build-generator ${CMAKE_GENERATOR} ${generatorArgs}
                --build-target    install
                --build-noclean
                --build-options   ${cacheArgs}
                WORKING_DIRECTORY ${boost_SOURCE_DIR}
                OUTPUT_FILE       ${boost_BINARY_DIR}/build_output.log
                ERROR_FILE        ${boost_BINARY_DIR}/build_output.log
                RESULT_VARIABLE   result
        )
        message(STATUS "boost build complete")
        if(result)
            message(FATAL_ERROR "Failed boost build, see build log at:\n"
                    "    ${boost_BINARY_DIR}/build_output.log")
        endif()

        set(ENABLE_BOOST_BUILD OFF CACHE BOOL "" FORCE)
    endif()

    list(APPEND CMAKE_FIND_ROOT_PATH ${boost_BINARY_DIR}/install)
    find_package(Boost 1.73.0 REQUIRED)
endif()
