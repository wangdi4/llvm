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
#include <set>
#include <map>

namespace Intel
{

// [LLVM 3.6 UPGRADE] WORKAROUND: MetaDataApi widely used llvm::Value RAUW ability
// but after the Metadata/Value split this ability of the Metadata was lost.
//
// See commit
//      From dad20b2ae2544708d6a33abdb9bddd0a329f50e0 Mon Sep 17 00:00:00 2001
//      From: "Duncan P. N. Exon Smith" <dexonsmith@apple.com>
//      Date: Tue, 9 Dec 2014 18:38:53 +0000
//      Subject: [PATCH] IR: Split Metadata from Value
//
// Because it is not clear if RAUW was really necessary by the MetaDataApiUtils in
// order to save resource it was decided to implement similar approach for the new
// Metadata. This approach relies on the fact what only users of OCL metadata might
// be MDNode or NamedMDNode. So each time MDNode or NamedMDNode is created or its
// operands get replaced an internal use mapping must be updated accordingly.

// The following enum is intended to differentiate MDNode and NamedMDNode because
// in LLVM 3.6.0 NamedMDNode is not inherited from Metadata so it is not integrated
// into the LLVM's custom RTTI
enum MDNodeKind { NamedMDNode_K, MDNode_K };

struct MDUser {
  MDUser(llvm::MDNode * node) : m_kind(MDNode_K), m_node(node) { }
  MDUser(llvm::NamedMDNode * node) : m_kind(NamedMDNode_K), m_node(node) { }

  bool operator< (MDUser const& other) const {
    return this->m_node < other.m_node;
  }

  MDNodeKind m_kind;
  void * m_node;
};

// FIXME: The metadataRAUW uses a global variable g_metaDataApiUtilsUseMap which is cleared
// in MetaDataApiUtils dtor. This approach should work OK with current implementation
// but bare in mind it is not thread safe and has other drawbacks typical for global
// variables.
typedef std::set<MDUser> UserSet;
extern std::map<llvm::Metadata *, UserSet> g_metaDataApiUtilsUseMap;

// This function is implementation of RAUW (aka llvm::Value::replaceAllUsesWith) for
// llvm::Metadata.
// FIXME: remove old unused metadata from LLVM context
inline void metadataRAUW(llvm::Metadata * oldNode, llvm::Metadata * newNode) {
  if(g_metaDataApiUtilsUseMap.count(oldNode)) {
    // Iterate over all users and replace the old operand with the new one
    UserSet & userSet = g_metaDataApiUtilsUseMap[oldNode];
    for(MDUser const& user : userSet) {
      // NamedMDNode or MDNode?
      switch(user.m_kind) {
        case NamedMDNode_K: {
          llvm::NamedMDNode * pNode = static_cast<llvm::NamedMDNode *>(user.m_node);
	  // Iterate over all used metadata and replace oldNode with newNode
          for(unsigned opIdx = 0; opIdx < pNode->getNumOperands(); ++opIdx) {
            if(pNode->getOperand(opIdx) == oldNode) {
              pNode->setOperand(opIdx, llvm::cast<llvm::MDNode>(newNode));
            }
          }
          break;
        }
        case MDNode_K: {
          llvm::MDNode * pNode = static_cast<llvm::MDNode *>(user.m_node);
	  // Iterate over all used metadata and replace oldNode with newNode
          for(unsigned opIdx = 0; opIdx < pNode->getNumOperands(); ++opIdx) {
            if(pNode->getOperand(opIdx).get() == oldNode) {
              pNode->replaceOperandWith(opIdx, newNode);
            }
          }
          break;
        }
      }
    }
    // Update use mapping
    g_metaDataApiUtilsUseMap[oldNode].swap(g_metaDataApiUtilsUseMap[newNode]);
    g_metaDataApiUtilsUseMap.erase(oldNode);
  }
}

template<typename MetaDataNode, typename Cont>
inline void updateMetadataUseMapping(MetaDataNode * user, Cont const & operands) {
  for(llvm::Metadata * operand : operands) {
    // Operand might be nullptr
    if(operand) g_metaDataApiUtilsUseMap[operand].insert(user);
  }
}

template<typename MetaDataNode, typename UsedMDNode>
inline void updateMetadataUseMapping(MetaDataNode * user, UsedMDNode * operand) {
  // Operand might be nullptr
  if(operand) g_metaDataApiUtilsUseMap[operand].insert(user);
}

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
        if( nullptr == pNode)
        {
            // it is ok to pass NULL nodes - part of support for optional values
            return nullptr;
        }

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
           metadataRAUW(trgt, generateValue(context, val));
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
           metadataRAUW(trgt, generateValue(context, val));
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
           metadataRAUW(trgt, generateValue(context, val));
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
           metadataRAUW(trgt, generateValue(context, val));
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
           metadataRAUW(trgt, generateValue(context, val));
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
           metadataRAUW(trgt, generateValue(context, val));
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
	if(val) return llvm::ValueAsMetadata::get(const_cast<value_type>(val));
	return nullptr;
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
           metadataRAUW(trgt, generateValue(context, val));
        }
    }
};



} //namespace
#endif
