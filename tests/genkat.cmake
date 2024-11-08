# setup KAT file name
if(${ALG} STREQUAL ascon128av13)
  set(KAT_PATH crypto_aead/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_128_128.txt)
elseif(${ALG} STREQUAL asconhashv13 OR
    ${ALG} STREQUAL asconxofv13)
  set(KAT_PATH crypto_hash/${ALG})
  set(KAT_FILE LWC_HASH_KAT_256.txt)
elseif((${ALG} STREQUAL asconav13) AND
    ${CRYPTO} STREQUAL aead)
  set(KAT_PATH crypto_aead_hash/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_128_128.txt)
elseif((${ALG} STREQUAL asconav13) AND
    ${CRYPTO} STREQUAL hash)
  set(KAT_PATH crypto_aead_hash/${ALG})
  set(KAT_FILE LWC_HASH_KAT_256.txt)
elseif(${ALG} STREQUAL asconmacv13 OR
       ${ALG} STREQUAL asconprfv13 OR
       ${ALG} STREQUAL asconprfsv13)
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
