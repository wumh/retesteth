

set(command "make;out-static/libleveldb.a")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-build-out.log"
  ERROR_FILE "/home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-build-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-build-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "leveldb build command succeeded.  See also /home/wumh/1/retesteth/deps/src/leveldb-stamp/leveldb-build-*.log")
  message(STATUS "${msg}")
endif()
