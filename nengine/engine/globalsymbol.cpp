/*
   FALCON - The Falcon Programming Language.
   FILE: globalsybmol.cpp

   Syntactic tree item definitions -- expression elements -- symbol.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 08 Jan 2011 18:46:25 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/globalsymbol.h>
#include <falcon/exprsym.h>
#include <falcon/vm.h>

#include <falcon/trace.h>

namespace Falcon {

GlobalSymbol::GlobalSymbol( const String& name, const Item& item ):
      Symbol( t_global_symbol, name ),
      m_item( item ),
      m_nRefCount(1)
{
}

GlobalSymbol::GlobalSymbol( const String& name ):
      Symbol( t_global_symbol, name ),
      m_nRefCount(1)
{
}



GlobalSymbol::GlobalSymbol( const GlobalSymbol& other ):
      Symbol( other ),
      m_item( other.m_item ),
      m_nRefCount(1)
{
}


GlobalSymbol::~GlobalSymbol()
{}

void GlobalSymbol::assign( VMachine*, const Item& value ) const
{
   const_cast<Item*>(&m_item)->assign( value );
}

void GlobalSymbol::apply_( const PStep* ps, VMContext* ctx )
{
   const ExprSymbol* self = static_cast<const ExprSymbol*>(ps);
   register GlobalSymbol* sym = static_cast<GlobalSymbol*>(self->symbol());

#ifndef NDEBUG
   String name = sym->name();
#endif

   // l-value (assignment)?
   if( self->m_lvalue )
   {
      TRACE2( "LValue apply to global '%s'", name.c_ize() );
      sym->m_item.assign( ctx->topData() );
      // topData is already the value of the l-value evaluation.
      // so we leave it alone.
   }
   else
   {
      TRACE2( "Apply global '%s'", name.c_ize() );
      ctx->pushData( sym->m_item );
   }
}

Expression* GlobalSymbol::makeExpression()
{
   ExprSymbol* sym = new ExprSymbol(this);
   sym->setApply( apply_ );
   return sym;
}

}

/* end of globalsymbol.cpp */