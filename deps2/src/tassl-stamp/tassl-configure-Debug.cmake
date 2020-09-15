

set(command "bash;config;-Wl,--rpath=./;shared")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/home/wumh/1/retesteth/deps/src/tassl-stamp/tassl-configure-out.log"
  ERROR_FILE "/home/wumh/1/retesteth/deps/src/tassl-stamp/tassl-configure-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /home/wumh/1/retesteth/deps/src/tassl-stamp/tassl-configure-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "tassl configure command succeeded.  See also /home/wumh/1/retesteth/deps/src/tassl-stamp/tassl-configure-*.log")
  message(STATUS "${msg}")
endif()
