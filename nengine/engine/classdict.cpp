/*
   FALCON - The Falcon Programming Language.
   FILE: classdict.cpp

   Function object handler.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Wed, 27 Apr 2011 15:33:32 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/classdict.h>
#include <falcon/itemid.h>
#include <falcon/vmcontext.h>

#include <map>

#include "falcon/accesserror.h"

namespace Falcon {

typedef std::map<Item, Item> ItemDictionary;

ClassDict::ClassDict():
   Class("Dictionary", FLC_CLASS_ID_DICT )
{
}


ClassDict::~ClassDict()
{
}


void ClassDict::dispose( void* self ) const
{
   ItemDictionary* f = static_cast<ItemDictionary*>(self);
   delete f;
}


void* ClassDict::clone( void* source ) const
{
   ItemDictionary* array = static_cast<ItemDictionary*>(source);
   return new ItemDictionary(*array);
}


void ClassDict::serialize( DataWriter*, void* ) const
{
   // todo
}


void* ClassDict::deserialize( DataReader* ) const
{
   //todo
   return 0;
}

void ClassDict::describe( void* instance, String& target, int maxDepth, int maxLen ) const
{
   if( maxDepth == 0 )
   {
      target = "...";
      return;
   }
   
   ItemDictionary* dict = static_cast<ItemDictionary*>(instance);
   target.size(0);
   ItemDictionary::iterator iter = dict->begin();
   String temp;
   while( iter != dict->end() )
   {
      if( target.size() == 0 )
      {
         target += "[";
      }
      else
      {
         target += ", ";
      }

      Class* cls;
      void* inst;

      temp.size(0);
      iter->first.forceClassInst(cls, inst);
      cls->describe( inst, temp, maxDepth - 1, maxLen );
      target += temp;
      target += " => ";

      temp.size(0);
      iter->second.forceClassInst(cls, inst);
      cls->describe( inst, temp, maxDepth - 1, maxLen );
      target += temp;
   }

   target += "]";
}



void ClassDict::gcMark( void* self, uint32 mark ) const
{
   ItemDictionary& dict = *static_cast<ItemDictionary*>(self);
   ItemDictionary::iterator pos = dict.begin();
   while( pos != dict.end() )
   {
      const Item& key = pos->first;
      const Item& value = pos->second;
      if( key.isUser() && key.isGarbaged() )
      {
         key.asClass()->gcMark(key.asInst(), mark);
      }

      if( value.isUser() && value.isGarbaged() )
      {
         value.asClass()->gcMark(value.asInst(), mark);
      }

      ++pos;
   }
}

void ClassDict::enumerateProperties( void*, PropertyEnumerator& cb ) const
{
   cb("len", true);
}

//=======================================================================
//
void ClassDict::op_create( VMContext* ctx, int pcount ) const
{
   static Collector* coll = Engine::instance()->collector();
   ctx->stackResult( pcount + 1, FALCON_GC_STORE( coll, this, new ItemDictionary ) );
}

void ClassDict::op_add( VMContext*, void* ) const
{
   //TODO
}

void ClassDict::op_isTrue( VMContext* ctx, void* self ) const
{
   ctx->stackResult( 1, static_cast<ItemDictionary*>(self)->size() != 0 );
}

void ClassDict::op_toString( VMContext* ctx, void* self ) const
{
   String s;
   s.A("[Dictionary of ").N((int64)static_cast<ItemDictionary*>(self)->size()).A(" elements]");
   ctx->stackResult( 1, s );
}


void ClassDict::op_getProperty( VMContext* ctx, void* self, const String& property ) const
{
   Class::op_getProperty( ctx, self, property );
}

void ClassDict::op_getIndex( VMContext* ctx, void* self ) const
{
   Item *index, *dict_item;
   ctx->operands( index, dict_item );

   ItemDictionary& dict = *static_cast<ItemDictionary*>(self);
   ItemDictionary::iterator pos = dict.find(*index);
   
   if( pos != dict.end() )
   {
      ctx->stackResult( 2, pos->second );
   }
   else
   {
      throw new AccessError( ErrorParam( e_arracc, __LINE__, __FILE__ ) );
   }
}

void ClassDict::op_setIndex( VMContext* ctx, void* self ) const
{
   Item *value, *index, *dict_item;
   ctx->operands( value, index, dict_item );

   ItemDictionary& dict = *static_cast<ItemDictionary*>(self);
   dict[*index] = *value;
   ctx->stackResult(3, *value);
}


}

/* end of classdict.cpp */