/*
   FALCON - The Falcon Programming Language.
   FILE: hash_ext.h

   Provides multiple hashing algorithms
   Interface extension functions
   -------------------------------------------------------------------
   Author: Maximilian Malek
   Begin: Thu, 25 Mar 2010 02:46:10 +0100

   -------------------------------------------------------------------
   (C) Copyright 2010: The above AUTHOR

         Licensed under the Falcon Programming Language License,
      Version 1.1 (the "License"); you may not use this file
      except in compliance with the License. You may obtain
      a copy of the License at

         http://www.falconpl.org/?page_id=license_1_1

      Unless required by applicable law or agreed to in writing,
      software distributed under the License is distributed on
      an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
      KIND, either express or implied. See the License for the
      specific language governing permissions and limitations
      under the License.

*/

/** \file
   Provides multiple hashing algorithms
   Interface extension functions - header file
*/

#ifndef hash_ext_H
#define hash_ext_H

#include <falcon/module.h>

namespace Falcon {
namespace Ext {

FALCON_FUNC Func_GetSupportedHashes( ::Falcon::VMachine *vm );
FALCON_FUNC Func_hash( ::Falcon::VMachine *vm );
FALCON_FUNC Func_makeHash( ::Falcon::VMachine *vm );
FALCON_FUNC Func_hmac( ::Falcon::VMachine *vm );

void Hash_updateItem_internal(Item *what, Mod::HashBase *hash, ::Falcon::VMachine *vm, uint32 stackDepth);

}
}


#endif

/* end of hash_ext.h */
