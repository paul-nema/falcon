# Configure Oracle 
# 
# Copyright (c) 2007 Petr Vanek <petr@scribus.info>
# Copyright (c) 2009 Ivan Brezina <ibre5041@ibrezina.net>
# Copyright (c) 2010 Steven Oliver <oliver.steven@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# ORACLE_FOUND - system has Oracle OCI
# ORACLE_INCLUDES - where to find oci.h
# ORACLE_LIBRARIES - the libraries to link against to use Oracle OCI

FALCON_DEFINE_MODULE( MODULE oracle )

SET(ORACLE_FOUND "NO")
SET(ORACLE_HOME $ENV{ORACLE_HOME})


IF (ORACLE_PATH_INCLUDES)
  SET (ORACLE_INCLUDES_LOCATION ${ORACLE_PATH_INCLUDES})
ELSE (ORACLE_PATH_INCLUDES)
    SET (ORACLE_INCLUDES_LOCATION
            ${ORACLE_HOME}/rdbms/public
            ${ORACLE_HOME}/include
            # xdk
            ${ORACLE_HOME}/xdk/include
            # sdk
            ${ORACLE_HOME}/sdk/include
            # xe on windows
            ${ORACLE_HOME}/OCI/include
       )
ENDIF (ORACLE_PATH_INCLUDES)

IF (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION ${ORACLE_PATH_LIB})
ELSE (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION
            # TODO add 32/64bit version here (lib32/lib64 on some platforms)
            ${ORACLE_HOME}/lib 
            # xe on windows
            ${ORACLE_HOME}/OCI/lib/MSVC
            ${ORACLE_HOME}
        )
ENDIF (ORACLE_PATH_LIB)

FIND_PATH(
    ORACLE_INCLUDES
    oci.h
    ${ORACLE_INCLUDES_LOCATION}
    NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
)

FIND_LIBRARY(
    ORACLE_LIBRARY_CLNTSH
    NAMES libclntsh clntsh oci
    PATHS ${ORACLE_LIB_LOCATION}
    NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
)

SET (ORACLE_LIBRARY ${ORACLE_LIBRARY_CLNTSH})

IF (APPLE)
    FIND_LIBRARY(ORACLE_LIBRARY_OCIEI
        NAMES libociei ociei
        PATHS ${ORACLE_LIB_LOCATION}
    )
    MESSAGE(STATUS "Searching for libociei: ${ORACLE_LIBRARY_OCIEI} in ${ORACLE_LIB_LOCATION}")
    IF (ORACLE_LIBRARY_OCIEI)
        LIST(APPEND ORACLE_LIBRARY ${ORACLE_LIBRARY_OCIEI})
    ELSE (ORACLE_LIBRARY_OCIEI)
        MESSAGE(STATUS "libociei.dylib is not found. Maybe it will cause crashes if you are building BUNDLE")
    ENDIF (ORACLE_LIBRARY_OCIEI)
ENDIF (APPLE)

IF (ORACLE_INCLUDES AND ORACLE_LIBRARY)
    SET(ORACLE_LIBRARIES ${ORACLE_LIBRARY})
    SET(ORACLE_FOUND "YES")
ENDIF (ORACLE_INCLUDES AND ORACLE_LIBRARY)
  
# guess OCI version
IF (NOT DEFINED ORACLE_OCI_VERSION AND ORACLE_INCLUDES)
  TRY_RUN(OCIVER_TYPE OCIVER_COMPILED
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/cmake/modules/OCIVersion.c
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${ORACLE_INCLUDES}"
    OUTPUT_VARIABLE OCIVER)

  IF (${OCIVER_TYPE} GREATER "110")
    SET(ORACLE_OCI_VERSION "11G")
  ELSEIF (${OCIVER_TYPE} EQUAL "102")
    SET(ORACLE_OCI_VERSION "10G_R2")
  ELSEIF (${OCIVER_TYPE} EQUAL "101")
    SET(ORACLE_OCI_VERSION "10G")
  ELSEIF (${OCIVER_TYPE} EQUAL "91")
    SET(ORACLE_OCI_VERSION "9")
  ELSEIF (${OCIVER_TYPE} EQUAL "92")
    SET(ORACLE_OCI_VERSION "9")
  ELSEIF (NOT ${OCIVER_TYPE} GREATER "90")
    SET(ORACLE_OCI_VERSION "8I")
  ELSE (${OCIVER_TYPE} GREATER "110")
    SET(ORACLE_OCI_VERSION "10G_R2")
  ENDIF (${OCIVER_TYPE} GREATER "110")
  
  MESSAGE(STATUS "Guessed ORACLE_OCI_VERSION value: ${ORACLE_OCI_VERSION} for ${OCIVER_TYPE}")
ENDIF (NOT DEFINED ORACLE_OCI_VERSION AND ORACLE_INCLUDES)

MESSAGE(STATUS "Found Oracle: ${ORACLE_LIBRARY} (ORACLE_HOME='${ORACLE_HOME}')")
IF (ORACLE_FOUND)
    IF (NOT ORACLE_FIND_QUIETLY)
         MESSAGE(STATUS "Found Oracle: ${ORACLE_LIBRARY} (ORACLE_HOME='${ORACLE_HOME}')")
	 MESSAGE(STATUS "              ${ORACLE_INCLUDES}")
    ENDIF (NOT ORACLE_FIND_QUIETLY)
    # there *must* be OCI version defined for internal libraries
    IF (ORACLE_OCI_VERSION)
        ADD_DEFINITIONS(-DOTL_ORA${ORACLE_OCI_VERSION})
    ELSE (ORACLE_OCI_VERSION)
        MESSAGE(FATAL_ERROR "Set -DORACLE_OCI_VERSION for your oci. [8, 8I, 9I, 10G, 10G_R2]")
    ENDIF (ORACLE_OCI_VERSION)

ELSE (ORACLE_FOUND)
    MESSAGE(STATUS "Oracle not found.")
    MESSAGE(STATUS "   currently found includes: ${ORACLE_INCLUDES}")
    MESSAGE(STATUS "   currently found libs: ${ORACLE_LIBRARY}")
    IF (ORACLE_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Oracle library")
    ELSE (ORACLE_FIND_REQUIRED)
        # setup the variables for silent continue
        SET (ORACLE_INCLUDES "")
    ENDIF (ORACLE_FIND_REQUIRED)
ENDIF (ORACLE_FOUND)

# Target
ADD_LIBRARY( ${MODULE} MODULE
   oracle.cpp
   oracle_ext.cpp
   oracle_srv.cpp
)

# Link
TARGET_LINK_LIBRARIES( ${MODULE}  falcon_engine ${ORACLE_LIBRARY})
FALCON_INSTALL_MODULE2( ${MODULE} dbi )
