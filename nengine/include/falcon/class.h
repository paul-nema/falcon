/*
   FALCON - The Falcon Programming Language.
   FILE: class.h

   Class definition of a Falcon Class
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Thu, 06 Jan 2011 15:01:08 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Core Class definition
*/

#ifndef FALCON_CLASS_H
#define FALCON_CLASS_H

#include <falcon/setup.h>
#include <falcon/string.h>
#include <falcon/enumerator.h>

namespace Falcon {

class VMContext;
class Item;
class DataReader;
class DataWriter;
class Module;

/** Representation of classes, that is item types.

 In falcon, each item has a type, which refers to a class.

 The Class represents the operations that can be preformed
 on a certain instance.
 
 To publish an item to the Virtual Machine, the calling program
 must create a class that instructs the VM about how the items
 of that type must be handled.
 
 Falcon::Class instances take care of the creation of objects, of their serialization
 and of their disposal. It is also responsible to check for properties

 You may wish to use the OpToken class to simplify the writing of operators in
 subclasses.

 \section implementing_operators Implementing operators

 At Falcon language level, operator overload is performed by specializing
 the operator methods in this class (the methods named op_*).

 All this methods have a prototype modelled after the following:

 @code
 virtual void op_something( VMContext* ctx, void* self );
 @endcode

 Each operator receives an instance of the real object that this Class
 manipulates in the \b self pointer and the virtual machine that requested
 the operation to be performed.

 Other than this, operands have also some parameters that are Item instances
 buried in the virtual machine stack in the moment they are called. Instead
 of unrolling this stack and passing the operand items to the method,
 it's safer and more efficient to keep them in the stack and let the operators
 to access their operands.

 There are several means to do that.

 The operator implementor may use the
 OpToken class that takes care of automaticly get the correct operand and
 unroll the stack correctly at the end of the operation.

 When maximum performance is needed, the VMachine::operands to get the operands
 and VMachine::stackResult to set a result for this operation.

 Finally, it is possible to access the operands directly from the current
 context stack.

 At low level, each operator MUST pop all its operands and MUST push exactly
 one result (or, pop all its operands - 1 and set the operand left in the stack
 as the result of the operation).

 Operators may go deep, calling the VMachine or other code that may in turn
 call the VMachine. In that case, the stack clean step should be performed
 by the deep step, before exiting the code frame.

 \note Usually the first operand is also the item holding the \b self instance.
 However, self is always passed to the operands because the VM did already some
 work in checking the kind of item stored as the first operand. However, the
 callee may be interested in accessing the first operand as well, as the Item
 in that place may hold some information (flags, out-of-band, copy marker and
 so on) that the implementor may find useful.

 Notice that \b self is set to 0 when the class is a flat item reflection core
 class (as the Integer, Nil, Boolean and Numeric handlers). It receives a value
 only if the first operand is a FLC_USER_ITEM or FLC_DEEP_ITEM.
 
 \see OpToken
*/

class FALCON_DYN_CLASS Class
{
public:

   /** Creates an item representation of a live class.
      The representation of the class needs a bit of extra informations that are provided by the
      virtual machine, other than the symbol that generated this item.
      The module id is useful as this object often refers to its module in the VM. Having the ID
      recorded here prevents the need to search for the live ID in the VM at critical times.

       By default the TypeID of a class is -1, meaning undefined. Untyped class instances
       require direct management through the owning core class, while typed class instances
       can use their type ID to make some reasoning about their class.
    *
    *  Falcon system uses this number in quasi-type classes, as arrays or dictionaries,
    * but the ID is available also for user application, starting from the baseUserID number.
    */
   Class( const String& name );
   
   /** Creates a class defining a type ID
    \param name The name of the class.
    \param the Type ID of the items created by this class.

    This constructor creates a class that is able to manage basic
    Falcon types. The TID associated with this is the numeric type
    ID of the datatype that are created as instances.

    Strings, Arrays, Dictionaries, Ranges and even Integer or NIL are
    all item types that have a class offering a TID.
    */
   Class( const String& name, int64 tid );
   
   virtual ~Class();

   const static int64 baseUserID = 100;

   int64 typeID() const { return m_typeID; }
   const String& name() const { return m_name; }

   /** Sets the module of this class.
    \param m The module where this class resides.
    */
   void module( Module* m ) { m_module = m; }
   
   /** Returns the module of this class.
    \return The module where this class resides, or 0 if the class is module-less.
    */
   Module* module() const { return m_module; }

   //=========================================
   // Instance management

   /** Disposes an instance.

    */
   virtual void dispose( void* self ) const = 0;

   /** Clones an instance */
   virtual void* clone( void* source ) const = 0;

   /** Serializes an instance */
   virtual void serialize( DataWriter* stream, void* self ) const = 0;

   /** Deserializes an instance.
      The new instance must be initialized and ready to be "selfed".
   */
   virtual void* deserialize( DataReader* stream ) const = 0;

   //=========================================================
   // Class management
   //

   /** Marks an instance.
    \parm self The intance of this class to be marked.
    \param mark The gc mark to be applied.

    This method is called every time an item with mark sign is inspected.

    The class has here the ability to set a mark flag on the self instance,
    and eventually propagate the mark to other contents or data related
    to the self instance.

    \note The base version does nothing.
    */
   virtual void gcMark( void* self, uint32 mark ) const;

   /** Determines if an instance should be disposed of.
    \parm self The intance of this class to be marked.
    \param mark The current gc Mark.
    \return True if the instance is alive, false if it can be disposed.

    This method is invoked when the garbage collector wants to check if
    an instance is still alive or needs to be disposed. The class should
    check the incoming mark against the last mark that has been applied
    during a gcMark() call. If the \b mark parameter is bigger, then
    the object has been left behind and can be disposed at class will.

    Returning false, the garbage collector will free its own accounting
    resources and won't call the gcCheck() method anymore. The class should
    dispose the instance on its own will before returning (the garbage
    collector will \b not call the dispose() method of this class after
    a gcCheck).

    Notice that disposing of the \b self parameter without returning false
    might make the GC to present the same item again to this class, possibly
    causing crashes (double free, illegal memory access etc.).

    \note The base version does nothing.
    */
   virtual bool gcCheck( void* self, uint32 mark ) const;


   /** Called back when this class is subject to GC as a metaclass data.
      \param The mark of this class.

    Classes may themselves be data for other higher-level metaclasses
    (i..e the ClassClass handler). Dynamic classes are subject to GC
    as any other dynamic data.

    This method is called by ClassClass and other
    class-type handlers, as well as some deep instances that know that their
    class could theoretically be dynamic.

    The base class sets an hidden mark value in this class.
    */
   virtual void gcMarkMyself( uint32 mark );

   /** Called back when this class is subject to GC as a metaclass data.
      \param The mark of this class.

    Classes may themselves be data for other higher-level metaclasses
    (i..e the ClassClass handler). Dynamic classes are subject to GC
    as any other dynamic data.

    This method is called back by metaclass handlers when a GC-enabled data
    is checked.

    The base class behavior is that to destroy itself and return false if
    \b mark is greater than the last mark seen in gcMarkMyself.
    */
   virtual bool gcCheckMyself( uint32 mark );

   /** Callback receiving all the properties in this class. */
   typedef Enumerator<String> PropertyEnumerator;

   /** List the properties in this class.
     @param self The object for which the properties have been requested.
     @param cb A callback function receiving one property at a time.

    The method may be called with a "self" 0 if this class has invariant
    instances.
     @note This base class implementation does nothing.
    */
   virtual void enumerateProperties( void* self, PropertyEnumerator& cb ) const;
   
   /** Return true if the class provides the given property.
      @param self The object for which the properties have been requested.

      The method may be called with a "self" 0 if this class has invariant
      instances.
    */
   virtual bool hasProperty( void* self, const String& prop ) const;

   /** Return a (possibly deep) summary or description of an instance.
      \param instance The instance to be described.
      \param target Where to place the rendered string.
      \param depth Maximum depth in recursions.
      \param maxlen Maximum length.
    
    This method returns a string containing a representation of this item. If
    the item is deep (an array, an instance, a dictionary) the contents are
    also passed through this function.

    This method traverses arrays and items deeply; there isn't any protection
    against circular references, which may cause endless loop. However, the
    default maximum depth is 3, which is a good depth for debugging (goes deep,
    but doesn't dig beyond average interesting points). Set to -1 to
    have infinite depth.

    By default, only the first 60 characters of strings and elements of membufs
    are displayed. You may change this default by providing a maxLen parameter.

    \note Differently from toString, this method ignores string rendering
    that may involve the virtual machine. Even if describe() is defined as a
    method that might be invoked by the VM, that will be ignored in
    renderings of items in a container.
    */
   virtual void describe( void* instance, String& target, int depth = 3, int maxlen = 60 ) const;

   //=========================================================
   // Operators.
   //
   
   /** Invoked by the VM to create an instance.
    \param VM a virtual machine invoking the object creation.
    \param pcount Number of parameters passed in the init request.

    This method is invoked by the VM when it requires an intance to be
    created by this class.

    The operator must reduce the stack of pcount elements and add the
    nelwy created instance as a Falcon Item on the stack. The instance may
    be either created as garbage sensible (DeepData) or as "static" UserData.

    If the method is not able to perform one of this operations, it must
    generate an exception.

    If the method needs to invoke other VM operations, the method can
    push a pstep to be called afterwards (see Context::ifDeep), and retard
    the generation of the instance at a later moment.
    */
   virtual void op_create( VMContext* ctx, int32 pcount ) const;

   /** Called back when the VM wants to negate an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.

    */
   virtual void op_neg( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to add something.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_add( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to subtract something.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_sub( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to multiply something.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_mul( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to divide something.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_div( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to apply the modulo operator.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_mod( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to apply the power operator on.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    */
   virtual void op_pow( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to add something to an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
    */
   virtual void op_aadd( VMContext* ctx, void* self) const;

   /** Called back when the VM wants to subtract something to an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
    */
   virtual void op_asub( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to multiply something to an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
    */
   virtual void op_amul( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to divide something to an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
    */
   virtual void op_adiv( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to apply the modulo something to an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance in op1 (or 0 on flat items)
     \param op1 The original first operand (where the instasnce was stored),
      that is also the place where to store the result.
     \param op2 The original second operand.
    */
   virtual void op_amod( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to apply get the power of an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
   */
   virtual void op_apow( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to increment (prefix) an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance in op1 (or 0 on flat items)
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.
   */
   virtual void op_inc( VMContext* vm, void* self ) const;

   /* Called back when the VM wants to decrement (prefix) an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance in op1 (or 0 on flat items)
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.
   */
   virtual void op_dec(VMContext* vm, void* self) const;

   /** Called back when the VM wants to increment (postfix) an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance in op1 (or 0 on flat items)
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.
   */
   virtual void op_incpost(VMContext* vm, void* self ) const;
   
   /** Called back when the VM wants to decrement (postfix) an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance in op1 (or 0 on flat items)
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.
   */
   virtual void op_decpost(VMContext* vm, void* self ) const;

   /** Called back when the VM wants to get an index out of an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    The first operand is self, the second operand is the index to be accessed.
   */
   virtual void op_getIndex(VMContext* vm, void* self ) const;
   
   /** Called back when the VM wants to get an index out of an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is ternary -- requires OpToken with 3 parameters.

    Operands are in order:
    - The item holding this \b self instance
    - The index being accessed (the value inside the [] brackets)
    - The new value for the item.

    Normally, the value of the operation should match with the value of the
    new item stored at the required index. For instance, the value of the
    following Falcon code:

    @code
    value = (array[10] = "something")
    @endcode

    is expected to be "something", but this method may set it to any sensible
    value the implementor wants to pass back.

   */
   virtual void op_setIndex(VMContext* vm, void* self ) const;

   /** Called back when the VM wants to get the value of a property of an item
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)
    \param prop The oroperty to be accessed.

    \note The operand is unary -- requires OpToken with 1 parameter.
   */
   virtual void op_getProperty( VMContext* ctx, void* self, const String& prop) const;

   /** Called back when the VM wants to set a value of a property in an item.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.
   */
   virtual void op_setProperty( VMContext* ctx, void* self, const String& prop ) const;

   /** Called back when the VM wants to compare an item to this instance. 
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    This method is called back on any comparison operation, including the six
    basic comparison operators (<, >, <=, >=, == and !=) but also in other
    ordering operations thar involve the virtual machine.

    The result of the operation should be
    - 0 if the two items are considered identical,
    - \< 0 if op1 is smaller than op2,
    - \> 0 if op1 is greater than op2.

    All the subclasses should call the base class op_compare as a residual
    criterion.
    */
   virtual void op_compare( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to know if an item is true.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.

    This method is called back when the VM wants to know if an item can be
    considered true.

    The result placed in target should be a boolean true or false value.
    */
   virtual void op_isTrue( VMContext* ctx, void* self ) const;

   /** Called back when the VM wants to know if an item is true.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items)

    \note The operand is binary -- requires OpToken with 2 parameters.

    This method is called back when the VM wants to know if an item can be
    considered true.

    The result should be a boolean true or false value.
    */
   virtual void op_in( VMContext* ctx, void* self ) const;
   
   /** Called back when the vm wants to know if a certain item provides a certain property.
     \param vm the virtual machine that will receive the result.
     \param self the instance (or 0 on flat items).
     \param property The property that should be accessed.

    \note The operand is unary -- requires OpToken with 1 parameter.

    The result placed in target should be a boolean true or false value.
    */
   virtual void op_provides( VMContext* ctx, void* self, const String& property ) const;

   /** Call the instance.
    \param vm A virutal machine where the call is performed.
    \param self An instance of this class
    \param pcount the number of parameters in the call.

    This operation has variable count of parameters. They are placed in reverse
    order on the stack. The item being called is always pushed before the
    parameters.

    When done, the operator should unroll the stack (VMContect::popData) of exactly
    paramCount elements; this will leave the called item on top of the stack.
    That item must be overwritten with the return value of this operation (might
    be nil).

    \note The default operation associated with the base class is that to raise
    a non-callable exception.
    */
   virtual void op_call( VMContext* ctx, int32 paramCount, void* self ) const;


   /** Implents textification operator for the Virtual Macine.
    \param vm the virtual machine that will receive the result.
    \param self the instance (or 0 on flat items)
     \param self the instance (or 0 on flat items)

    \note The operand is unary -- requires OpToken with 1 parameter.

    This method obtains immediately a textual value for the instance
    to be stored in the virtual machine, or prepares the call for the proper
    string geneartor code.

    The base class behavior is that of calling Class::describe() with depth 1
    and without maximum lenght on the instance passed as self in the virtual
    machine and uses it as the result of the operation.

    Implementors not willing to use describe() or wishing to skip an extra
    virtual function call should reimplement this class.
    */
   virtual void op_toString( VMContext* ctx, void* self ) const;

   

protected:
   String m_name;
   int64 m_typeID;

   Module* m_module;
   uint32 m_lastGCMark;
};

}

#endif

/* end of class.h */