####################################################################
# The Falcon Programming language
#
# CMake configuration file for module json
####################################################################

if(WIN32)
   SET( SYS_RC json.rc )
endif()

add_library(json_fm MODULE
   json_st.cpp
   json_mod.cpp
   json_ext.cpp
   json_srv.cpp
   json.cpp
   ${SYS_RC}
)

falcon_finalize_module( json_fm )
 
set(_project_targets ${_project_targets} json_fm PARENT_SCOPE)