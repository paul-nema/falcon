/*
   Mini XML lib PLUS for C++

   Error class

   Author: Giancarlo Niccolai <gian@niccolai.ws>

*/

#ifndef MXML_ERROR_H
#define MXML_ERROR_H

#include <mxml_element.h>
#include <falcon/string.h>
#include <falcon/falcondata.h>

namespace MXML {

typedef enum {
   malformedError=1,
   ioError,
   notFoundError
} errorType;

class Error: public Falcon::FalconData
{
private:
   int m_code;
   int m_beginLine;
   int m_beginChar;
   int m_line;
   int m_char;

public:

/** Error codes
   This define what kind of error happened while decoding the XML document
*/
enum codes
{
   errNone = 0,
   errIo,
   errNomem,
   errOutChar,
   errInvalidNode,
   errInvalidAtt,
   errMalformedAtt,
   errInvalidChar,
   errUnclosed,
   errUnclosedEntity,
   errWrongEntity,
   errChildNotFound,
   errAttrNotFound,
   errHyerarcy,
   errCommentInvalid,
   errMultipleXmlDecl
};

protected:
   Error( const codes code, const Element *generator );

public:
   virtual ~Error();
   virtual const errorType type() const = 0;
   int numericCode() const;
   const Falcon::String description() const;
   void toString( Falcon::String &target ) const;
   void describeLine( Falcon::String &target ) const;

   const Falcon::String describeLine() const { Falcon::String s; describeLine(s); return s; }

   virtual void gcMark( Falcon::uint32 mk ) {};
   virtual Falcon::FalconData *clone() const { return 0; }
};


class MalformedError: public Error
{
public:
   MalformedError( const codes code, const Element *generator ):
      Error( code, generator ) {};
   virtual const errorType type() const  { return malformedError; }
};

class IOError: public Error
{
public:
   IOError( const codes code, const Element *generator  ):
      Error( code, generator ) {};
   virtual const errorType type() const { return ioError; }
};

class NotFoundError: public Error
{
public:
   NotFoundError( const codes code, const Element *generator ):
      Error( code, generator ) {};
   virtual const errorType type() const { return notFoundError; }
};

}

#endif
