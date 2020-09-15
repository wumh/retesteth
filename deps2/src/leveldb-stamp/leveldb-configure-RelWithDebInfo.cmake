

set(command "/usr/bin/cmake;-Dmake=${make};-Dconfig=${config};-P;/home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-configure-RelWithDebInfo-impl.cmake")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-configure-out.log"
  ERROR_FILE "/home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-configure-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-configure-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "leveldb configure command succeeded.  See also /home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-configure-*.log")
  message(STATUS "${msg}")
endif()
