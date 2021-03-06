cmake_minimum_required(VERSION 3.6)
project(ascon LANGUAGES C ASM)
enable_testing()

# set the default version, algorithms, implementations, tests, flags, defs
set(DEFAULT_VERSIONS v12)
set(DEFAULT_ALGS ascon128 ascon128a ascon80pq asconhash asconxof
  asconhasha asconxofa ascon ascona)
set(DEFAULT_IMPLS ref opt64 opt64_lowsize bi32 bi32_lowsize bi32_lowreg opt8 bi8)
set(DEFAULT_TESTS genkat getcycles)
set(DEFAULT_COMPILE_DEFS)

if(MSVC)
  set(DEFAULT_REL_FLAGS /O2)
  set(DEFAULT_DBG_FLAGS /Od)
else()
  set(DEFAULT_REL_FLAGS -std=c99 -O2 -march=native -mtune=native -fomit-frame-pointer)
  set(DEFAULT_DBG_FLAGS -std=c99 -O2 -Wall -Wextra -Wshadow)
endif()

# set cmake variables for version, algorithms, implementations, tests, flags, defs
set(VERSION_LIST ${DEFAULT_VERSIONS} CACHE STRING "Choose the ascon versions to include.")
set(ALG_LIST ${DEFAULT_ALGS} CACHE STRING "Choose the list of algorithms to include.")
set(IMPL_LIST ${DEFAULT_IMPLS} CACHE STRING "Choose the list of implementations to include.")
set(TEST_LIST ${DEFAULT_TESTS} CACHE STRING "Choose the list of tests to include.")
set(REL_FLAGS ${DEFAULT_REL_FLAGS} CACHE STRING "Define custom Release (performance) flags.")
set(DBG_FLAGS ${DEFAULT_DBG_FLAGS} CACHE STRING "Define custom Debug (NIST) flags.")
set(COMPILE_DEFS ${DEFAULT_COMPILE_DEFS} CACHE STRING "Define custom compile definitions.")

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0" AND NOT WIN32 AND NOT CYGWIN AND NOT MSYS)
  # use sanitizer in Debug build (not on windows and only of target_link_option is available)
  set(DBG_FLAGS ${DBG_FLAGS} -fsanitize=address,undefined)
endif()

# set the default build type for single-config generators if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# add platform specific implementations
message(STATUS "cmake host system name: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "cmake host system processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL x86_64 OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL AMD64)
  set(DEFAULT_IMPLS ${DEFAULT_IMPLS})
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES ARM OR ${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES arm)
  set(DEFAULT_IMPLS ${DEFAULT_IMPLS} bi32_arm neon)
endif()

set(KAT_PATH KAT)
set(TEST_PATH tests)
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
        target_include_directories(${IMPL_NAME} PUBLIC ${IMPL_PATH} ${TEST_PATH})
        target_compile_definitions(${IMPL_NAME} PRIVATE ${COMPILE_DEFS})
        #target_compile_features(${IMPL_NAME} PUBLIC c_std_99) # cmake >= 3.8.2
        target_compile_options(${IMPL_NAME} PUBLIC $<$<CONFIG:RELEASE>:${REL_FLAGS}>)
        target_compile_options(${IMPL_NAME} PUBLIC $<$<CONFIG:DEBUG>:${DBG_FLAGS}>)
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
          if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
            target_link_options(${EXE_NAME} PRIVATE $<$<CONFIG:DEBUG>:${DBG_FLAGS}>)
          endif()
          target_link_libraries(${EXE_NAME} PRIVATE ${IMPL_NAME})
          if(${TEST_NAME} STREQUAL genkat)
            add_test(NAME ${EXE_NAME} COMMAND ${CMAKE_COMMAND}
              -DEXE_NAME=${EXE_NAME} -DALG=${ALG}${VER} -DCRYPTO=${CRYPTO}
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

