/*
   FALCON - The Falcon Programming Language.
   FILE: tostring.cpp

   Falcon core module -- len function/method
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 11 Jun 2011 20:45:56 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/cm/len.h>
#include <falcon/vm.h>
#include <falcon/vmcontext.h>
#include <falcon/itemid.h>

#include "falcon/cm/tostring.h"

namespace Falcon {
namespace Ext {

ToString::ToString():
   Function( "toString" )
{
   setDeterm(true);
   signature("X,[S]");
   addParam("item");
   addParam("format");
}

ToString::~ToString()
{
}

void ToString::apply( VMContext* ctx, int32 )
{
   Item *elem, *format;
   
   if ( ctx->isMethodic() )
   {
      elem = &ctx->self();
      format = ctx->param( 0 );
   }
   else
   {
      elem = ctx->param( 0 );
      format = ctx->param( 1 );

      if ( elem == 0 )
      {
         throw paramError();
      }
      else
      {
         
      }
   }

   Class* cls;
   void* data;
   elem->forceClassInst( cls, data );

   ctx->ifDeep( &m_next );
   ctx->pushData( *elem );
   cls->op_toString( ctx, data );
   if( ctx->wentDeep() )
   {
      return;
   }

   ctx->retval( ctx->topData() );
   ctx->returnFrame();
}

void ToString::Next::apply_( const PStep*, VMContext* ctx  )
{
   ctx->retval( ctx->topData() );
   ctx->returnFrame();
}

}
}

/* end of tostring.cpp */