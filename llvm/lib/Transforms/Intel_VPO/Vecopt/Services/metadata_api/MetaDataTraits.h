/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef METADATATRAITS_H
#define METADATATRAITS_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/DataTypes.h"

namespace Intel
{
///
// Generic template for the traits types.
// Assumes the the T type is inherited from the llvm::Value
// (in CPP0X we solve this problem - below )
template< class T, typename C = void >
struct MDValueTraits
{
    // Value type that will be used in the metadata containers
    typedef T* value_type;

    ///
    // Loads the given value_type from the given node
    static value_type load(llvm::Value* pNode)
    {
        if( NULL == pNode || !pNode->hasValueHandle())
        {
            // it is ok to pass NULL nodes - part of support for optional values
            return NULL;
        }

        value_type pT = llvm::dyn_cast<T>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pT )
        {
            throw "can't load value, wrong node type";
        }
#endif

        return pT;
    }

    ///
    // Creates the new metadata node from the given value_type
    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return const_cast<value_type>(val);
    }

    ///
    // Indicates that the value_type was changed
    static bool dirty(const value_type& val)
    {
        return false;
    }

    ///
    // Discard value changes
    static void discardChanges(value_type& val)
    {
    }

    ///
    // Save the given value to the target node
    static void save(llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};

///
// Metadata traits specialization for the std::string type
// We are storing std::string by value
template<>
struct MDValueTraits<std::string, void>
{
    typedef std::string value_type;

    static  value_type load(llvm::Value* pNode)
    {
        if( NULL == pNode || !pNode->hasValueHandle())
        {
            return std::string();
        }

        llvm::MDString* mdStr = llvm::dyn_cast<llvm::MDString>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( !mdStr )
        {
            throw "can't load string, wrong node type";
        }
#endif
        return mdStr->getString();
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::MDString::get(context, val);
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save(llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};

template<>
struct MDValueTraits<bool, void>
{
    typedef bool value_type;

    static value_type load( llvm::Value* pNode)
    {
        //we allow for NULL value loads
        if(NULL == pNode || !pNode->hasValueHandle())
        {
            return value_type();
        }

        llvm::ConstantInt* pval = llvm::dyn_cast<llvm::ConstantInt>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
#endif
        return pval->isOne();
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return val ? llvm::ConstantInt::getTrue(context) : llvm::ConstantInt::getFalse(context);
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};

template<>
struct MDValueTraits<int64_t, void>
{
    typedef int64_t value_type;

    static value_type load( llvm::Value* pNode)
    {
        //we allow for NULL value loads
        if( NULL == pNode || !pNode->hasValueHandle())
        {
            return value_type();
        }

        llvm::ConstantInt* pval = llvm::dyn_cast<llvm::ConstantInt>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
#endif
        return pval->getValue().getSExtValue();
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), val);
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};

template<>
struct MDValueTraits<int32_t, void>
{
    typedef int32_t value_type;

    static value_type load( llvm::Value* pNode)
    {
        //we allow for NULL value loads
        if( NULL == pNode || !pNode->hasValueHandle())
        {
            return value_type();
        }

        llvm::ConstantInt* pval = llvm::dyn_cast<llvm::ConstantInt>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
#endif
        return (int32_t)pval->getValue().getSExtValue();
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), val);
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};


#if _HAS_CPP0X
template<class T>
struct MDValueTraits< T,  typename std::enable_if<std::is_base_of<llvm::Value, T >::value>::type >
{
    typedef T* value_type;

    static value_type load(llvm::Value* pNode)
    {
        if(NULL == pNode || !pNode->hasValueHandle())
        {
            return NULL;
        }

        value_type pT = llvm::dyn_cast<T>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pT)
        {
            throw "can't local value , wrong node type";
        }
#endif
        return pT;
    }

    static llvm::Value* generateValue( llvm::LLVMContext& context, const value_type& val )
    {
        return const_cast<value_type>(val);
    }

    static bool dirty(const value_type& val)
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
            //llvm::MDNode::deleteTemporary(trgt);
        }
    }
};
#endif

template<>
struct MDValueTraits<llvm::Function, void>
{
    typedef llvm::Function* value_type;

    static value_type load( llvm::Value* pNode)
    {
        if(NULL == pNode || !pNode->hasValueHandle())
        {
            // it is ok to pass NULL nodes - part of support for optional values
            return NULL;
        }

        value_type pT = llvm::dyn_cast<llvm::Function>(pNode->stripPointerCasts());
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pT )
        {
            throw "can't load value, wrong node type";
        }
#endif

        return pT;
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return static_cast<llvm::Value*>(const_cast<value_type>(val));
    }

    static llvm::Value* generateValue(llvm::LLVMContext& context, const llvm::Value*& val)
    {
        return const_cast<llvm::Value*>(val);
    }

    static bool dirty(const value_type& val)
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Value* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            trgt->replaceAllUsesWith( generateValue(context, val) );
        }
    }
};



} //namespace
#endif
