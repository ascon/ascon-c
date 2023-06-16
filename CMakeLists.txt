cmake_minimum_required(VERSION 3.9)
project(ascon LANGUAGES C ASM)
enable_testing()

# set the default version
set(DEFAULT_VERSIONS v12)

# default to all Ascon algorithms:
set(DEFAULT_ALGS ascon128 ascon128a ascon80pq
  asconhash asconhasha asconxof asconxofa
  asconmac asconmaca asconprf asconprfa asconprfs
  ascon128bi32 ascon128abi32 asconhashbi32 asconhashabi32
  ascon ascona asconabi32 asconbi32)

# default to all C implementations:
set(DEFAULT_IMPLS ref opt64 opt64_lowsize opt32 opt32_lowsize bi32 bi32_lowsize bi32_lowreg esp32 opt8 opt8_lowsize bi8)

# default flags, tests {genkat, getcycles}, emulator {"qemu-arm;-L;/usr/arm-linux-gnueabi", "sde;--"}
set(DEFAULT_COMPILE_DEFS)
set(DEFAULT_TESTS genkat)
set(DEFAULT_EMULATOR)

if(MSVC)
  set(DEFAULT_REL_FLAGS /O2)
  set(DEFAULT_DBG_FLAGS /Od)
else()
  set(DEFAULT_REL_FLAGS -std=c99 -O2 -fomit-frame-pointer -march=native -mtune=native)
  set(DEFAULT_DBG_FLAGS -std=c99 -O2 -Wall -Wextra -Wshadow)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0" AND NOT WIN32 AND NOT CYGWIN AND NOT MSYS)
  # use sanitizers in default Debug build (not on windows and only of target_link_option is available)
  set(DEFAULT_DBG_FLAGS ${DEFAULT_DBG_FLAGS} -fsanitize=address,undefined)
endif()

# set cmake variables for version, algorithms, implementations, tests, flags, defs
set(VERSION_LIST ${DEFAULT_VERSIONS} CACHE STRING "Choose the ascon versions to include.")
set(ALG_LIST ${DEFAULT_ALGS} CACHE STRING "Choose the list of algorithms to include.")
set(IMPL_LIST ${DEFAULT_IMPLS} CACHE STRING "Choose the list of implementations to include.")
set(TEST_LIST ${DEFAULT_TESTS} CACHE STRING "Choose the list of tests to include.")
set(REL_FLAGS ${DEFAULT_REL_FLAGS} CACHE STRING "Define custom Release (performance) flags.")
set(DBG_FLAGS ${DEFAULT_DBG_FLAGS} CACHE STRING "Define custom Debug (NIST) flags.")
set(COMPILE_DEFS ${DEFAULT_COMPILE_DEFS} CACHE STRING "Define custom compile definitions.")
set(EMULATOR ${DEFAULT_EMULATOR} CACHE STRING "Define custom emulator command.")

# set the default build type for single-config generators if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# add platform specific implementations
message(STATUS "cmake host system name: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "cmake host system processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(KAT_PATH KAT)
set(TEST_PATH tests)
foreach(CRYPTO aead hash auth)
  foreach(VER ${VERSION_LIST})
    foreach(ALG ${ALG_LIST})
      foreach(IMPL ${IMPL_LIST})
        set(IMPL_PATH crypto_${CRYPTO}/${ALG}${VER}/${IMPL})
        if((NOT ${CRYPTO} STREQUAL auth) AND
            NOT EXISTS ${CMAKE_SOURCE_DIR}/${IMPL_PATH})
          set(IMPL_PATH crypto_aead_hash/${ALG}${VER}/${IMPL})
        endif()
        if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${IMPL_PATH})
          continue()
        endif()
        message("Adding implementation ${IMPL_PATH}")
        set(IMPL_NAME crypto_${CRYPTO}_${ALG}${VER}_${IMPL})
        file(GLOB IMPL_FILES RELATIVE ${CMAKE_SOURCE_DIR} "${IMPL_PATH}/*.[chS]")
        if(${IMPL} MATCHES protected.*)
          set(IMPL_FILES ${IMPL_FILES} ${TEST_PATH}/randombytes.h)
        endif()
        add_library(${IMPL_NAME} ${IMPL_FILES})
        target_include_directories(${IMPL_NAME} PUBLIC ${IMPL_PATH} ${TEST_PATH})
        target_compile_definitions(${IMPL_NAME} PRIVATE ${COMPILE_DEFS})
        #target_compile_features(${IMPL_NAME} PUBLIC c_std_99) # cmake >= 3.8.2
        target_compile_options(${IMPL_NAME} PUBLIC $<$<CONFIG:RELEASE>:${REL_FLAGS}>)
        target_compile_options(${IMPL_NAME} PUBLIC $<$<CONFIG:DEBUG>:${DBG_FLAGS}>)
        target_link_libraries(${IMPL_NAME} PRIVATE $<$<CONFIG:DEBUG>:atomic>)
        foreach(TEST_NAME ${TEST_LIST})
          if(${TEST_NAME} STREQUAL genkat)
            set(TEST_FILES ${TEST_PATH}/crypto_${CRYPTO}.h ${TEST_PATH}/genkat_${CRYPTO}.c)
          else()
            set(TEST_FILES ${TEST_PATH}/crypto_${CRYPTO}.h ${TEST_PATH}/getcycles.c)
          endif()
          string(TOUPPER CRYPTO_${CRYPTO} DEFINE_CRYPTO)
          if(${IMPL} MATCHES protected.*)
            set(DEFINE_CRYPTO ${DEFINE_CRYPTO}_SHARED)
          endif()
          if(${ALG} STREQUAL asconprfs)
            set(DEFINE_MAXMSGLEN "MAX_DATA_LENGTH=16")
          else()
            set(DEFINE_MAXMSGLEN "MAX_DATA_LENGTH=1024")
          endif()
          set(EXE_NAME ${TEST_NAME}_${IMPL_NAME})
          add_executable(${EXE_NAME} ${TEST_FILES})
          target_compile_definitions(${EXE_NAME} PRIVATE ${DEFINE_CRYPTO} ${DEFINE_MAXMSGLEN})
          if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
            target_link_options(${EXE_NAME} PRIVATE $<$<CONFIG:DEBUG>:${DBG_FLAGS}>)
          endif()
          target_link_libraries(${EXE_NAME} PRIVATE ${IMPL_NAME})
          string(REPLACE ";" " " EMULATOR_STRING "${EMULATOR}")
          if(${TEST_NAME} STREQUAL genkat)
            add_test(NAME ${EXE_NAME} COMMAND ${CMAKE_COMMAND}
              -DEXE_NAME=${EXE_NAME} -DALG=${ALG}${VER} -DCRYPTO=${CRYPTO}
              -DSRC_DIR=${CMAKE_SOURCE_DIR} -DBIN_DIR=${CMAKE_BINARY_DIR}
              -DEMULATOR=${EMULATOR_STRING} -DCONFIG=$<CONFIGURATION>
              -P ${CMAKE_SOURCE_DIR}/genkat.cmake)
          else()
            add_test(${EXE_NAME} ${EXE_NAME})
          endif()
        endforeach()
      endforeach()
    endforeach()
  endforeach()
endforeach()

