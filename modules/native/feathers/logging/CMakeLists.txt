####################################################################
# The Falcon Programming language
#
# CMake configuration file for module "logging"
####################################################################


if(WIN32)
   set( SYS_RC logging.rc )
   set( SYS_SRC logging_srv_win.cpp )
else()
   set( SYS_SRC logging_srv_syslog.cpp )
endif()

# Target
add_library(logging_fm MODULE
   logging.cpp
   logging_ext.cpp
   logging_mod.cpp
   logging_srv.cpp
   logging_st.cpp
   ${SYS_SRC}
   ${SYS_RC}
)

falcon_finalize_module( logging_fm )
 
set(_project_targets ${_project_targets} logging_fm PARENT_SCOPE)