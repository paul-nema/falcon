/*
   FALCON - The Falcon Programming Language.
   FILE: item.h

   Basic Item API.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Thu, 06 Jan 2011 13:21:10 +0100

   -------------------------------------------------------------------
   (C) Copyright 2010: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/


#ifndef FLC_ITEM_H
#define FLC_ITEM_H

#include <falcon/types.h>
#include <falcon/itemid.h>
#include <falcon/string.h>
#include <falcon/class.h>
#include <falcon/gctoken.h>
#include <falcon/collector.h>
#include <falcon/engine.h>

namespace Falcon {

class Function;
class ItemArray;

/** Basic item abstraction.*/
class FALCON_DYN_CLASS Item
{
public:

  struct {
     union {
        int32 val32;
        int64 val64;
        numeric number;

        struct {
           void *pInst;
           Class *pClass;
        } ptr;

     } data;

     union {
        Function *function;
        Class *base;
        int32 ruleTop;
     } mth;

     union {
        struct {
           byte type;
           byte flags;
           byte oldType;
           byte copied;
        } bits;
        uint16 half;
        uint32 whole;
     } base;
  } content;


#ifdef _MSC_VER
	#if _MSC_VER < 1299
	#define flagIsGarbage 0x02
	#define flagIsOob 0x04
	#define flagLiteral 0x08
	#else
	   static const byte flagIsGarbage = 0x02;
	   static const byte flagIsOob = 0x04;
	   static const byte flagLiteral = 0x08;
	#endif
#else
   static const byte flagIsGarbage = 0x02;
   static const byte flagIsOob = 0x04;
   static const byte flagLiteral = 0x08;
#endif

public:
   inline Item()
   {
      type( FLC_ITEM_NIL );
   }

   inline void setNil()
   {
      type( FLC_ITEM_NIL );
   }

   inline Item( const Item &other )
   {
      other.copied(true);
      copy( other );
   }

   /** Creates a String item.
    \param str A C string that is taken as-is and converted to a Falcon String.
    
    A Falcon String is created as static (the input \b str is considered to stay
    valid throut the lifetime of the program), and then the String is sent to
    the garbage collector.

    In case the input data is transient, use the Item( const String& ) constructor
    instead, throught the following idiom:

    @code
      char *cptr = ret_transient();
      Item item( String( cptr, -1 ) );
    @endcode

    \note The operation is Encoding neuter.
    */
   Item( const char* str )
   {
      setString(str);
   }

   /** Creates a String item.
    \param str A wchar_t string that is taken as-is and converted to a Falcon String.

     A Falcon String is created as static (the input \b str is considered to stay
    valid throut the lifetime of the program), and then the String is sent to
    the garbage collector.

    In case the input data is transient, use the Item( const String& ) constructor
    instead, throught the following idiom:

    @code
      wchar_t *cptr = ret_transient();
      Item item( String( cptr, -1 ) );
    @endcode

    \note The operation is Encoding neuter (the input is considered UDF).
    */
   Item( const wchar_t* str )
   {
      setString(str);
   }

   /** Creates a String item.
    \param str A Falcon string that will be copied.

    The deep contents of the string are copied only if the string is
    buffered; static strings (strings created out of static char* or wchar_t*
    data) are not deep-copied.

    The created string is subject to garbage collecting.
    */
   Item( const String& str )
   {
      setString(str);
   }

   /** Creates a String item, adopting an existing string.
    \param str A pointer to a Falcon String that must be adopted.

    The string is taken as-is and stored for garbage in the Falcon engine.
    This constructor is useful when a String* has been created for another reason,
    and then must be sent to the engine as a Falcon item. In this way, the
    copy (and eventual deep-copy) of the already created string is avoided.
    
    */
   Item( String* str )
   {
      setString(str);
   }

   /** Sets this item to a String item.
    \param str A C string that is taken as-is and converted to a Falcon String.
    \see Item( char* )
    */
   void setString( const char* str );

   /** Sets this item to a String item.
    \param str A wchar_t string that is taken as-is and converted to a Falcon String.
    \see Item( wchar_t* )
    */
   void setString( const wchar_t* str );

   /** Sets this item to a String item.
    \param str A Falcon string that will be copied.
    \see Item( const String& )
    */
   void setString( const String& str );

   /** Sets this item to a String item.
    \param str A pointer to a Falcon String that must be adopted.
    \see Item( String* )
    */
   void setString( String* str );

   /** Creates a boolean item. */
   explicit inline Item( bool b ) {
      setBoolean( b );
   }

   /** Sets this item as boolean */
   inline void setBoolean( bool tof )
   {
      type( FLC_ITEM_BOOL );
      content.data.val32 = tof? 1: 0;
   }

   /** Creates an integer item */
   inline Item( int32 val )
   {
      setInteger( (int64) val );
   }

   /** Creates an integer item */
   inline Item( int64 val )
   {
      setInteger( val );
   }

   inline void setInteger( int64 val ) {
      type(FLC_ITEM_INT);
      content.data.val64 = val;
   }

   /** Creates a numeric item */
   inline Item( numeric val )
   {
      setNumeric( val );
   }

   inline void setNumeric( numeric val ) {
      type( FLC_ITEM_NUM );
      content.data.number = val;
   }

   inline Item( Function* f )
   {
      setFunction(f);
   }

   inline void setFunction( Function* f )
   {
      type( FLC_ITEM_FUNC );
      content.data.ptr.pInst = f;
   }

   inline Item( Class* cls, void* inst ) {
       setUser( cls, inst );
   }

   inline void setUser( const Class* cls, void* inst )
   {
       type( FLC_ITEM_USER );
       content.data.ptr.pInst = inst;
       content.data.ptr.pClass = (Class*) cls;
   }

   inline Item( GCToken* token )
   {
      setUser( token );
   }
   
   inline void setUser( GCToken* token )
   {
       type( FLC_ITEM_USER );
       content.base.bits.flags |= flagIsGarbage;
       content.data.ptr.pInst = token->data();
       content.data.ptr.pClass = token->cls();
   }

   /** Declare this item a garbage-sensible item.
    This flags has effect only for USER type items.
    */
   inline void garbage() { content.base.bits.flags |= flagIsGarbage; }

   /** Declare this item a garbage.
    This flags has effect only for USER type items.
    */
   inline bool isGarbaged() const {
      return (content.base.bits.flags & flagIsGarbage) != 0;
   }

   inline void methodize( Function* mthFunc )
   {
       content.mth.function = mthFunc;
       content.base.bits.oldType = content.base.bits.type;
       content.base.bits.type = FLC_ITEM_METHOD;
   }

   inline void methodize( Class* mthClass )
   {
       content.mth.base = mthClass;
       content.base.bits.oldType = content.base.bits.type;
       content.base.bits.type = FLC_ITEM_BASEMETHOD;
   }

   inline void unmethodize()
   {
       content.base.bits.type = content.base.bits.oldType;
   }

   inline void getMethodItem( Item& tgt )
   {
      tgt.content.base.bits.type = content.base.bits.oldType;
      tgt.content.base.bits.flags = 0;
      tgt.content.data.ptr.pInst = content.data.ptr.pInst;
      tgt.content.data.ptr.pClass = content.data.ptr.pClass;
   }

   /** Turn this item in to an array (deep).
    \param array The array handled to the vm.
    
    This method turns the item into an array with deep semantics (i.e.
    controlled by the garbage collector.

    For user-semantic (i.e. user-controlled) you must use directly the
    setUser() method.
    */
   void setArray( ItemArray* array );

   /** Turn this item in to a dictionary (deep).
    \param dict The dictionary handled to the vm
    
    This method turns the item into an dictionary with deep semantics (i.e.
    controlled by the garbage collector.

    For user-semantic (i.e. user-controlled) you must use directly the
    setUser() method.
    */
   //void setDict( ItemDictionary* dict );

   /** Defines this item as a out of band data.
      Out of band data allow out-of-order sequencing in functional programming.
      If an item is out of band, it's type it's still the original one, and
      its value is preserved across function calls and returns; however, the
      out of band data may innescate a meta-level processing of the data
      travelling through the functions.

      In example, returning an out of band NIL value from an xmap mapping
      function will cause xmap to discard the data.
   */
   void setOob() { content.base.bits.flags |= flagIsOob; }

   /** Clear out of band status of this item.
      \see setOob()
   */
   void resetOob() { content.base.bits.flags &= ~flagIsOob; }

   /** Toggle out of band status of this item.
      \see setOob() resetOob()
   */
   void xorOob() { content.base.bits.flags ^= flagIsOob; }

   /** Sets or clears the out of band status status of this item.
      \param oob true to make this item out of band.
      \see setOob()
   */
   void setOob( bool oob ) {
      if ( oob )
         content.base.bits.flags |= flagIsOob;
      else
         content.base.bits.flags &= ~flagIsOob;
   }

   /** Tells wether this item is out of band.
      \return true if out of band.
      \see oob()
   */
   bool isOob() const { return (content.base.bits.flags & flagIsOob )== flagIsOob; }

   /** Returns the item type*/
   byte type() const { return content.base.bits.type; }
   
   /** Changes the item type.
   
      Flags are also reset. For example, if this item was OOB before,
      the OOB flag is cleared.

      The copy marker is cleared as well.
   */
   void type( byte nt ) { 
      content.base.bits.copied = false;
      content.base.bits.flags = 0;
      content.base.bits.type = nt;
   }

   /** Returns true if this item has the copy-marker.
    \return true if copied.
    */
   bool copied() const { return content.base.bits.copied; }

   /** Sets the copy mode.
      \note the method is marked "const" because it operates on
      a "virtually mutable" copied marker.
    */
   void copied( bool bMode ) const {
      const_cast<Item*>(this)->content.base.bits.copied = bMode;
   }

   /** Copies the full contents of another item.
      \param other The copied item.

    This performs a full flat copy of the original item.

    \note This doesn't set the copy marker; the copy marker is meant to determine
    if the VM has copied an item a script level while basic copy operations
    may not have this semantic at lower level. For example, one may make a
    temporary flat copy of a stack item without willing to notify that to the
    VM.

    The copy mark is actually copied as any other flag or value in the original
    item. To explicitly declare the original item copied, use the assign()
    method or used the explicit copied(bool) method.
    */
   void copy( const Item &other )
   {
      #ifdef _SPARC32_ITEM_HACK
      register int32 *pthis, *pother;
      pthis = (int32*) this;
      pother = (int32*) &other;
      pthis[0]= pother[0];
      pthis[1]= pother[1];
      pthis[2]= pother[2];
      pthis[3]= pother[3];

      #else
         content = other.content;
      #endif
   }

   /** Assign an item to this item.    
    This operation resolves into marking the source item as copied, and then
    applying the copy item.
    */
   void assign( const Item& other )
   {
      other.copied(true);
      copy(other);
   }

   bool asBoolean() const { return content.data.val32 != 0; }
   int64 asInteger() const { return content.data.val64; }
   numeric asNumeric() const { return content.data.number; }
   Function* asFunction() const { return static_cast<Function*>(content.data.ptr.pInst); }

   Function* asMethodFunction() const { return content.mth.function; }
   Class* asMethodClass() const { return content.mth.base; }

   void* asInst() const { return content.data.ptr.pInst; }
   Class* asClass() const { return content.data.ptr.pClass; }

   /** Convert current object into an integer.
      This operations is usually done on integers, numeric and CoreStrings.
      It will do nothing meaningfull on other types.
   */
   int64 forceInteger() const ;

   /** Convert current object into an integer.
      This operations is usually done on integers, numeric and CoreStrings.
      It will do nothing meaningfull on other types.

      \note this version will throw a code error if the item is not an ordinal.
   */
   int64 forceIntegerEx() const ;

   /** Convert current object into a numeric.
      This operations is usually done on integers, numeric and CoreStrings.
      It will do nothing meaningfull on other types.
   */
   numeric forceNumeric() const ;

   bool isNil() const { return type() == FLC_ITEM_NIL; }
   bool isBoolean() const { return type() == FLC_ITEM_BOOL; }
   bool isInteger() const { return type() == FLC_ITEM_INT; }
   bool isNumeric() const { return type() == FLC_ITEM_NUM; }
   bool isFunction() const { return type() == FLC_ITEM_FUNC; }
   bool isMethod() const { return type() == FLC_ITEM_METHOD; }
   bool isBaseMethod() const { return type() == FLC_ITEM_BASEMETHOD; }
   bool isOrdinal() const { return type() == FLC_ITEM_INT || type() == FLC_ITEM_NUM; }
   bool isUser() const { return type() == FLC_ITEM_USER; }

   bool isString() const {
      return (type() == FLC_ITEM_USER && asClass()->typeID() == FLC_CLASS_ID_STRING);
   }

   bool isArray() const {
      return (type() == FLC_ITEM_USER && asClass()->typeID() == FLC_CLASS_ID_ARRAY);
   }

   bool isDict() const {
      return (type() == FLC_ITEM_USER && asClass()->typeID() == FLC_CLASS_ID_DICT);
   }

   bool isTrue() const;

   /** Turns the item into a string.
    This method turns the item into a string for a minimal external representation.

    In case the item is deep, it will use the Class::describe() member to obtain
    a representation.
    */
   void describe( String& target, int depth = 3, int maxLen = 60 ) const;

   String describe( int depth = 3, int maxLen = 60) const {
      String t;
      describe(t, depth, maxLen);
      return t;
   }

   /** Operator version of copy.
    \note This doesn't set the copy marker. Use assign() for that.
    */
   Item &operator=( const Item &other ) { copy( other ); return *this; }
   bool operator==( const Item &other ) const { return compare(other) == 0; }
   bool operator!=( const Item &other ) const { return compare(other) != 0; }
   bool operator<(const Item &other) const { return compare( other ) < 0; }
   bool operator<=(const Item &other) const { return compare( other ) <= 0; }
   bool operator>(const Item &other) const { return compare( other ) > 0; }
   bool operator>=(const Item &other) const { return compare( other ) >= 0; }

   bool exactlyEqual( const Item &other ) const;
   int compare( const Item& other ) const;

   /** Flags, used for internal vm needs. */
   byte flags() const { return content.base.bits.flags; }
   void flags( byte b ) { content.base.bits.flags = b; }
   void flagsOn( byte b ) { content.base.bits.flags |= b; }
   void flagsOff( byte b ) { content.base.bits.flags &= ~b; }


   /** Clone the item.
      If the item is not cloneable, the method returns false. Is up to the caller to
      raise an appropriate error if that's the case.
      The VM parameter may be zero; in that case, returned items will not be stored in
      any garbage collector.

      Reference items are de-referenced; if cloning a reference, the caller will obtain
      a clone of the target item, not a clone of the reference.

      Also, in that case, the returned item will be free of reference.

      \param target the item where to stored the cloned instance of this item.
      \return true if the clone operation is possible
   */
   bool clone( Item &target ) const;


   /** Gets the class and the instance from a deep item.
    \param cls The class of this deep or user item.
    \param udata The user data associated with this item.
    \return false if the item is not deep or user.

    This method simplifies the detection of deep items and the
    extraction of their vital elements (user data and handling class).

    This method fails if the object is flat. A more generic function is
    forceClassInst() that will return the handler class also for simple items;
    however forceClassInst() is slower.
    */
   bool asClassInst( Class*& cls, void*& udata ) const
   {
      switch( type() )
      {
      case FLC_ITEM_USER:
         cls = asClass();
         udata = asInst();
         return true;
      }
      return false;
   }

   
   /** Gets the class and instance from any item.
     \param cls The class of this deep or user item.
     \param udata The user data associated with this item or 0 if this item is flat.

    In case of flat items, a slower call is made to identify the
    base handler class, and that is returned. In that case, the udata*
    is set to "this" pointer (items always start with the data, then
    an optional method pointer, and finally the type and description portion,
    so this is consistent even when the handler class is expecting a something
    that's not necessarily an Item*).
   */
   void forceClassInst( Class*& cls, void*& udata ) const
   {
      switch( type() )
      {
      case FLC_ITEM_USER:
         cls = asClass();
         udata = asInst();
         break;

      case FLC_ITEM_FUNC:
         cls = Engine::instance()->getTypeClass(type());
         udata = asFunction();
         break;

      default:
         cls = Engine::instance()->getTypeClass(type());
         udata = (void*)this;
      }
   }

   //===================================================================
   // Is string?
   //
   
   bool isString( String*& str )
   {
      Class* cls;
      if ( asClassInst( cls, (void*&)str ) )
      {
         return cls->typeID() == FLC_CLASS_ID_STRING;
      }
      return false;
   }

   String* asString()
   {
      Class* cls;
      void* udata;
      if ( asClassInst( cls, udata ) )
      {
         if( cls->typeID() == FLC_CLASS_ID_STRING )
            return static_cast<String*>(udata);
      }

      return 0;
   }

   //======================================================
   // Utilities
   //

   int64 len() const;

};

}

#endif
/* end of item.h */