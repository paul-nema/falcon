/*
   FALCON - The Falcon Programming Language.
   FILE: stream.cpp

   Base class for I/O operations.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 12 Mar 2011 13:00:57 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Implementation of common stream utility functions
*/

#include <falcon/stream.h>
#include <falcon/error.h>
#include <falcon/unsupportederror.h>

namespace Falcon {

Stream::Stream():
   m_status( t_none ),
   m_lastError( 0 ),
   m_bShouldThrow( false )
{}

Stream::Stream( const Stream &other ):
   m_status( other.m_status ),
   m_lastError( other.m_lastError ),
   m_bShouldThrow(other.m_bShouldThrow)
{}

Stream::~Stream()
{
}

void Stream::throwUnsupported()
{
   status( status() & t_unsupported );
   throw new UnsupportedError( ErrorParam( e_io_unsup, __LINE__, __FILE__ ) );
}

}

/* end of stream.cpp */