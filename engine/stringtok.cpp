/*
   FALCON - The Falcon Programming Language.
   FILE: stringtok.cpp

   String tokenizer general purpose class.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 05 Apr 2014 15:01:12 +0200

   -------------------------------------------------------------------
   (C) Copyright 2014: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#define SRC "engine/stringtok.cpp"

#include <falcon/stringtok.h>
#include <falcon/stderrors.h>

namespace Falcon {

StringTokenizer::StringTokenizer( const String& source, const String& token, bool bGroupTokens ):
   m_source(source),
   m_pos(0),
   m_count(0),
   m_limit(0xffffffff),
   m_bGroupTokens(bGroupTokens)
{
   if( token.empty() )
   {
      throw FALCON_SIGN_XERROR(ParamError, e_inv_params, .extra("Empty token in tokenizer") );
   }

   if( token.length() == 1 )
   {
      m_chr = token.getCharAt(0);
      m_bIsChr = true;
   }
   else {
      m_bIsChr = false;
      m_token = token;
   }
}


StringTokenizer::StringTokenizer( const StringTokenizer& other ):
   m_source( other.m_source ),
   m_token( other.m_token ),
   m_chr( other.m_chr ),
   m_pos( other.m_pos ),
   m_count( other.m_count ),
   m_limit( other.m_limit ),
   m_bGroupTokens( other.m_bGroupTokens ),
   m_bIsChr( other.m_bIsChr )
{
}


bool StringTokenizer::next( String& target )
{
   if( m_pos >= m_source.length() )
   {
      return false;
   }

   // hit limit? -- return the rest.
   if( m_count == m_limit )
   {
      target = m_source.subString(m_pos);
      // from now on, we have no next.
      m_pos = String::npos;
      // anyhow account the find.
      m_count++;
      return true;
   }

   if( m_bIsChr )
   {
      nextChr( target );
   }
   else {
      nextStr( target );
   }

   ++m_count;

   return true;
}


bool StringTokenizer::hasNext() const
{
   return m_pos < m_source.length();
}


void StringTokenizer::nextChr( String& target )
{
   uint32 nextPos = m_source.find(m_chr,m_pos);
   target = m_source.subString(m_pos,nextPos);

   if (nextPos != String::npos)
   {
      ++nextPos;
      if( m_bGroupTokens )
      {
         uint32 len = m_source.length();
         while( nextPos < len && m_source.getCharAt(nextPos) == m_chr )
         {
            ++nextPos;
         }
      }
   }

   m_pos = nextPos;
}


void StringTokenizer::nextStr( String& target )
{
   uint32 nextPos = m_source.find(m_token,m_pos);
   target = m_source.subString(m_pos,nextPos);

   if (nextPos != String::npos)
   {
      uint32 tokLen = m_token.length();
      nextPos += tokLen;

      if( m_bGroupTokens )
      {
         uint32 len = m_source.length();
         uint32 tokPos = 0;
         while(
                  nextPos + tokLen < len
               && tokPos < tokLen
               && m_source.getCharAt(nextPos + tokPos) == m_token.getCharAt(tokPos) )
         {
            if( ++tokPos == tokLen )
            {
               nextPos += tokLen;
               tokPos = 0;
            }
         }
      }
   }

   m_pos = nextPos;
}

}

/* end of stringtok.cpp */