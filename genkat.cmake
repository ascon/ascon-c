# setup KAT file name
if(${CRYPTO} STREQUAL aead AND
    (${ALG} STREQUAL ascon128v12 OR ${ALG} STREQUAL ascon128av12 OR
    ${ALG} STREQUAL asconv12 OR ${ALG} STREQUAL asconav12))
  set(KAT_PATH crypto_aead/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_128_128.txt)
elseif(${ALG} STREQUAL ascon80pqv12)
  set(KAT_PATH crypto_aead/${ALG})
  set(KAT_FILE LWC_AEAD_KAT_160_128.txt)
elseif(${CRYPTO} STREQUAL hash AND
    (${ALG} STREQUAL asconhashv12 OR ${ALG} STREQUAL asconhashav12 OR
    ${ALG} STREQUAL asconxofv12 OR ${ALG} STREQUAL asconxofav12 OR
    ${ALG} STREQUAL asconv12 OR ${ALG} STREQUAL asconav12))
  set(KAT_PATH crypto_hash/${ALG})
  set(KAT_FILE LWC_HASH_KAT_256.txt)
else()
  message(FATAL_ERROR "KAT file name not defined for algorithm ${ALG}.")
endif()

# remove previous and generate new KAT file
file(REMOVE ${BIN_DIR}/${KAT_FILE})
if(EXISTS ${BIN_DIR}/${CONFIG})
  execute_process(COMMAND ${BIN_DIR}/${CONFIG}/${EXE_NAME})
else()
  execute_process(COMMAND ${BIN_DIR}/${EXE_NAME})
endif()
configure_file(${BIN_DIR}/${KAT_FILE} ${BIN_DIR}/${KAT_FILE} NEWLINE_STYLE LF)

# compare KAT files
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
  ${BIN_DIR}/${KAT_FILE} ${SRC_DIR}/${KAT_PATH}/${KAT_FILE}
  RESULT_VARIABLE COMPARE_RESULT)
if(${COMPARE_RESULT})
  message(FATAL_ERROR "KAT files are not identical.")
endif()
