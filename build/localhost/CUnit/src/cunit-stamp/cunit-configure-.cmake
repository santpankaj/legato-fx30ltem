

set(command "sh;/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CUnit/configure.sh")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CUnit/src/cunit-stamp/cunit-configure-out.log"
  ERROR_FILE "/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CUnit/src/cunit-stamp/cunit-configure-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CUnit/src/cunit-stamp/cunit-configure-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "cunit configure command succeeded.  See also /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/localhost/CUnit/src/cunit-stamp/cunit-configure-*.log")
  message(STATUS "${msg}")
endif()
