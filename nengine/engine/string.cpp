/*
   FALCON - The Falcon Programming Language.
   FILE: string.cpp

   Implementation of Falcon Strings.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 13 Feb 2011 15:02:40 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/string.cpp"

/** \file
   Implementation of Core Strings.
   \todo Add support for international strings.
*/

#include <stdlib.h>
#include <falcon/string.h>
#include <falcon/datawriter.h>
#include <falcon/datareader.h>
#include <falcon/common.h>
#include <falcon/collector.h>
#include <falcon/engine.h>


#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#include "falcon/datareader.h"


namespace Falcon {

namespace csh {

Static handler_static;
Buffer handler_buffer;
Static16 handler_static16;
Buffer16 handler_buffer16;
Static32 handler_static32;
Buffer32 handler_buffer32;


template<typename t1, typename t2>
inline void copySized( byte* dest_, byte* src_, uint32 size )
{
   if ( size != 0 )
   {
      t1* dest = (t1*) dest_;
      t2* src = (t2*) src_;

      do {
         size--;
         dest[size] = (t1) src[size];
      } while( size > 0 );
   }
}

template<typename T>
inline void copySame( byte* dest_, byte* src_, uint32 size )
{
   if ( size != 0 )
   {
      uint32 len = size * sizeof(T);
      T* dest = (T*) dest_;
      T* src = (T*) src_;


      // overlapping? -- use memmove, else use memcpy; but do the check only if worth
      if( len < 10 ||
            (src + len > dest) ||
            (dest + len > src )
      )
         memmove( dest, src, len );
      else
         memcpy( dest, src, len );
   }
}

// service function; adapts a smaller buffer into a larger one
// srclen is in "elements" (bytes * charLen).
// returns also the dynamic buffer manipulator useful to handle the target buffer
static Base* adaptBuffer( byte *srcBuffer, uint32 srcPos, uint32 srcCharLen,
                          byte *destBuffer, uint32 destPos, uint32 destCharLen,
                          uint32 srcLen )
{
   srcBuffer += srcPos * srcCharLen;
   destBuffer += destPos * destCharLen;

   switch( destCharLen )
   {
      case 1:
         switch( srcCharLen ) {
            case 1: copySame<byte>( destBuffer, srcBuffer, srcLen ); break;
            case 2: copySized<byte, uint16>( destBuffer, srcBuffer, srcLen ); break;
            case 4: copySized<byte, uint32>( destBuffer, srcBuffer, srcLen ); break;
         }
         return &handler_buffer;

      case 2:
         switch( srcCharLen ) {
            case 1: copySized<uint16, byte>( destBuffer, srcBuffer, srcLen ); break;
            case 2: copySame<uint16>( destBuffer, srcBuffer, srcLen ); break;
            case 4: copySized<uint16, uint32>( destBuffer, srcBuffer, srcLen ); break;
         }
         return &handler_buffer16;

      case 4:
         switch( srcCharLen ) {
            case 1: copySized<uint32,byte>( destBuffer, srcBuffer, srcLen ); break;
            case 2: copySized<uint32,uint16>( destBuffer, srcBuffer, srcLen ); break;
            case 4: copySame<uint32>( destBuffer, srcBuffer, srcLen ); break;
         }
         return &handler_buffer32;
   }

   return 0;
}

length_t Byte::length( const String *str ) const
{
   return str->size() / charSize();
}


char_t Byte::getCharAt( const String *str, length_t pos ) const
{
   return (char_t) str->getRawStorage()[pos];
}

length_t Static16::length( const String *str ) const
{
   return str->size() >> 1;
}

length_t Static32::length( const String *str ) const
{
   return str->size() >> 2;
}

length_t Buffer16::length( const String *str ) const
{
   return str->size() >> 1;
}

length_t Buffer32::length( const String *str ) const
{
   return str->size() >> 2;
}


char_t Static16::getCharAt( const String *str, length_t pos ) const
{
   return (char_t)  reinterpret_cast< uint16 *>(str->getRawStorage())[ pos ];
}

char_t Buffer16::getCharAt( const String *str, length_t pos ) const
{
   return (char_t)  reinterpret_cast< uint16 *>(str->getRawStorage())[ pos ];
}

char_t Static32::getCharAt( const String *str, length_t pos ) const
{
   return reinterpret_cast< char_t *>(str->getRawStorage())[ pos ];
}

char_t Buffer32::getCharAt( const String *str, length_t pos ) const
{
   return reinterpret_cast< char_t *>(str->getRawStorage())[ pos ];
}



void Byte::subString( const String *str, int32 start, int32 end, String *tgt ) const
{
   length_t nlen = str->length();
   
   if( start < 0 )
      start = int(nlen) + start;
   if( end < 0 )
      end = int(nlen) + end + 1;
   if ( start < 0 || start >= (int)nlen || end < 0 || end == start) {
      tgt->size( 0 );
      return;
   }


   byte *storage, *source;
   uint16 cs = charSize();
   source = str->getRawStorage();

   if ( end < start ) {
      length_t len = start - end + 1;
      if ( tgt->allocated() < len * cs ) {
         storage = (byte *) malloc( len * cs );
      }
      else
         storage = tgt->getRawStorage();

      switch( cs )
      {
         case 1:
         {
            for( length_t i = 0; i < len ; i ++ )
               storage[i] = source[start-i];
            tgt->size( len );
         }
         break;

         case 2:
         {
            uint16 *storage16 = (uint16 *) storage;
            uint16 *source16 = (uint16 *) source;
            for( length_t i = 0; i < len ; i ++ )
               storage16[i] = source16[start-i];
            tgt->size( len * 2 );
         }
         break;

         case 4:
         {
            uint32 *storage32 = (uint32 *) storage;
            uint32 *source32 = (uint32 *) source;
            for( length_t i = 0; i < len ; i ++ )
               storage32[i] = source32[start-i];
            tgt->size( len * 4 );
         }
         break;
      }
   }
   else {
      if ( end > (int)nlen ) 
         end = nlen;
         
      length_t len = (end - start)*cs;
      if ( tgt->allocated() < len ) {
         storage = (byte *) malloc( len );
      }
      else
         storage = tgt->getRawStorage();

      memcpy( storage, str->getRawStorage() + (start * cs) , len  );
      tgt->size( len );
   }

   // was the storage not enough?
   if ( storage != tgt->getRawStorage() )
   {
      if ( tgt->allocated() != 0 )
         tgt->manipulator()->destroy( tgt );

      tgt->allocated( tgt->size() );
      tgt->setRawStorage( storage );
   }

   switch( cs )
   {
      case 1: tgt->manipulator( &handler_buffer ); break;
      case 2: tgt->manipulator( &handler_buffer16 ); break;
      case 4: tgt->manipulator( &handler_buffer32 ); break;
   }
}


bool Byte::change( String *str, length_t start, length_t end, const String *source ) const
{
   length_t strLen = str->length();

   if ( start >= strLen )
      return false;

   if ( end > strLen )
      end = strLen;


   if ( end < start ) {
      length_t temp = end;
      end = start+1;
      start = temp;
   }
   int32 len = end - start;
   insert( str, start, len, source );
   return true;
}


String *Byte::clone( const String *str ) const
{
   return new String( *str );
}

length_t Byte::find( const String *str, const String *element, length_t start, length_t end ) const
{
   if ( str->size() == 0 || element->size() == 0 )
      return npos;

   if ( end > str->length() )  // npos is defined to be greater than any size
      end = str->length();

   if ( end < start ) {
      register length_t temp = end;
      end = start;
      start = temp;
   }

   length_t pos = start;
   char_t keyStart = element->getCharAt( 0 );
   length_t elemLen = element->length();

   while( pos + elemLen <= end )
   {
      if ( str->getCharAt( pos ) == keyStart )
      {
         length_t len = 1;
         while( pos + len < end && len < elemLen && element->getCharAt(len) == str->getCharAt( pos + len ) )
            len++;
         if ( len == elemLen )
            return pos;
      }
      pos++;
   }

   // not found.
   return npos;
}


length_t Byte::rfind( const String *str, const String *element, length_t start, length_t end ) const
{
   if ( str->size() == 0 || element->size() == 0 )
      return npos;

   if ( end > str->length() )  // npos is defined to be greater than any size
      end = str->length();

   if ( end < start ) {
      length_t temp = end;
      end = start;
      start = temp;
   }

   char_t keyStart = element->getCharAt( 0 );
   length_t elemLen = element->length();
   if ( elemLen > (end - start) )
   {
      // can't possibly be found
      return npos;
   }

   length_t pos = end - elemLen;

   while( pos >= start  )
   {
      if ( str->getCharAt( pos ) == keyStart ) {
         length_t len = 1;
         while( pos + len < end && len < elemLen && element->getCharAt(len) == str->getCharAt( pos + len ) )
            len++;
         if ( len == elemLen )
            return pos;
      }
      if ( pos == 0 ) break;
      pos--;
   }

   // not found.
   return npos;
}


void Byte::remove( String *str, length_t pos, length_t len ) const
{
   length_t sl = str->length();
   if ( len == 0 || pos > sl )
      return;

   uint16 cs = charSize();
   if ( pos + len > sl )
      len = sl - pos;

   length_t newLen = (sl - len) *cs;
   byte *mem = (byte *) malloc( newLen );
   if ( pos > 0 )
      memcpy( mem, str->getRawStorage(), pos * cs );
   if ( pos + len < sl )
      memcpy( mem + pos *cs, str->getRawStorage() +( pos + len) *cs , (sl - pos - len) * cs );

   // for non-static strings...
   if ( str->allocated() != 0 )
   {
      free( str->getRawStorage() );
   }
   str->setRawStorage( mem, newLen );

   // subclasses will set correct manipulator if needed.
}


void Byte::bufferize( String *str ) const
{
   // already buffered?
   if ( ! str->isStatic() )
      return;

   length_t size = str->m_size;
   if ( size != 0 )
   {
      length_t oldSize = str->allocated();

      byte *mem = (byte *) malloc( size );
      memcpy( mem, str->getRawStorage(), size );

      if( oldSize != 0 )
      {
         free( str->getRawStorage() );
      }

      str->setRawStorage( mem, size );
      str->m_class = str->m_class->bufferedManipulator();
   }
}

void Byte::bufferize( String *str, const String *strOrig ) const
{
   // copy the other string contents.
   if ( str->m_allocated != 0 )
      free( str->m_storage );

   length_t size = strOrig->m_size;
   if ( size == 0 ) {
      str->m_class = &handler_static;
      str->setRawStorage( 0, 0 );
   }
   else {
      byte *mem = (byte *) malloc( size );
      memcpy( mem, strOrig->getRawStorage(), size );
      str->setRawStorage( mem, size );
      str->m_class = strOrig->m_class->bufferedManipulator();
   }

}

void Byte::reserve( String *str, length_t size, bool relative, bool block ) const
{
   if ( relative )
      size += str->m_allocated;

   register uint16 chs = charSize();
   if ( block )
   {
      if ( size % FALCON_STRING_ALLOCATION_BLOCK != 0 )
      {
         size /= (FALCON_STRING_ALLOCATION_BLOCK * chs);
         size ++;
         size *= (FALCON_STRING_ALLOCATION_BLOCK * chs);
      }
   }

   length_t nextAlloc = size * chs;

   // the required size may be already allocated
   if ( nextAlloc > str->allocated() )
   {
      byte *mem = (byte *) malloc( nextAlloc );
      length_t size = str->m_size;
      if ( str->m_size > 0 )
         memcpy( mem, str->m_storage, str->m_size );

      // we can now destroy the old string.
      if ( str->allocated() != 0 )
         free( str->m_storage );

      str->m_storage = mem;
      str->m_size = size;
      str->m_allocated = nextAlloc;
   }

   // let the subclasses set the correct manipulator
}


//============================================================0

void Static::shrink( String * ) const
{
// do nothing
}

void Static::reserve( String *str, length_t size, bool relative, bool block ) const
{
   Byte::reserve( str, size, relative, block );
   str->m_class = &handler_buffer;
}

const Base *Static::bufferedManipulator() const
{
   return  &handler_buffer;
}

void Static16::reserve( String *str, length_t size, bool relative, bool block ) const
{
   Byte::reserve( str, size, relative, block );
   str->m_class = &handler_buffer16;
}

const Base *Static16::bufferedManipulator() const
{
   return  &handler_buffer16;
}

void Static32::reserve( String *str, length_t size, bool relative, bool block ) const
{
   Byte::reserve( str, size, relative, block );
   str->m_class = &handler_buffer32;
}

const Base *Static32::bufferedManipulator() const
{
   return  &handler_buffer32;
}

void Static::setCharAt( String *str, length_t pos, char_t chr ) const
{

   byte *buffer;
   length_t size = str->size();

   if( chr <= 0xFF )
   {
      buffer = (byte *) malloc( size );
      memcpy( buffer, str->getRawStorage(), size );
      buffer[ pos ] = (byte) chr;
      str->manipulator( &handler_buffer );
   }
   else if ( chr <= 0xFFFF )
   {
      uint16 *buf16 =  (uint16 *) malloc( size * 2 );
      buffer = str->getRawStorage();
      for ( length_t i = 0; i < size; i ++ )
         buf16[ i ] = (uint16) buffer[ i ];

      buf16[ pos ] = (uint16) chr;
      buffer = (byte *) buf16;
      size *= 2;
      str->manipulator( &handler_buffer16 );
   }
   else
   {
      uint32 *buf32 =  (uint32 *) malloc( size * 4 );
      buffer = str->getRawStorage();
      for ( length_t i = 0; i < size; i ++ )
         buf32[ i ] = (uint32) buffer[ i ];

      buf32[ pos ] = chr;
      buffer = (byte *) buf32;
      size *= 4;
      str->manipulator( &handler_buffer32 );
   }

   uint32 oldSize = str->allocated();
   if( oldSize != 0 )
      free( str->getRawStorage() );
   str->setRawStorage( buffer, size );
}


void Static16::setCharAt( String *str, length_t pos, char_t chr ) const
{

   byte *buffer;
   length_t size = str->size();

   if ( chr <= 0xFFFF )
   {
      uint16 *buf16 =  (uint16 *) malloc( size );
      memcpy( buf16, str->getRawStorage(), size );
      buf16[ pos ] = (uint16) chr;
      buffer = (byte *) buf16;
      str->manipulator( &handler_buffer16 );
   }
   else
   {
      uint32 *buf32 =  (uint32 *) malloc( size * 2 );
      uint16 *buf16 = (uint16 *) str->getRawStorage();
      for ( length_t i = 0; i < size; i ++ )
         buf32[ i ] = (uint32) buf16[ i ];

      buf32[ pos ] = chr;
      buffer = (byte *) buf32;
      size *= 2;
      str->manipulator( &handler_buffer32 );
   }

   uint32 oldSize = str->allocated();
   str->setRawStorage( buffer, size );
   if( oldSize != 0 )
      free( str->getRawStorage() );
}

void Static32::setCharAt( String *str, length_t pos, char_t chr ) const
{
   byte *buffer;
   length_t size = str->size();

   uint32 *buf32 =  (uint32 *) malloc( size );
   memcpy( buf32, str->getRawStorage(), size );

   buf32[ pos ] = chr;
   buffer = (byte *) buf32;
   str->manipulator( &handler_buffer32 );
   uint32 oldSize = str->allocated();
   str->setRawStorage( buffer, size );
   if( oldSize != 0 )
      free( str->getRawStorage() );
}

void Buffer::setCharAt( String *str, length_t pos, char_t chr ) const
{
   byte *buffer;
   length_t size = str->size();

   if( chr <= 0xFF )
   {
      str->getRawStorage()[ pos ] = (byte) chr;
   }
   else if ( chr <= 0xFFFF )
   {
      uint16 *buf16 =  (uint16 *) malloc( size * 2 );
      buffer = str->getRawStorage();
      for ( length_t i = 0; i < size; i ++ )
         buf16[ i ] = (uint16) buffer[ i ];

      buf16[ pos ] = (uint16) chr;
      size *= 2;
      str->manipulator( &handler_buffer16 );
      if( str->allocated() > 0 )
         free( buffer );
      str->setRawStorage( (byte *) buf16, size );
   }
   else
   {
      uint32 *buf32 =  (uint32 *) malloc( size * 4 );
      buffer = str->getRawStorage();
      for ( length_t i = 0; i < size; i ++ )
         buf32[ i ] = (uint32) buffer[ i ];

      buf32[ pos ] = chr;
      size *= 4;
      str->manipulator( &handler_buffer32 );
      if( str->allocated() > 0 )
         free( buffer );
      str->setRawStorage( (byte *) buf32, size );
   }
}


void Buffer16::setCharAt( String *str, length_t pos, char_t chr ) const
{
   if ( chr <= 0xFFFF )
   {
      uint16 *buf16 =  (uint16 *) str->getRawStorage();
      buf16[ pos ] = (uint16) chr;
   }
   else
   {
      int32 size = str->size();
      uint32 *buf32 =  (uint32 *) malloc( size * 2 );
      uint16 *buf16 = (uint16 *) str->getRawStorage();
      for ( int i = 0; i < size; i ++ )
         buf32[ i ] = (uint32) buf16[ i ];

      buf32[ pos ] = chr;
      size *= 2;
      str->manipulator( &handler_buffer32 );
      if( str->allocated() > 0 )
         free( buf16 );
      str->setRawStorage( (byte *) buf32, size );
   }

}

void Buffer32::setCharAt( String *str, length_t pos, char_t chr ) const
{
   uint32 *buf32 = (uint32 *) str->getRawStorage();
   buf32[ pos ] = chr;
}


void Static::insert( String *str, uint32 pos, uint32 len, const String *source ) const
{
   uint32 sourceLen = source->length();

   uint32 strLen = str->length();

   if ( pos + len > str->size() )
      len = str->size() - pos;

   uint32 strCharSize = str->manipulator()->charSize();

   uint32 destCharSize = source->manipulator()->charSize() > str->manipulator()->charSize() ?
      source->manipulator()->charSize() : str->manipulator()->charSize() ; // can be 1 or larger

   uint32 finalSize = destCharSize * (strLen - len + sourceLen );
   uint32 finalAlloc = ((finalSize / FALCON_STRING_ALLOCATION_BLOCK) + 1) *
      FALCON_STRING_ALLOCATION_BLOCK;

   // we know we have to relocate, so just do the relocation step
   byte *mem = (byte*) malloc( finalAlloc );
   if ( pos > 0 )
      adaptBuffer( str->getRawStorage(), 0, strCharSize, mem, 0, destCharSize, pos );

   str->manipulator(
      adaptBuffer( source->getRawStorage(), 0, source->manipulator()->charSize(),
                   mem, pos, destCharSize, sourceLen ) );

   if ( pos + len < strLen )
      adaptBuffer( str->getRawStorage(), pos + len, strCharSize,
                   mem,  pos + sourceLen, destCharSize,
                   strLen - pos - len );

   str->size( finalSize );

   // Static strings CAN have non-static memory: expecially if they are de-serialized strings in modules.
   uint32 oldSize = str->allocated();
   str->allocated( finalAlloc );

   if ( oldSize > 0 )
   {
      free( str->getRawStorage() );
   }
   str->setRawStorage( mem );
}


void Buffer::insert( String *str, length_t pos, length_t len, const String *source ) const
{
   length_t sourceLen = source->length();

   length_t strLen = str->length();
   if ( pos + len > str->size() )
      len = str->size() - pos;

   uint16 strCharSize = str->manipulator()->charSize();
   length_t posBytes = pos *strCharSize;
   length_t lenBytes = len *strCharSize;

   uint16 destCharSize = source->manipulator()->charSize() > strCharSize ?
      source->manipulator()->charSize() : strCharSize; // can be 1 or larger

   length_t finalSize = destCharSize * (strLen - len + sourceLen );

   // should we re-allocate?
   if( finalSize > str->allocated() || destCharSize > strCharSize )
   {

      length_t finalAlloc = ((finalSize / FALCON_STRING_ALLOCATION_BLOCK) + 1) *
         FALCON_STRING_ALLOCATION_BLOCK;

      // we know we have to relocate, so just do the relocation step
      byte *mem = (byte*) malloc( finalAlloc );
      if ( pos > 0 )
         adaptBuffer( str->getRawStorage(), 0, strCharSize,
                      mem, 0, destCharSize, pos );

      str->manipulator(
         adaptBuffer( source->getRawStorage(), 0, source->manipulator()->charSize(),
                      mem, pos, destCharSize, sourceLen ) );

      if ( pos + len < strLen )
         adaptBuffer( str->getRawStorage(), pos + len, strCharSize,
                      mem, pos+sourceLen, destCharSize,
                      strLen - pos - len );

      if ( str->allocated() != 0 )
         free( str->getRawStorage() );

      str->allocated( finalAlloc );
      str->setRawStorage( mem );
   }
   else
   {
      // should we move the tail?
      if ( pos + len < strLen )
      {
         // can we maintain our char size?
         if( destCharSize == strCharSize )
         {
            length_t sourceLenBytes = sourceLen * destCharSize;
            // then just move the postfix away
            memmove( str->getRawStorage() + posBytes + sourceLenBytes,
                     str->getRawStorage() + posBytes + lenBytes,
                     str->size() - posBytes - lenBytes );
         }
         else {
            // adapt it to the new size.
               adaptBuffer( str->getRawStorage(), pos + len, strCharSize,
                  str->getRawStorage(), pos + sourceLen, destCharSize,
                  strLen - pos - len );
         }
      }

      // adapt the incoming part
      str->manipulator(
         adaptBuffer( source->getRawStorage(), 0, source->manipulator()->charSize(),
                     str->getRawStorage(), pos, destCharSize,
                     sourceLen ) );

      // eventually adapt the head -- adaptBuffer can work on itself
      if ( pos > 0 && destCharSize != strCharSize )
         str->manipulator(
            adaptBuffer( str->getRawStorage(), 0, strCharSize,
                         str->getRawStorage(), 0, destCharSize,
                         pos ) );
   }

   str->size( finalSize );
}


void Static::remove( String *str, length_t pos, length_t len ) const
{
   Byte::remove( str, pos, len );
   // changing string type.
   str->manipulator( &handler_buffer );
}

void Static16::remove( String *str, length_t pos, length_t len ) const
{
   Byte::remove( str, pos, len );
   // changing string type.
   str->manipulator( &handler_buffer16 );
}

void Static32::remove( String *str, length_t pos, length_t len ) const
{
   Byte::remove( str, pos, len );
   // changing string type.
   str->manipulator( &handler_buffer32 );
}


void Static::destroy( String *str ) const
{
   if ( str->allocated() > 0 ) {
      free( str->getRawStorage() );
      str->allocated( 0 );
      str->size(0);
   }
}


void Buffer::shrink( String *str ) const
{
   if( str->size() > str->allocated() )
   {
      if ( str->size() == 0 )
      {
         destroy( str );
      }
      else {
         byte *mem = (byte *) realloc( str->getRawStorage(), str->size() );
         if ( mem != str->getRawStorage() )
         {
            memcpy( mem, str->getRawStorage(), str->size() );
            str->setRawStorage( mem );
         }
         str->allocated( str->size() );
      }
   }
}

void Buffer::reserve( String *str, length_t size, bool relative, bool block ) const
{
   Byte::reserve( str, size, relative, block );
   // manipulator is ok
}


void Buffer::destroy( String *str ) const
{
   if ( str->allocated() > 0 ) {
      free( str->getRawStorage() );
      str->allocated( 0 );
      str->size(0);
   }
}

} // namespace csh


//=================================================================
// The string class
//=================================================================


String::String( length_t size ):
   m_class( &csh::handler_buffer )
{
   m_storage = (byte *) malloc( size );
   m_allocated = size;
   m_size = 0;
}

String::String( const char *data ):
   m_class( &csh::handler_static ),
   m_allocated( 0 ),
   m_storage( (byte*) const_cast< char *>(data) )
{
   m_size = strlen( data );
}

String::String( const char *data, length_t len ):
   m_class( &csh::handler_buffer )
{
   m_size = len != String::npos ? len : strlen( data );
   m_allocated = (( m_size / FALCON_STRING_ALLOCATION_BLOCK ) + 1 ) * FALCON_STRING_ALLOCATION_BLOCK;
   m_storage = (byte *) malloc( m_allocated );
   memcpy( m_storage, data, m_size );
}


String::String( const wchar_t *data ):
   m_allocated( 0 ),
   m_storage( (byte*) const_cast< wchar_t *>(data) )
{
   if ( sizeof( wchar_t ) == 2 )
      m_class = &csh::handler_static16;
   else
      m_class = &csh::handler_static32;

   length_t s = 0;
   while( data[s] != 0 )
      ++s;
   m_size = s * sizeof( wchar_t );
}


String::String( const wchar_t *data, length_t len ):
   m_allocated( 0 ),
   m_storage( (byte *) const_cast< wchar_t *>( data ) )
{
   if ( sizeof( wchar_t ) == 2 )
      m_class = &csh::handler_buffer16;
   else
      m_class = &csh::handler_buffer32;

   if ( len != String::npos )
   {
      m_size = len * sizeof( wchar_t );
   }
   else
   {
      length_t s = 0;
      while( data[s] != 0 )
         ++s;
      m_size = s * sizeof( wchar_t );
   }

   m_allocated = (( m_size / FALCON_STRING_ALLOCATION_BLOCK ) + 1 ) * FALCON_STRING_ALLOCATION_BLOCK;
   m_storage = (byte *) malloc( m_allocated );
   memcpy( m_storage, data, m_size );
}


String::String( const String &other, length_t begin, length_t end ):
   m_allocated( 0 ),
   m_size( 0 ),
   m_storage( 0 )
{
   // by default, copy manipulator
   m_class = other.m_class;

   if ( other.m_allocated == 0 )
   {
      if ( other.m_size == 0 ) {
         m_size = 0;
         m_allocated = 0;
         setRawStorage( 0 );
      }
      else {
         if( begin < end )
         {
            uint32 cs = m_class->charSize();
            setRawStorage( other.getRawStorage() + begin * cs );
            m_size = (end - begin ) * cs;
            if ( m_size > (other.m_size - (begin *cs )) )
               m_size = other.m_size - (begin *cs );
         }
         else {
            // reverse substring, we have to bufferize.
            other.m_class->subString( &other, begin, end, this );
         }
      }
   }
   else
      other.m_class->subString( &other, begin, end, this );
}


void String::copy( const String &other )
{
   if ( m_allocated != 0 )
      m_class->destroy( this );

   m_class = other.m_class;
   m_size = other.m_size;
   m_allocated = other.m_allocated;
   if ( m_allocated > 0 ) {
      m_storage = (byte *) malloc( m_allocated );
      if ( m_size > 0 )
         memcpy( m_storage, other.m_storage, m_size );
   }
   else
      m_storage = other.m_storage;
}


String &String::adopt( char *buffer, length_t size, length_t allocated )
{
   if ( m_allocated != 0 )
      m_class->destroy( this );

   m_class = &csh::handler_buffer;
   m_size = size;
   m_allocated = allocated;
   m_storage = (byte *) buffer;


   return *this;
}

String &String::adopt( wchar_t *buffer, length_t size, length_t allocated )
{
   if ( m_allocated != 0 )
      m_class->destroy( this );

   if ( sizeof( wchar_t ) == 2 )
      m_class = &csh::handler_buffer16;
   else
      m_class = &csh::handler_buffer32;

   m_size = size * sizeof( wchar_t );
   m_allocated = allocated;
   m_storage = (byte *) buffer;

   return *this;
}

int String::compare( const char *other ) const
{
   length_t pos = 0;
   length_t len = length();

   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      if ( c1 < (char_t) *other )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > (char_t) *other || (char_t) *other == 0 )
         return 1;

      other ++;
      pos++;
   }

   if ( *other != 0 )
      // other is greater
      return -1;

   // the strings are the same
   return 0;
}

int String::compareIgnoreCase( const char *other ) const
{
   length_t pos = 0;
   length_t len = length();

   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      char_t cmpval = (char_t) *other;
      if ( c1 >=0x41 && c1 <= 0x5A )
         c1 += 0x20;

      if ( cmpval >=0x41 && cmpval <= 0x5A )
         cmpval += 0x20;

      if ( c1 < cmpval )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > cmpval || cmpval == 0 )
         return 1;

      other ++;
      pos++;
   }

   if ( *other != 0 )
      // other is greater
      return -1;

   // the strings are the same
   return 0;
}

int String::compare( const wchar_t *other ) const
{
   length_t pos = 0;
   length_t len = length();

   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      if ( c1 < (char_t) *other )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > (char_t) *other || (char_t) *other == 0 )
         return 1;

      other ++;
      pos++;
   }

   if ( *other != 0 )
      // other is greater
      return -1;

   // the strings are the same
   return 0;
}

int String::compareIgnoreCase( const wchar_t *other ) const
{
   length_t pos = 0;
   length_t len = length();

   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      char_t cmpval = (char_t) *other;
      if ( c1 >=0x41 && c1 <= 0x5A )
         c1 += 0x20;

      if ( cmpval >=0x41 && cmpval <= 0x5A )
         cmpval += 0x20;

      if ( c1 < cmpval )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > cmpval || cmpval == 0 )
         return 1;

      other ++;
      pos++;
   }

   if ( *other != 0 )
      // other is greater
      return -1;

   // the strings are the same
   return 0;
}

int String::compare( const String &other ) const
{
   length_t len1 = length();
   length_t len2 = other.length();
   length_t len = len1 > len2 ? len2 : len1;

   length_t pos = 0;
   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      char_t c2 = other.getCharAt( pos );

      if ( c1 < c2 )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > c2 )
         return 1;

      pos++;
   }

   // the strings are the same?
   if ( len1 < len2 )
      return -1;
   // return greater also if other is ended; we are NOT ended...
   else if ( len1 > len2 )
      return 1;

   // yes
   return 0;
}


int String::compareIgnoreCase( const String &other ) const
{
   length_t len1 = length();
   length_t len2 = other.length();
   length_t len = len1 > len2 ? len2 : len1;

   length_t pos = 0;
   while( pos < len )
   {
      char_t c1 = getCharAt( pos );
      char_t c2 = other.getCharAt( pos );

      if ( c1 >=0x41 && c1 <= 0x5A )
         c1 += 0x20;

      if ( c2 >=0x41 && c2 <= 0x5A )
         c2 += 0x20;

      if ( c1 < c2 )
         return -1;
      // return greater also if other is ended; we are NOT ended...
      else if ( c1 > c2 )
         return 1;

      pos++;
   }

   // the strings are the same?

   if ( len1 < len2 )
      return -1;
   // return greater also if other is ended; we are NOT ended...
   else if ( len1 > len2 )
      return 1;

   // yes
   return 0;
}

length_t String::toCString( char *target, uint32 bufsize ) const
{
   length_t len = length();

   // we already know that the buffer is too small?
    if ( bufsize <= len )
      return npos;

   length_t pos = 0;
   length_t done = 0;

   while( done < len && pos < bufsize )
   {
      char_t chr = getCharAt( done );

      if ( chr < 0x80 )
      {
         target[ pos ++ ] = chr;
      }
      else if ( chr < 0x800 )
      {
         if ( pos + 2 > bufsize )
            return npos;

         target[ pos ++ ] = 0xC0 | ((chr >> 6 ) & 0x1f);
         target[ pos ++ ] = 0x80 | (0x3f & chr);
      }
      else if ( chr < 0x10000 )
      {
         if ( pos + 3 > bufsize )
            return npos;

         target[ pos ++ ] = 0xE0 | ((chr >> 12) & 0x0f );
         target[ pos ++ ] = 0x80 | ((chr >> 6) & 0x3f );
         target[ pos ++ ] = 0x80 | (0x3f & chr);
      }
      else
      {
         if ( pos + 4 > bufsize )
            return npos;

         target[ pos ++ ] = 0xF0 | ((chr >> 18) & 0x7 );
         target[ pos ++ ] = 0x80 | ((chr >> 12) & 0x3f );
         target[ pos ++ ] = 0x80 | ((chr >> 6) & 0x3f );
         target[ pos ++ ] = 0x80 | (0x3f & chr );
      }

      ++done;
   }

   if ( pos >= bufsize )
      return npos;

   target[pos] = '\0';
   return pos;
}

length_t String::toWideString( wchar_t *target, length_t bufsize ) const
{
   length_t len = length();
   if ( bufsize <= len * sizeof( wchar_t ) )
      return npos;

   if ( sizeof( wchar_t ) == 2 ) {
      for( length_t i = 0; i < len; i ++ )
      {
         char_t chr = getCharAt( i );
         if ( chr > 0xFFFF )
            target[ i ] = L'?';
         else
            target[ i ] = chr;
      }
   }
   else {
      for( length_t i = 0; i < len; i ++ )
      {
         target[ i ] = getCharAt( i );
      }
   }
	target[len] = 0;

   return len;
}

void String::append( const String &str )
{
/*   if ( str.size() < FALCON_STRING_ALLOCATION_BLOCK )
      m_class->reserve( this, FALCON_STRING_ALLOCATION_BLOCK, true, true ); */
   m_class->insert( this, length(), 0, &str );
}

void String::append( char_t chr )
{
   if ( size() >= allocated() )
   {
      m_class->reserve( this, size() + FALCON_STRING_ALLOCATION_BLOCK, false, true ); // allocates a whole block next
   }

   // reserve forces to buffer, so we have only to extend
   size( size() + m_class->charSize() );
   // and insert the last char:

   setCharAt( length() - 1, chr );
   // if the chr can't fit our size, the whole string will be refitted, including the last
   // empty char that we added above.
}

void String::prepend( char_t chr )
{
   if ( size() >= allocated() )
   {
      m_class->reserve( this, size() + FALCON_STRING_ALLOCATION_BLOCK, false, true ); // allocates a whole block next
   }

   // reserve forces to buffer, so we have only to extend
   memmove( m_storage + m_class->charSize(), m_storage, size() );
   size( size() + m_class->charSize() );

   // and insert the first char
   setCharAt( 0, chr );
}


void String::escape( String &strout ) const
{
   internal_escape( strout, false );
}

void String::escapeFull( String &strout ) const
{
   internal_escape( strout, true );
}


void String::internal_escape( String &strout, bool full ) const
{
   length_t len = length();
   length_t pos = 0;
   strout.m_class->reserve( &strout, len ); // prepare for at least len chars
   strout.size( 0 ); // clear target string

   while( pos < len )
   {
      uint32 chat = getCharAt( pos );
      switch( chat )
      {
         case '"':strout += "\\\""; break;
         case '\'':strout += "\\\'"; break;
         case '\r': strout += "\\r"; break;
         case '\n': strout += "\\n"; break;
         case '\t': strout += "\\t"; break;
         case '\b': strout += "\\b"; break;
         case '\\': strout += "\\\\"; break;
         default:
            if ( chat < 32 || (chat >= 128 && full) ) {
               strout += 'x';
               strout.writeNumberHex(chat, true );
            }
            else{
               strout += chat;
            }
      }
      pos++;
   }
}

void String::unescape()
{
   length_t len = length();
   length_t pos = 0;

   while( pos < len )
   {
      char_t chat = getCharAt( pos );
      if ( chat == (char_t) '\\' )
      {
         // an escape must take place
         length_t endSub = pos + 1;
         if( endSub == len - 1 )
            return;

         char_t chnext = getCharAt( endSub );
         char_t chsub=0;

         switch( chnext )
         {
            case '"':  chsub = (uint32) '"'; break;
            case '\r': chsub = (uint32) '\r'; break;
            case '\n': chsub = (uint32) '\n'; break;
            case '\t': chsub = (uint32) '\t'; break;
            case '\b': chsub = (uint32) '\b'; break;
            case '\\': chsub = (uint32) '\\'; break;
            case '0':
               // parse octal number
               endSub ++;
               chsub = 0;
               // max lenght of octals = 11 chars, + 2 for stubs
               while( endSub < len && endSub - pos < 13 )
               {
                  chnext = getCharAt( endSub );
                  if ( chnext < 0x30 || chnext > 0x37 )
                     break;
                  chsub <<= 3; //*8
                  chsub |= (0x7) & (chnext - 0x30);
                  endSub ++;
               }
            break;

            case 'x':
               // parse exadecimal number
               endSub ++;
               chsub = 0;
               // max lenght of octals = 11 chars, + 2 for stubs
               while( endSub < len && endSub - pos < 13 )
               {
                  chnext = getCharAt( endSub );
                  if ( chnext >= 0x30 && chnext <= 0x39 ) // 0 - 9
                  {
                     chsub <<= 4; //*16
                     chsub |=  chnext - 0x30;
                  }
                  else if( chnext >= 0x41 && chnext <= 0x46 ) // A - F
                  {
                     chsub <<= 4; //*16
                     chsub |=  chnext - 0x41 + 10;
                  }
                  else if( chnext >= 0x61 && chnext <= 0x66 ) // a - f
                  {
                     chsub <<= 4; //*16
                     chsub |=  chnext - 0x61 + 10;
                  }
                  endSub ++;
               }
            break;
         }
         // substitute the char
         setCharAt( pos, chsub );
         // remove the rest
         remove( pos + 1, endSub - pos );
      }

      pos++;
   }
}


void String::serialize( DataWriter *out ) const
{
   out->write( *this );
}


bool String::deserialize( DataReader *in )
{
   in->read( *this );
   return true;
}


const char* String::c_ize() const
{
   if ( allocated() <= size() )
   {
      const_cast<String*>(this)->append( 0 );
      const_cast<String*>(this)->size( size() - m_class->charSize() );
   }
   else
   {
      const_cast<String*>(this)->setCharAt(length(), 0);
   }

   return (const char*) getRawStorage();
}

bool String::setCharSize( uint16 nsize )
{
   // same size?
   if ( nsize == m_class->charSize() )
      return true;

   // change only the manipulator?
   if( size() == 0 ) {
      m_class->destroy( this ); // dispose anyhow
      allocated(0);
      switch( nsize ) {
         case 1: m_class = &csh::handler_buffer; break;
         case 2: m_class = &csh::handler_buffer16; break;
         case 4: m_class = &csh::handler_buffer32; break;
         default: return false;
      }

      return true;
   }

   if ( nsize != 1 && nsize != 2 && nsize != 4 )
      return false;

   // full change.
   // use allocated to decide re-allocation under new char size.
   byte *mem = getRawStorage();
   uint16 oldcs = m_class->charSize();
   length_t nalloc = (allocated()/oldcs) * nsize;
   length_t oldsize = size();
   byte *nmem = (byte*) malloc( nalloc );
   csh::Base* manipulator = csh::adaptBuffer( mem, 0, oldcs, nmem, 0, nsize, length() );
   m_class->destroy( this );
   allocated( nalloc );
   size( (oldsize/oldcs)*nsize );
   m_class = manipulator;
   setRawStorage( nmem );

   return true;
}


bool String::parseInt( int64 &target, length_t pos ) const
{
   length_t len = length();
   if ( pos >= len )
      return false;

   target = 0;

   bool neg = false;

   char_t chnext = getCharAt( pos );
   if ( chnext == (char_t) '-' )
   {
      neg = true;
      pos ++;
      if ( pos == len )
         return false;
      chnext = getCharAt( pos );
   }

   // detect overflow
   int64 tgtCopy = target;
   while( chnext >= 0x30 && chnext <= 0x39 )
   {
      if( target < tgtCopy )
         return false;

      tgtCopy = target;
      target *= 10;
      target += chnext - 0x30;
      pos ++;
      if ( pos == len )
         break;
      chnext = getCharAt( pos );
   }

   if ( chnext < 0x30 || chnext > 0x39 )
      return false;

   if (neg)
      target = -target;

   return true;
}

bool String::parseDouble( double &target, length_t pos ) const
{
   char buffer[64];
   length_t maxlen = 63;

   length_t len = length();
   if ( pos >= len )
      return false;

   // if we are single byte string, just copy
   if ( m_class->charSize() == 1 )
   {
      if ( maxlen > len - pos )
         maxlen = len - pos;
      memcpy( buffer, m_storage, maxlen );
      buffer[ maxlen ] = '\0';
   }
   else {

      // else convert to C string
      length_t bufpos = 0;
      while ( bufpos < maxlen && pos < len )
      {
         char_t chr = getCharAt( pos );
         if( chr != (char_t) '-' && chr != (char_t) 'e' && chr != (char_t) 'E' &&
                  chr < 0x30 && chr > 0x39 )
            return false;
         buffer[ bufpos ] = (char) chr;
         bufpos ++;
         pos ++;
      }
   }

   // then apply sscanf
   if ( sscanf( buffer, "%lf", &target ) == 1 )
      return true;
   return false;
}


bool String::parseBin( uint64 &target, length_t pos ) const
{
   length_t len = length();
   if ( pos >= len )
      return false;
   // parse octal number
   target = 0;
   length_t endSub = pos;

   // max length of binary = 64 chars, + 2 for stubs
   while( endSub < len && (endSub - pos < 64) )
   {
      char_t chnext = getCharAt( endSub );
      if ( chnext < 0x30 || chnext > 0x31 )
         break;
      target <<= 1; //*2
      target |= (0x1) & (chnext - 0x30);
      endSub ++;
   }

   if( endSub != pos )
      return true;
   return false;
}


bool String::parseOctal( uint64 &target, length_t pos ) const
{
   length_t len = length();
   if ( pos >= len )
      return false;
   // parse octal number
   target = 0;
   length_t endSub = pos;

   // max length of octals = 11 chars, + 2 for stubs
   while( endSub < len && (endSub - pos < 26) )
   {
      char_t chnext = getCharAt( endSub );
      if ( chnext < 0x30 || chnext > 0x37 )
         break;
      target <<= 3; //*8
      target |= (0x7) & (chnext - 0x30);
      endSub ++;
   }

   if( endSub != pos )
      return true;
   return false;
}

bool String::parseHex( uint64 &target, length_t pos ) const
{
   length_t len = length();
   if ( pos >= len )
      return false;
   // parse octal number
   target = 0;
   length_t endSub = pos;

   while( endSub < len && (endSub - pos < 16) )
   {
      char_t chnext = getCharAt( endSub );
      if ( chnext >= 0x30 && chnext <= 0x39 ) // 0 - 9
      {
         target <<= 4; //*16
         target |=  chnext - 0x30;
      }
      else if( chnext >= 0x41 && chnext <= 0x46 ) // A - F
      {
         target <<= 4; //*16
         target |=  chnext - 0x41 + 10;
      }
      else if( chnext >= 0x61 && chnext <= 0x66 ) // a - f
      {
         target <<= 4; //*16
         target |=  chnext - 0x61 + 10;
      }
      endSub ++;
   }

   if( endSub != pos )
      return true;

   return false;
}

void String::writeNumber( int64 number )
{
   // prepare the buffer
   bool neg;
   char buffer[21];
   length_t pos = 19;
   buffer[20] = '\0';


   if ( number == 0 )
   {
      buffer[pos] = '0';
   }
   else {
      if ( number < 0 )
      {
         neg = true;
         number = - number;
         if ( number < 0 )
         {
            // is NAN
            append( "NaN" );
            return;
         }
      }
      else
         neg = false;

      while( number != 0 ) {
         buffer[pos--] = (char) ((number % 10) + 0x30);
         number /= 10;
      }

      if ( neg )
         buffer[ pos ] = '-';
      else
         pos++;
   }

   append( buffer + pos );
}

void String::writeNumberHex( uint64 number, bool uppercase, int ciphers  )
{
   // prepare the buffer
   char buffer[18];
   length_t pos = 16;
   buffer[17] = '\0';

   byte base = uppercase ? 0x41 : 0x61;
   if( ciphers > 16 )
   {
      ciphers = 16;
   }

   if ( number == 0 )
   {
      buffer[pos] = '0';
   }
   else {

      while( number != 0 ) {
         byte b = (byte)(number & 0xf);
         if ( b <= 9 )
            buffer[pos--] = (char) (b + 0x30);
         else {
            buffer[pos--] = (char) ( b - 10 + base );
         }

         number >>= 4;
         ciphers--;
      }
   }

   while( ciphers > 0 )
   {
      buffer[pos--] = '0';
      ciphers --;
   }

   append( buffer + pos + 1 );
}

void String::writeNumberOctal( uint64 number )
{
   // prepare the buffer
   char buffer[32];
   length_t pos = 30;
   buffer[31] = '\0';

   if ( number == 0 )
   {
      buffer[pos] = '0';
   }
   else {

      while( number != 0 ) {
         buffer[pos--] = (char) ((number & 0x7) + 0x30);
         number >>= 3;
      }
      pos++;
   }

   append( buffer + pos );
}

void String::writeNumber( int64 number, const String &format )
{
   char buffer[64];

   char bufFormat[32];
   if ( format.toCString( bufFormat, 32 ) == npos )
      return;

   sprintf( buffer, bufFormat, number );
   append( buffer );
}

void String::writeNumber( double number, const String &format )
{
   char buffer[64];

   char bufFormat[32];
   if ( format.toCString( bufFormat, 32 ) == npos )
      return;

   sprintf( buffer, bufFormat, number );
   append( buffer );
}


String &String::bufferize( const String &other )
{
   m_class->bufferize( this, &other );
   return *this;
}


String &String::bufferize()
{
   m_class->bufferize( this );
   return *this;
}


void String::trim( int mode )
{
   length_t front = 0;
   length_t len = length();

   // modes: 0 = all, 1 = front, 2 = back

   // first, trim from behind.
   if ( mode == 0 || mode == 2 ) {
      while( len > 0 )
      {
         char_t chr = getCharAt( len - 1 );
         if( chr != ' ' && chr != '\n' && chr != '\r' && chr != '\t' )
         {
            break;
         }

         len --;
      }

      if ( len == 0 )
      {
         // string is actually empty.
         m_size = 0;
         return;
      }
   }

   // front trim
   if ( mode == 0 || mode == 1 ) {
      while( front < len )
      {
         char_t chr = getCharAt( front );
         if( chr != ' ' && chr != '\n' && chr != '\r' && chr != '\t' )
         {
            break;
         }
         ++front;
      }
   }

   // front can't be == to len.
   if ( front > 0 )
   {
      // source and dest should be different, but it will work for this configuration.
      m_class->subString( this, front, len, this );
   }
   else {
      m_size = len * m_class->charSize();
   }
}

void String::lower()
{
   length_t len = length();
   for( length_t i = 0; i < len; i++ )
   {
      char_t chr = getCharAt( i );
      if ( chr >= 'A' && chr <= 'Z' ) {
         setCharAt( i, chr | 0x20 );
      }
   }
}

void String::upper()
{
   length_t len = length();
   for( length_t i = 0; i < len; i++ )
   {
      char_t chr = getCharAt( i );
      if ( chr >= 'a' && chr <= 'z' ) {
         setCharAt( i, chr & ~0x20 );
      }
   }
}


bool String::fromUTF8( const char *utf8 )
{
   return fromUTF8( utf8, -1 );
}


bool String::fromUTF8( const char *utf8, length_t len )
{
   // destroy old contents

   if ( m_allocated )
   {
      m_class->destroy( this );
      m_allocated = 0;
   }
   m_size = 0;

   // empty string?
   if ( len == 0 || *utf8 == 0 )
   {
      m_class = &csh::handler_static;
      return true;
   }

   // start scanning

   while ( len != 0 && *utf8 != 0 )
   {
      char_t chr = 0;

      byte in = (byte) *utf8;

      // 4 bytes? -- pattern 1111 0xxx
      int count;
      if ( (in & 0xF8) == 0xF0 )
      {
         chr = (in & 0x7 ) << 18;
         count = 18;
      }
      // pattern 1110 xxxx
      else if ( (in & 0xF0) == 0xE0 )
      {
         chr = (in & 0xF) << 12;
         count = 12;
      }
      // pattern 110x xxxx
      else if ( (in & 0xE0) == 0xC0 )
      {
         chr = (in & 0x1F) << 6;
         count = 6;
      }
      else if( in < 0x80 )
      {
         chr = (char_t) in;
         count = 0;
      }
      // invalid pattern
      else {
         return false;
      }

      // read the other characters with pattern 0x10xx xxxx
      while( count > 0 )
      {
         count -= 6;

         utf8++;
         byte in = (byte) *utf8;

         if ( in == 0 ) {
            // short utf8 sequence
            return false;
         }
         else if( (in & 0xC0) != 0x80 )
         {
            // unrecognized pattern, protocol error
            return false;
         }
         chr |= (in & 0x3f) << count;
      }

      this->append( chr );

      utf8++;
      --len;
   }

   return true;
}

bool String::startsWith( const String &str, bool icase ) const
{
   length_t len = str.length();
   if ( len > length() ) return false;

   if ( icase )
   {
      for ( length_t i = 0; i < len; i ++ )
      {
         char_t chr1, chr2;
         if ( (chr1 = str.getCharAt(i)) != (chr2 = getCharAt(i)) )
         {
            if ( chr1 >= 'A' && chr1 <= 'z' && (chr1 | 0x20) != (chr2|0x20) )
               return false;
         }
      }
   }
   else
   {
      for ( uint32 i = 0; i < len; i ++ )
         if ( str.getCharAt(i) != getCharAt(i) )
            return false;
   }

   return true;
}


bool String::endsWith( const String &str, bool icase ) const
{
   length_t len = str.length();
   length_t mlen = length();
   length_t start = mlen-len;

   if ( len > mlen ) return false;

   if ( icase )
   {
      for ( length_t i = 0; i < len; ++i )
      {
         char_t chr1, chr2;
         if ( (chr1 = str.getCharAt(i)) != (chr2 = getCharAt(i+start)) )
         {
            if ( ((chr1 >= 'A' && chr1 <= 'Z') || (chr1 >= 'a' && chr1 <= 'z') ) 
                  && (chr1 | 0x20) == (chr2|0x20) )
            {
               continue;
            }
            
            return false;
         }
      }
   }
   else
   {
      for ( length_t i = 0; i < len; ++i )
         if ( str.getCharAt(i) != getCharAt(i+start) )
            return false;
   }

   return true;
}

bool String::wildcardMatch( const String& wildcard, bool bIcase ) const
{
   const String* wcard = &wildcard;
   const String* cfr = this;

   length_t wpos = 0, wlen = wcard->length();
   length_t cpos = 0, clen = cfr->length();

   length_t wstarpos = 0xFFFFFFFF;

   while ( cpos < clen )
   {
      if( wpos == wlen )
      {
         // we have failed the match; but if we had a star, we
         // may roll back to the starpos and try to match the
         // rest of the string
         if ( wstarpos != 0xFFFFFFFF )
         {
            wpos = wstarpos;
         }
         else {
            // no way, we're doomed.
            break;
         }
      }

      char_t wchr = wcard->getCharAt( wpos );
      char_t cchr = cfr->getCharAt( cpos );

      switch( wchr )
      {
         case '?': // match any character
            wpos++;
            cpos++;
         break;

         case '*':
         {
            // mark for restart in case of bad match.
            wstarpos = wpos;

            // match till the next character
            wpos++;
            // eat all * in a row
            while( wpos < wlen )
            {
               wchr = wcard->getCharAt( wpos );
               if ( wchr != '*' )
                  break;
               wpos++;
            }

            if ( wpos == wlen )
            {
               // we have consumed all the chars
               cpos = clen;
               break;
            }


            //eat up to next character
            while( cpos < clen )
            {
               cchr = cfr->getCharAt( cpos );
               if ( cchr == wchr )
                  break;
               cpos ++;
            }

            // we have eaten up the same char? --  then advance also wpos to prepare next loop
            if ( cchr == wchr )
            {
               wpos++;
               cpos++;
            }
            // else, everything must stay as it is, so cpos == clen but wpos != wlen causing fail.
         }
         break;

         default:
            if ( cchr == wchr ||
                  ( bIcase && cchr < 128 && wchr < 128 && (cchr | 32) == (wchr | 32) )
               )
            {
               cpos++;
               wpos++;
            }
            else
            {
               // can we retry?
               if ( wstarpos != 0xFFFFFFFF )
                  wpos = wstarpos;
               else {
                  // check failed -- we're doomed
                  return false;
               }
            }
      }
   }

   // at the end of the loop, the match is ok only if both the cpos and wpos are at the end
   return wpos == wlen && cpos == clen;
}

void String::escapeQuotes()
{
   length_t len = length();
   length_t i = 0;

   while( i < len )
   {
      register char_t chr =  getCharAt(i);
      switch( chr )
      {
      case '\'': case '\"':
         insert( i, 0, "\\" );
         i+=2;
         ++len;
         break;

      default:
         ++i;
      }
   }
}

void String::unescapeQuotes()
{
   length_t len = length();
   length_t i = 0;
   bool state = false;

   while( i < len )
   {
      register length_t chr =  getCharAt(i);
      switch( chr )
      {
      case '\'': case '\"':
         if( state )
         {
            remove( i-1, 1 );
            --len;
         }
         break;

      case '\\':
         state = true;
         ++i;
         break;

      default:
         state = false;
         ++i;
      }
   }
}

GCToken* String::garbage()
{
   static Class* stringClass = Engine::instance()->stringClass();
   static Collector* collector = Engine::instance()->collector();

   return FALCON_GC_STORE( collector, stringClass, this );
}

}

/* end of string.cpp */