####################################################################
# The Falcon Programming language
#
# CMake configuration file for module SDL_ttf
####################################################################

FALCON_DEFINE_MODULE( SDLGLU glu )

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

Find_Package ( SDL REQUIRED )
Find_Package ( OpenGL REQUIRED )

# Inclusion settings
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Target
ADD_LIBRARY( ${SDLGLU} MODULE
   glu.cpp
   glu_ext.cpp
   glu_mod.cpp
)

#Link
TARGET_LINK_LIBRARIES( ${SDLGLU} falcon_engine ${SDL_LIBRARY} ${OPENGL_LIBRARY})

SET_TARGET_PROPERTIES( ${SDLGLU}
   PROPERTIES PREFIX "")
#Install
FALCON_INSTALL_MODULE2( ${SDLGLU} sdl )
