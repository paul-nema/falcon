/*
   FALCON - The Falcon Programming Language.
   FILE: synfunc.h

   SynFunc objects -- expanding to new syntactic trees.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 15 Jan 2011 19:09:07 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/synfunc.h>
#include <falcon/vmcontext.h>
#include <falcon/item.h>
#include <falcon/statement.h>
#include <falcon/trace.h>
#include <falcon/localsymbol.h>
#include <falcon/closedsymbol.h>

namespace Falcon
{

SynFunc::SynFunc( const String& name, Module* owner, int32 line ):
   Function( name, owner, line )
{}


SynFunc::~SynFunc()
{
}

void SynFunc::invoke( VMContext* ctx, int32 nparams )
{
   // Used by the VM to insert this opcode if needed to exit SynFuncs.
   static StmtReturn s_a_return;

   // nothing to do?
   if( syntree().empty() )
   {
      TRACE( "-- function %s is empty -- not calling it", locate().c_ize() );
      ctx->returnFrame();
      return;
   }
   
   // fill the parameters
   TRACE1( "-- filing parameters: %d/%d, and locals %d",
         nparams, this->paramCount(),
         this->symbols().localCount() - this->paramCount() );

   register int lc = (int) this->symbols().localCount();
   if( lc > nparams )
   {
      ctx->addLocals( lc - nparams );
   }
   
   if( this->syntree().last()->type() != Statement::return_t )
   {
      MESSAGE1( "-- Pushing extra return" );
      ctx->pushCode( &s_a_return );
   }

   ctx->pushCode( &this->syntree() );
}

}

/* end of synfunc.cpp */