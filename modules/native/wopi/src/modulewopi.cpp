/*
   FALCON - The Falcon Programming Language.
   FILE: modulewopi.cpp

   Web Oriented Programming Interface main module
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Fri, 04 Oct 2013 14:34:08 +0200

   -------------------------------------------------------------------
   (C) Copyright 2013: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/process.h>

#include <falcon/wopi/modulewopi.h>
#include <falcon/wopi/wopi.h>
#include <falcon/wopi/classwopi.h>
#include <falcon/wopi/request.h>
#include <falcon/wopi/reply.h>
#include <falcon/wopi/replystream.h>

#include <falcon/wopi/classrequest.h>
#include <falcon/wopi/classreply.h>
#include <falcon/wopi/classuploaded.h>
#include <falcon/wopi/classwopi.h>

namespace Falcon {
namespace WOPI {

ModuleWopi::ModuleWopi( const String& name )
{
   m_process = 0;
   m_oldStdout = 0;
   m_oldStderr = 0;

   m_request = new Request(this);
   m_reply = new Reply(this);

   m_wopi = new Wopi;
   m_provider = name;

   m_classWopi = new ClassWopi;
   m_classUploaded = new ClassUploaded;

   Class* requestClass = new ClassRequest;
   Class* replyClass = new ClassReply;
   *this << requestClass
         << replyClass
         << m_classWopi
         << m_classUploaded
   ;

   // set the global items without creating the singletons.
   addGlobal("Request", FALCON_GC_STORE(requestClass, m_request), true );
   addGlobal("Reply", FALCON_GC_STORE(replyClass, m_reply), true );
   addGlobal("Wopi", FALCON_GC_STORE(m_classWopi, m_wopi), true );
}

ModuleWopi::~ModuleWopi()
{
   delete m_wopi;
   delete m_request;
   delete m_reply;
}


void ModuleWopi::resumeOutputStreams()
{
   if( m_process != 0 ) {
        m_oldStdout->decref();
        m_oldStderr->decref();
   }
}


void ModuleWopi::interceptOutputStreams( Process* prc )
{
   m_oldStdout = prc->stdOut();
   m_oldStderr = prc->stdErr();

   m_oldStdout->incref();
   m_oldStderr->incref();

   prc->stdOut( new ReplyStream(m_reply, m_oldStdout) );
   prc->stdErr( new ReplyStream(m_reply, m_oldStderr) );
}
}
}

/* modulewopi.cpp */