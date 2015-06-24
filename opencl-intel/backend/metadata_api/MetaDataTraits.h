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
// [LLVM 3.6 UPGRADE] FIXME: MetaDataApi relied on RAUW ability of llvm::Value before,
// but after the Metadata/Value split see commit 
//      From dad20b2ae2544708d6a33abdb9bddd0a329f50e0 Mon Sep 17 00:00:00 2001
//      From: "Duncan P. N. Exon Smith" <dexonsmith@apple.com>
//      Date: Tue, 9 Dec 2014 18:38:53 +0000
//      Subject: [PATCH] IR: Split Metadata from Value
// This ability of the Metadata was lost.
// It is not clear if it was really used before by OCL BE so the code below was instrumented
// with assert(false && "[LLVM 3.6 UPGRADE] FIXME) in each place where the Metadata/Value
// split had an impact.
// If Metadata's RAUW will turn out to be used (and useful) by the OCL BE then it seems
// feasible to store a mapping from used values to it's users during loading/saving the
// 'IMetaDataObject's iniside the MetaDataUtils instance. Otherwise the better way is to
// cut the RAUW.

///
// Generic template for the traits types.
// Assumes the the T type is inherited from the llvm::Metadata
// (in CPP0X we solve this problem - below )
template< class T, typename C = void >
struct MDValueTraits
{
    // Value type that will be used in the metadata containers
    typedef T* value_type;

    ///
    // Loads the given value_type from the given node
    static value_type load(llvm::Metadata* pNode)
    {
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if( nullptr == pNode) // || !pNode->hasValueHandle())
        {
            // it is ok to pass NULL nodes - part of support for optional values
            return nullptr;
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        value_type pT = llvm::dyn_cast<T>(pNode);
        if( nullptr == pT )
        {
            throw "can't load value, wrong node type";
        }

        return pT;
    }

    ///
    // Creates the new metadata node from the given value_type
    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
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
    static void save(llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
           assert(false && "[LLVM 3.6 UPGRADE] FIXME");
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

    static  value_type load(llvm::Metadata* pNode)
    {
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        //if( nullptr == pNode || !pNode->hasValueHandle())
        if( nullptr == pNode )
        {
            return std::string();
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        llvm::MDString* mdStr = llvm::dyn_cast<llvm::MDString>(pNode);
        if( !mdStr )
        {
            throw "can't load string, wrong node type";
        }
        return mdStr->getString();
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
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

    static void save(llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
            //trgt->replaceAllUsesWith( generateValue(context, val) );
            assert(false && "[LLVM 3.6 UPGRADE] FIXME");
        }
    }
};

template<>
struct MDValueTraits<bool, void>
{
    typedef bool value_type;

    static value_type load( llvm::Metadata* pNode)
    {
        using namespace llvm;
        //we allow for NULL value loads
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if(nullptr == pNode)// || !pNode->hasValueHandle())
        {
            return value_type();
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        ConstantInt* pval = mdconst::dyn_extract<ConstantInt>(pNode);
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
        return pval->isOne();
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ConstantAsMetadata::get(
          val ? llvm::ConstantInt::getTrue(context) : llvm::ConstantInt::getFalse(context));
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
            assert(false && "[LLVM 3.6 UPGRADE] FIXME");
        }
    }
};

template<>
struct MDValueTraits<int64_t, void>
{
    typedef int64_t value_type;

    static value_type load( llvm::Metadata* pNode)
    {
        //we allow for NULL value loads
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if( nullptr == pNode )//|| !pNode->hasValueHandle())
        {
            return value_type();
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        using namespace llvm;
        ConstantInt* pval = mdconst::dyn_extract<ConstantInt>(pNode);
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
        return pval->getValue().getSExtValue();
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ConstantAsMetadata::get(
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), val));
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
           assert(false && "[LLVM 3.6 UPGRADE] FIXME");
        }
    }
};

template<>
struct MDValueTraits<int32_t, void>
{
    typedef int32_t value_type;

    static value_type load( llvm::Metadata* pNode)
    {
        //we allow for NULL value loads
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if( nullptr == pNode )//|| !pNode->hasValueHandle())
        {
            return value_type();
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        using namespace llvm;
        ConstantInt* pval = mdconst::dyn_extract<ConstantInt>(pNode);
        if( !pval )
        {
            throw "can't load bool value, wrong node type";
        }
        return (int32_t)pval->getValue().getSExtValue();
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ConstantAsMetadata::get(
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), val));
    }

    static bool dirty(const value_type& val )
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
           assert(false && "[LLVM 3.6 UPGRADE] FIXME");
        }
    }
};


#if _HAS_CPP0X
template<class T>
struct MDValueTraits< T,  typename std::enable_if<std::is_base_of<llvm::Metadata, T >::value>::type >
{
    typedef T* value_type;

    static value_type load(llvm::Metadata* pNode)
    {
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if(nullptr == pNode )//|| !pNode->hasValueHandle())
        {
            return nullptr;
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        value_type pT = llvm::dyn_cast<T>(pNode);
        if( nullptr == pT)
        {
            throw "can't local value , wrong node type";
        }
        return pT;
    }

    static llvm::Metadata* generateValue( llvm::LLVMContext& context, const value_type& val )
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

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
           assert(false && "[LLVM 3.6 UPGRADE] FIXME");
            //llvm::MDNode::deleteTemporary(trgt);
        }
    }
};
#endif

template<>
struct MDValueTraits<llvm::Function, void>
{
    typedef llvm::Function* value_type;

    static value_type load( llvm::Metadata* pNode)
    {
        using namespace llvm;
        // [LLVM 3.6 UPGRADE] FIXME: why was !pNode->hasValueHandle() below?
        if(nullptr == pNode )//|| !pNode->hasValueHandle())
        {
            // it is ok to pass NULL nodes - part of support for optional values
            return nullptr;
        }
        // [LLVM 3.6 UPGRADE] FIXME: In order to figure out why !pNode->hasValueHandle()
        // is needed let's catch a situation when pNode is not NULL live.
        // assert(!pNode && "[LLVM 3.6 UPGRADE] FIXME: pNode is not NULL - hint for !pNode->hasValueHandle()");

        ValueAsMetadata * vAsM = dyn_cast<ValueAsMetadata>(pNode);
        assert(vAsM && "VauseAsMetadata is expected");
        value_type pT = dyn_cast<Function>(vAsM->getValue()->stripPointerCasts());
        if( nullptr == pT )
        {
            throw "can't load value, wrong node type";
        }

        return pT;
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return llvm::ValueAsMetadata::get(const_cast<value_type>(val));
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const llvm::Metadata*& val)
    {
        return const_cast<llvm::Metadata*>(val);
    }

    static bool dirty(const value_type& val)
    {
        return false;
    }

    static void discardChanges(value_type& val)
    {
    }

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        if( load(trgt) != val )
        {
//            trgt->replaceAllUsesWith( generateValue(context, val) );
//           assert(false && "[LLVM 3.6 UPGRADE] FIXME");
        }
    }
};



} //namespace
#endif
