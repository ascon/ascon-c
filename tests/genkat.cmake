# setup KAT file name
if(${ALG} STREQUAL ascon128v12 OR ${ALG} STREQUAL ascon128av12 OR
   ${ALG} STREQUAL ascon128bi32v12 OR ${ALG} STREQUAL ascon128abi32v12)
  set(KAT_PATH crypto_aead/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_128_128.txt)
elseif(${ALG} STREQUAL ascon80pqv12)
  set(KAT_PATH crypto_aead/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_160_128.txt)
elseif(${ALG} STREQUAL asconhashv12 OR ${ALG} STREQUAL asconhashav12 OR
    ${ALG} STREQUAL asconhashbi32v12 OR ${ALG} STREQUAL asconhashabi32v12 OR
    ${ALG} STREQUAL asconxofv12 OR ${ALG} STREQUAL asconxofav12)
  set(KAT_PATH crypto_hash/${ALG})
  set(KAT_FILE LWC_HASH_KAT_256.txt)
elseif((${ALG} STREQUAL asconv12 OR ${ALG} STREQUAL asconav12 OR
        ${ALG} STREQUAL asconbi32v12 OR ${ALG} STREQUAL asconabi32v12) AND
    ${CRYPTO} STREQUAL aead)
  set(KAT_PATH crypto_aead_hash/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_128_128.txt)
elseif((${ALG} STREQUAL asconv12 OR ${ALG} STREQUAL asconav12 OR
        ${ALG} STREQUAL asconbi32v12 OR ${ALG} STREQUAL asconabi32v12) AND
    ${CRYPTO} STREQUAL hash)
  set(KAT_PATH crypto_aead_hash/${ALG})
  set(KAT_FILE LWC_HASH_KAT_256.txt)
elseif(${ALG} STREQUAL asconmacv12 OR ${ALG} STREQUAL asconmacav12 OR
       ${ALG} STREQUAL asconprfv12 OR ${ALG} STREQUAL asconprfav12 OR
       ${ALG} STREQUAL asconprfsv12)
  set(KAT_PATH crypto_auth/${ALG})
  set(KAT_FILE LWC_AUTH_KAT_128_128.txt)
else()
  message(FATAL_ERROR "KAT file name not defined for algorithm ${ALG}.")
endif()

# prepend emulator and add config
if(EXISTS ${BIN_DIR}/${CONFIG})
  set(CMD "${EMULATOR} ${BIN_DIR}/${CONFIG}/${EXE_NAME}")
else()
  set(CMD "${EMULATOR} ${BIN_DIR}/${EXE_NAME}")
endif()

# first entry is command (emulator or native command)
separate_arguments(CMDLIST NATIVE_COMMAND ${CMD})
list(GET CMDLIST 0 CMD)
list(REMOVE_AT CMDLIST 0)

# remove previous and generate new KAT file
file(REMOVE ${BIN_DIR}/${KAT_FILE})
execute_process(COMMAND ${CMD} ${CMDLIST})
configure_file(${BIN_DIR}/${KAT_FILE} ${BIN_DIR}/${KAT_FILE} NEWLINE_STYLE LF)

# compare KAT files
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol
  ${BIN_DIR}/${KAT_FILE} ${SRC_DIR}/${KAT_PATH}/${KAT_FILE}
  RESULT_VARIABLE COMPARE_RESULT)
if(${COMPARE_RESULT})
  message(FATAL_ERROR "KAT files are not identical.")
endif()
