/* $Id: ClassName.h,v 1.2 2006/10/02 01:55:39 garvin Exp $ */
/******************************************************************************/
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
/******************************************************************************/
//***************************************************************************
// ClassName.h
//
// Purpose:
//   Provide a character string name for a class for use as a type tag.
//   the implementation guarantees that these character strings can
//   be compared as pointers for an equality test.
//   We can test for exact type via the virtual IsExactClass method 
//   and derived type via IsOfClass method 
//   
// History:
//   10/--/93 - John Mellor-Crummey - initial version
//   07/27/95 - Gil Hansen - added IS_CLASS()
//   08/28/95 - Lei Zhou - add const when possible
//
//***************************************************************************

#ifndef ClassName_h
#define ClassName_h

#define CLASS_NAME(Class)  Class ## ClassName

#define CLASS_NAME_EIMPL_DERIVED(Class, basename) \
   const char *CLASS_NAME(Class) = # basename

#define IS_CLASS(Item, Class)   (Item->ClassName() == CLASS_NAME(Class))

//----------------------------------------------------------------------//
// extern defs of CLASS_NAME(Class) variable and ConvertTo ## Class fct //
//----------------------------------------------------------------------//
#define CLASS_NAME_EDEF(Class) \
   extern const char *CLASS_NAME(Class) \

#define CLASS_NAME_EIMPL(Class) \
   const char *CLASS_NAME(Class) = # Class \


#define CLASS_NAME_CONVERT_EDEF(Class, BaseClass) \
   class Class; \
   extern Class * ConvertTo ## Class (const BaseClass *nd)

#define CLASS_NAME_CONVERT_EIMPL(Class, BaseClass) \
   Class * \
   ConvertTo ## Class (const BaseClass *nd) \
   { if (nd && !(nd->IsOfClass(CLASS_NAME(Class)))) { \
       return NULL;   \
     } else return (Class *) nd;  \
   } 

//--------------------------------//
// method defs and implemetations //
//--------------------------------//
#define CLASS_NAME_FDEF(Class)  \
   virtual const char *ClassName(void) const; \
   static  const char *ClassString(void); \
   virtual bool     IsExactClass(const char *_type_) const; \
   virtual bool     IsOfClass(const char *_type_) const 

// pure virtual methods 
#define CLASS_NAME_VDEF(Class)  \
   virtual const char *ClassName(void) const = 0; \
   static  const char *ClassString(void); \
   virtual bool     IsExactClass(const char *_type_) const = 0; \
   virtual bool     IsOfClass(const char *_type_) const = 0

// use this if class is not derived 
#define CLASS_NAME_FIMPL(Class) \
   const char *Class::ClassName(void) const  { return CLASS_NAME(Class); } \
   const char *Class::ClassString(void) { return CLASS_NAME(Class); } \
   bool     Class::IsExactClass(const char *_type_) const \
                                          { return ClassName() == _type_; } \
   bool     Class::IsOfClass(const char * _type_) const \
                                          { return  _type_ == CLASS_NAME(Class); }

// use this if class is derived 
#define DERIVED_CLASS_NAME_FIMPL(Class, BaseClass) \
   const char *Class::ClassName(void) const  { return CLASS_NAME(Class); } \
   const char *Class::ClassString(void) { return CLASS_NAME(Class); } \
   bool     Class::IsExactClass(const char *_type_) const \
                                             { return ClassName() == _type_; } \
   bool     Class::IsOfClass(const char * _type_) const  \
                    { return _type_ == CLASS_NAME(Class) || \
                             BaseClass::IsOfClass(_type_); }

//----------------------//
// class implementation //
//----------------------//
#define CLASS_NAME_IMPL(Class) \
   CLASS_NAME_EIMPL(Class); \
   CLASS_NAME_FIMPL(Class)

#define DERIVED_CLASS_NAME_IMPL(Class, BaseClass) \
   CLASS_NAME_EIMPL(Class); \
   DERIVED_CLASS_NAME_FIMPL(Class, BaseClass)

#endif // ClassName_h
