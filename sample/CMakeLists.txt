add_subdirectory(controller)
if(OPENRTM_VERSION STREQUAL OPENRTM_VERSION100 OR OPENRTM_VERSION STREQUAL OPENRTM_VERSION101 OR OPENRTM_VERSION STREQUAL OPENRTM_VERSION102 OR OPENRTM_VERSION STREQUAL OPENRTM_VERSION110)
  add_subdirectory(example)
  add_subdirectory(JoystickControl)
endif()

if( COLLADA_DOM_FOUND )
  file(GLOB_RECURSE _wrlfiles *.wrl)
  foreach(_wrlfile ${_wrlfiles})
    string(REGEX REPLACE ".wrl$" ".dae" _daefile ${_wrlfile})
    file(STRINGS ${_wrlfile} _robotmodel REGEX "DEF .* Humanoid")
    if(NOT _robotmodel STREQUAL "")
      add_custom_command(OUTPUT ${_daefile}
        COMMAND ../bin/export-collada -i ${_wrlfile} -o ${_daefile}
        DEPENDS ../bin/export-collada ${_wrlfile})
      string(REGEX REPLACE "\\." "_" _daefile_target ${_daefile})
      string(REGEX REPLACE "/" "_" _daefile_target ${_daefile_target})
      add_custom_target(export_collada_${_daefile_target} ALL DEPENDS ${_daefile})
    endif()
  endforeach()
endif()

install(DIRECTORY model project script
  DESTINATION ${RELATIVE_SAMPLE_INSTALL_PATH}
  PATTERN ".svn" EXCLUDE PATTERN "mockup" EXCLUDE)
