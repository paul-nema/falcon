/*
   FALCON - The Falcon Programming Language
   FILE: stdstreams_win.cpp

   Windows specific standard streams factories.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: ven ago 25 2006

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/


#include <falcon/stdstreams.h>
#include <falcon/fstream_win.h>
#include <falcon/ioerror.h>

#include <windows.h>

namespace Falcon {

inline WinFStreamData* make_handle( HANDLE orig_handle, bool bDup )
{
   HANDLE hTarget;
   
   if ( bDup )
   {
      HANDLE curProc = GetCurrentProcess();
      BOOL bRes = ::DuplicateHandle(
                    curProc, 
                    orig_handle, 
                    curProc,
                    &hTarget, 
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
      if( ! bRes )
      {
         throw new IOError( ErrorParam(e_io_dup, __LINE__, __FILE__)
            .sysError(::GetLastError()) );
      }
   }
   else
   {
      hTarget = orig_handle;
   }

   return new WinFStreamData( hTarget, false );
}


StdInStream::StdInStream( bool bDup ):
ReadOnlyFStream(make_handle(GetStdHandle(STD_INPUT_HANDLE), bDup ))
{}

StdOutStream::StdOutStream( bool bDup ):
   WriteOnlyFStream(make_handle(GetStdHandle(STD_OUTPUT_HANDLE), bDup ))
{}

StdErrStream::StdErrStream( bool bDup ):
   WriteOnlyFStream(make_handle(GetStdHandle(STD_ERROR_HANDLE), bDup ))
{}

}


/* end of stdstreams_win.cpp */