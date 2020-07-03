cmake_minimum_required(VERSION 3.6)
project(ascon LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()
set(TEST_PATH tests)
set(KAT_PATH KAT)

if(NOT MSVC)
  # PERF_FLAGS are used for CMAKE_BUILD_TYPE=Release
  set(PERF_FLAGS -O3 -funroll-loops -fomit-frame-pointer -march=native -mtune=native)
  # NIST_FLAGS are used for CMAKE_BUILD_TYPE=Debug
  set(NIST_FLAGS -std=c99 -Wall -Wextra -Wshadow -O2)
endif()
if(NOT WIN32 AND NOT CYGWIN AND NOT MSYS)
  # do not use sanitizer on windows
  set(NIST_FLAGS ${NIST_FLAGS} -fsanitize=address,undefined -static-libasan)
endif()

# set the default build type for single-config generators if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# set the default version, algorithms, implementations and tests to build
set(DEFAULT_VERSIONS v12)
set(DEFAULT_ALGS ascon128 ascon128a ascon80pq asconhash asconxof)
set(DEFAULT_IMPLS ref opt64 opt64_lowsize bi32 bi32_lowsize bi32_lowreg bi16 bi8)
set(DEFAULT_TESTS genkat getcycles)

# add platform specific implementations
message(STATUS "cmake host system name: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "cmake host system processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL x86_64 OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL AMD64)
  set(DEFAULT_IMPLS ${DEFAULT_IMPLS})
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES ARM OR ${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES arm)
  set(DEFAULT_IMPLS ${DEFAULT_IMPLS} bi32_arm neon)
endif()

set(VERSION_LIST ${DEFAULT_VERSIONS} CACHE STRING "Choose the ascon versions to include.")
set(ALG_LIST ${DEFAULT_ALGS} CACHE STRING "Choose the list of algorithms to include.")
set(IMPL_LIST ${DEFAULT_IMPLS} CACHE STRING "Choose the list of implementations to include.")
set(TEST_LIST ${DEFAULT_TESTS} CACHE STRING "Choose the list of tests to include.")

foreach(CRYPTO aead hash)
  foreach(VER ${VERSION_LIST})
    foreach(ALG ${ALG_LIST})
      foreach(IMPL ${IMPL_LIST})
        set(IMPL_PATH crypto_${CRYPTO}/${ALG}${VER}/${IMPL})
        if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${IMPL_PATH})
          continue()
        endif()
        message("Adding implementation ${IMPL_PATH}")
        string(REGEX REPLACE "/" "_" IMPL_NAME ${IMPL_PATH})
        file(GLOB IMPL_FILES RELATIVE ${CMAKE_SOURCE_DIR} "${IMPL_PATH}/*.[ch]")
        add_library(${IMPL_NAME} ${IMPL_FILES})
        target_include_directories(${IMPL_NAME} PRIVATE ${IMPL_PATH} ${TEST_PATH})
        target_compile_options(${IMPL_NAME} PRIVATE $<$<CONFIG:RELEASE>:${PERF_FLAGS}>)
        target_compile_options(${IMPL_NAME} PRIVATE $<$<CONFIG:DEBUG>:${NIST_FLAGS}>)
        foreach(TEST_NAME ${TEST_LIST})
          if(${TEST_NAME} STREQUAL genkat)
            set(TEST_FILES ${TEST_PATH}/crypto_${CRYPTO}.h ${TEST_PATH}/${TEST_NAME}_${CRYPTO}.c)
          else()
            set(TEST_FILES ${TEST_PATH}/crypto_${CRYPTO}.h ${TEST_PATH}/${TEST_NAME}.c)
          endif()
          string(TOUPPER CRYPTO_${CRYPTO} CRYPTO_DEFINE)
          set(EXE_NAME ${TEST_NAME}_${IMPL_NAME})
          add_executable(${EXE_NAME} ${TEST_FILES})
          target_compile_definitions(${EXE_NAME} PRIVATE ${CRYPTO_DEFINE})
          target_include_directories(${EXE_NAME} PRIVATE ${IMPL_PATH} ${TEST_PATH})
          target_compile_options(${EXE_NAME} PRIVATE $<$<CONFIG:RELEASE>:${PERF_FLAGS}>)
          target_compile_options(${EXE_NAME} PRIVATE $<$<CONFIG:DEBUG>:${NIST_FLAGS}>)
          target_link_libraries(${EXE_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${NIST_FLAGS}>" ${IMPL_NAME})
          if(${TEST_NAME} STREQUAL genkat)
            add_test(NAME ${EXE_NAME} COMMAND ${CMAKE_COMMAND}
              -DEXE_NAME=${EXE_NAME} -DALG=${ALG}${VER}
              -DSRC_DIR=${CMAKE_SOURCE_DIR} -DBIN_DIR=${CMAKE_BINARY_DIR}
              -DCONFIG=$<CONFIGURATION> -P ${CMAKE_SOURCE_DIR}/genkat.cmake)
          else()
            add_test(${EXE_NAME} ${EXE_NAME})
          endif()
        endforeach()
      endforeach()
    endforeach()
  endforeach()
endforeach()

