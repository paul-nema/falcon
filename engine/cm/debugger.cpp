/*
   FALCON - The Falcon Programming Language.
   FILE: debugger.cpp

   Falcon core module -- Interface to the Debugger class.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 26 Jan 2013 19:35:45 +0100

   -------------------------------------------------------------------
   (C) Copyright 2013: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "falcon/cm/debugger.cpp"

#include <falcon/cm/debugger.h>

#include <falcon/vm.h>
#include <falcon/vmcontext.h>
#include <falcon/process.h>
#include <falcon/path.h>
#include <falcon/errors/paramerror.h>
#include <falcon/errors/codeerror.h>
#include <falcon/errors/accesstypeerror.h>
#include <falcon/stdhandlers.h>
#include <falcon/processor.h>
#include <falcon/modspace.h>

#include <falcon/datawriter.h>
#include <falcon/datareader.h>
#include <falcon/itemarray.h>
#include <falcon/module.h>
#include <falcon/function.h>
#include <falcon/stream.h>

namespace Falcon {
namespace Ext {


//====================================================
// Properties.
//


namespace CDebugger {

/*#
  @method breakpoint Debugger
  @brief Breaks and gives back the control to the installed debugger.
 */
FALCON_DECLARE_FUNCTION( breakpoint, "" )
void Function_breakpoint::invoke( VMContext* ctx, int32 )
{
   ctx->returnFrame();
   ctx->setBreakpointEvent();
}

}

//==========================================================================
// Main class
//==========================================================================

ClassDebugger::ClassDebugger():
         Class("%Debugger")
{
   addMethod( new CDebugger::Function_breakpoint );

}

ClassDebugger::~ClassDebugger()
{

}

void* ClassDebugger::createInstance() const
{
   return 0;
}

void ClassDebugger::dispose( void* ) const
{
}

void* ClassDebugger::clone( void* ) const
{
   return 0;
}


}
}

/* end of debugger.cpp */