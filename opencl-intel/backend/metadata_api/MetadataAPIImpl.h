// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef METADATAAPIIMPL_H
#define METADATAAPIIMPL_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/DataTypes.h"

#include <string>

namespace Intel {
namespace MetadataAPI {

// Generic template for the traits types.
// Assumes the the T type is inherited from the llvm::Metadata
template <class T, typename C = void>
struct MDValueTraits {
  typedef T *value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return nullptr;
    }

    assert(llvm::isa<T>(pNode) && "can't load value, wrong node type");
    value_type pT = llvm::cast<T>(pNode);

    return pT;
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    return const_cast<value_type>(Value);
  }
};

template <>
struct MDValueTraits<llvm::StringRef, void> {
  typedef llvm::StringRef value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return llvm::StringRef();
    }

    assert(llvm::isa<llvm::MDString>(pNode) &&
           "can't load string, wrong node type");
    auto mdStr = llvm::cast<llvm::MDString>(pNode);

    return mdStr->getString();
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    return llvm::MDString::get(Context, Value);
  }
};

template <>
struct MDValueTraits<std::string, void> {
  typedef std::string value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return std::string();
    }

    assert(llvm::isa<llvm::MDString>(pNode) &&
      "can't load string, wrong node type");
    auto mdStr = llvm::cast<llvm::MDString>(pNode);

    return mdStr->getString();
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
    const value_type &Value) {
    return llvm::MDString::get(Context, Value);
  }
};

template <>
struct MDValueTraits<bool, void> {
  typedef bool value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return value_type();
    }

    auto pVal = llvm::mdconst::dyn_extract<llvm::ConstantInt>(pNode);
    assert(pVal && "can't load bool value, wrong node type");

    return pVal->isOne();
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    return llvm::ConstantAsMetadata::get(
        Value ? llvm::ConstantInt::getTrue(Context)
              : llvm::ConstantInt::getFalse(Context));
  }
};

template <>
struct MDValueTraits<int32_t, void> {
  typedef int32_t value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return value_type();
    }

    auto pval = llvm::mdconst::dyn_extract<llvm::ConstantInt>(pNode);
    assert(pval && "Can't locate pval, wrong node type!");
    return (int32_t)pval->getValue().getSExtValue();
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    return llvm::ConstantAsMetadata::get(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Value));
  }
};

template <>
struct MDValueTraits<llvm::Type, void> {
  typedef llvm::Type *value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      return nullptr;
    }

    auto *pT = llvm::mdconst::dyn_extract<llvm::UndefValue>(pNode);
    assert(pT && "Can't locate UndefValue, wrong node type!");

    return pT->getType();
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    return llvm::ConstantAsMetadata::get(llvm::UndefValue::get(Value));
  }
};

template <>
struct MDValueTraits<llvm::Function, void> {
  typedef llvm::Function *value_type;

  static value_type load(llvm::Metadata *pNode) {
    if (nullptr == pNode) {
      // it is ok to pass nullptr nodes - part of support for optional values
      return nullptr;
    }

    value_type pT = llvm::mdconst::dyn_extract<llvm::Function>(pNode);
    assert(pT && "can't load value, wrong node type");

    return pT;
  }

  static llvm::Metadata *generateValue(llvm::LLVMContext &Context,
                                       const value_type &Value) {
    if (!Value)
      return nullptr;
    return llvm::ValueAsMetadata::get(const_cast<value_type>(Value));
  }
};

// Iterator over the meta data nodes list. It is assumed that
// all the nodes are of the same type ( as specified by the T template
// parameter) Template parameters: T - type of the entry node N - type of the
// root(parent) node (supported types are MDNode and NamedMDNode ) C - traits
// type (see the MDValueTraits )
template <class T, class RootT = llvm::MDNode, class Traits = MDValueTraits<T>>
class MetaDataIterator {
public:
  typedef typename Traits::value_type value_type;
  typedef MetaDataIterator<T, RootT, Traits> this_type;

  // Ctor. Creates the sentinel iterator. Usually used as an end iterator
  explicit MetaDataIterator(const RootT *pNode)
      : m_pNode(pNode), m_index(pNode->getNumOperands()) {}

  // Ctor. Create the iterator on given index
  // ToDo: consider removing index from here.
  // I suppose with GlobalObject Metadata everything will start with zero.
  // Maybe for statistics the index would be useful.
  MetaDataIterator(const RootT *pNode, unsigned int Index)
      : m_pNode(pNode), m_index(Index) {
    assert(Index <= pNode->getNumOperands());
  }

  llvm::Metadata *operator*() {
    assert(!isNil() && "Reached nullptr!");

    return m_pNode->getOperand(m_index);
  }

  // returns the current item in the list
  value_type get() {
    assert(!isNil() && "Reached nullptr!");

    return Traits::load(m_pNode->getOperand(m_index));
  }

  this_type &operator++() {
    if (!isNil())
      ++m_index;
    return (*this);
  }

  this_type operator++(int) {
    this_type tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const this_type &rhs) {
    return m_pNode == rhs.m_pNode && m_index == rhs.m_index;
  }

  bool operator!=(const this_type &rhs) { return !this->operator==(rhs); }

private:
  bool isNil() { return m_index == m_pNode->getNumOperands(); }

private:
  const RootT *m_pNode; // pointer to the parent node
  unsigned int m_index;
};

// Represents the metadata value.
// The root node is actually storing the value.
template <class T, class Traits = MDValueTraits<T>>
class MDValue {
public:
  typedef typename Traits::value_type value_type;

  MDValue(llvm::Metadata *pNode)
      : m_pNode(pNode), m_value(Traits::load(pNode)) {}

  MDValue() : m_pNode(nullptr), m_value() {}

  MDValue(const value_type &Value) : m_pNode(nullptr), m_value(Value) {}

  value_type get() const { return m_value; }

  bool hasValue() const { return m_pNode != nullptr; }

  llvm::Metadata *generateNode(llvm::LLVMContext &Context) const {
    return Traits::generateValue(Context, m_value);
  }

private:
  llvm::Metadata *m_pNode;
  value_type m_value;
};

// Allows switching between llvm::Module/llvm:GlobalObject
// named metadata handling.
class MDValueGlobalObjectStrategy {
public:
  typedef llvm::GlobalObject *subject_type;

  static void setMetadata(subject_type pGlobal, llvm::StringRef Kind,
                          llvm::MDNode *MD) {
    pGlobal->setMetadata(Kind, MD);
  }

  static llvm::MDNode *getMetadata(const subject_type pGlobal,
                                   llvm::StringRef Kind) {
    return pGlobal->getMetadata(Kind);
  }

  static void unset(subject_type pGlobal, llvm::StringRef Kind) {
    pGlobal->setMetadata(Kind, nullptr);
  }
};

class MDValueModuleStrategy {
public:
  typedef llvm::Module *subject_type;

  static void setMetadata(subject_type pModule, llvm::StringRef Kind,
                          llvm::MDNode *MD) {
    unset(pModule, Kind);
    auto root = pModule->getOrInsertNamedMetadata(Kind);
    root->addOperand(MD);
  }

  static llvm::MDNode *getMetadata(const subject_type pModule,
                                   llvm::StringRef Kind) {
    auto namedMDNode = pModule->getNamedMetadata(Kind);
    if (namedMDNode == nullptr)
      return nullptr;
    
    // Several or zero nodes are not supported! Probably result of linking.
    return namedMDNode->getOperand(0);
  }

  static void unset(subject_type pModule, llvm::StringRef Kind) {
    pModule->eraseNamedMetadata(pModule->getOrInsertNamedMetadata(Kind));
  }
};

// Represents the meta data value stored as named function metadata
template <class T, class MDGetSetStrategy, class Traits = MDValueTraits<T>>
class NamedMDValue {
public:
  typedef typename Traits::value_type value_type;
  typedef typename MDGetSetStrategy::subject_type subject_type;

  NamedMDValue(subject_type pGlobal, const char *Name)
      : m_pGlobal(pGlobal), m_id(Name), m_isLoaded(false), m_value() {}

  NamedMDValue(subject_type pGlobal, const char *Name, const value_type &Value)
      : m_pGlobal(pGlobal), m_id(Name), m_isLoaded(false), m_value(Value) {}

  operator value_type() {
    load();
    return (value_type)m_value;
  }

  value_type get() {
    load();
    return (value_type)m_value.get();
  }

  bool hasValue() const {
    load();
    return m_value.hasValue();
  }

  void generateNode() const {
    llvm::MDNode *pNode = llvm::MDNode::get(
        m_pGlobal->getContext(), m_value.generateNode(m_pGlobal->getContext()));

    MDGetSetStrategy::setMetadata(m_pGlobal, m_id, pNode);
  }

  llvm::StringRef getID() const { return m_id; }

protected:
  void load() const {
    if (m_isLoaded || nullptr == m_pGlobal) {
      return;
    }

    m_value = getValueNode(MDGetSetStrategy::getMetadata(m_pGlobal, m_id));

    m_isLoaded = true;
  }

private:
  llvm::Metadata *getValueNode(const llvm::Metadata *pNode) const {
    if (nullptr == pNode) {
      return nullptr; // this is allowed for optional nodes
    }

    const llvm::MDNode *pMDNode = llvm::dyn_cast<const llvm::MDNode>(pNode);
    assert(pMDNode && "Named value parent node is not of MDNode type");

    assert(!(pMDNode->getNumOperands() < 1) &&
           "MDValue doesn't have a value node");

    return pMDNode->getOperand(0);
  }

private:
  subject_type m_pGlobal;
  llvm::StringRef m_id;
  mutable bool m_isLoaded;
  mutable MDValue<value_type, Traits> m_value;
};

// Metadata list. It is assumed that
// all the nodes are of the same type ( as specified by the T template
// parameter) Template parameters: T - type of the entry node Traits - convertor
// type (see the MDValueTraits )
template <class T, class MDGetSetStrategy, class Traits = MDValueTraits<T>>
class NamedMDList {
public:
  typedef MetaDataIterator<T, llvm::MDNode, Traits> meta_iterator;
  typedef typename Traits::value_type item_type;
  typedef typename llvm::SmallVector<item_type, 8> vector_type;
  typedef typename MDGetSetStrategy::subject_type subject_type;
  typedef typename vector_type::iterator iterator;
  typedef typename vector_type::const_iterator const_iterator;

  NamedMDList(subject_type pGlobal, const char *Name)
      : m_pGlobal(pGlobal), m_id(Name),
        m_pNode(MDGetSetStrategy::getMetadata(pGlobal, Name)),
        m_isLoaded(false), m_data() {}

  size_t size() const {
    load();
    return m_data.size();
  }

  bool empty() const {
    load();
    return m_data.empty();
  }

  iterator begin() {
    load();
    return m_data.begin();
  }

  iterator end() {
    load();
    return m_data.end();
  }

  const_iterator begin() const {
    load();
    return m_data.begin();
  }

  const_iterator end() const {
    load();
    return m_data.end();
  }

  item_type getItem(size_t Index) {
    load();
    assert(Index < m_data.size() && "out of bounds access");
    return m_data[Index];
  }

  // make a copy
  vector_type getList() const {
    load();
    return m_data;
  }

  bool hasValue() const { return m_pNode != nullptr; }

  void set(const vector_type &Vector) {
    llvm::SmallVector<llvm::Metadata *, 8> args;

    for (const auto &e : Vector) {
      MDValue<item_type, Traits> value(e);
      args.push_back(value.generateNode(m_pGlobal->getContext()));
    }

    llvm::MDNode *pNode = llvm::MDNode::get(m_pGlobal->getContext(), args);

    MDGetSetStrategy::setMetadata(m_pGlobal, m_id, pNode);

    m_pNode = MDGetSetStrategy::getMetadata(m_pGlobal, m_id);

    m_isLoaded = false;
  }

  void unset() {
    MDGetSetStrategy::unset(m_pGlobal, m_id);

    m_isLoaded = false;
  }

  llvm::StringRef getID() const { return m_id; }

protected:
  void load() const {
    if (m_isLoaded || nullptr == m_pNode) {
      return;
    }

    for (meta_iterator i(m_pNode, 0), e(m_pNode); i != e; ++i) {
      m_data.push_back(i.get());
    }

    m_isLoaded = true;
  }

protected:
  subject_type m_pGlobal;
  llvm::StringRef m_id;
  const llvm::MDNode *m_pNode;
  mutable bool m_isLoaded;
  mutable vector_type m_data;
};

// ToDo: Generalize to any length. Variadic templates, maybe?
template <class U, class V, class UTraits = MDValueTraits<U>,
          class VTraits = MDValueTraits<V>>
class NamedHeteroTupleMDList {
public:
  typedef typename UTraits::value_type Uitem_type;
  typedef typename VTraits::value_type Vitem_type;

  NamedHeteroTupleMDList(const llvm::Function *pFunction, const char *Name)
      : m_pID(Name), m_pNode(pFunction->getMetadata(Name)), m_isLoaded(false) {}

  Uitem_type getFirstItem() {
    load();
    return std::get<0>(m_data);
  }

  Vitem_type getSecondItem() {
    load();
    return std::get<1>(m_data);
  }

  bool hasValue() const { return m_pNode != nullptr; }

  llvm::StringRef getID() const { return m_pID; }

protected:
  virtual void load() const {
    if (m_isLoaded || nullptr == m_pNode) {
      return;
    }

    assert(m_pNode->getNumOperands() > 1 && "Unexpected number of operands!");

    auto Uvalue = UTraits::load(m_pNode->getOperand(0));
    auto Vvalue = VTraits::load(m_pNode->getOperand(1));

    std::tuple<Uitem_type, Vitem_type> data(Uvalue, Vvalue);
    m_data.swap(data);

    m_isLoaded = true;
  }

protected:
  const char *m_pID;
  const llvm::MDNode *m_pNode;
  mutable bool m_isLoaded;
  mutable std::tuple<Uitem_type, Vitem_type> m_data;
};

template <typename T>
class NamedMDValueAccessor {
  typedef typename T::subject_type subject_type;
  typedef typename T::value_type value_type;

public:
  NamedMDValueAccessor(subject_type Global, const char *Id)
      : m_pGlobal(Global), m_pId(Id), m_mdValue(Global, Id) {}

  bool hasValue() { return m_mdValue.hasValue(); }

  value_type get() { return m_mdValue.get(); }

  void set(const value_type &item) {
    T mdValue(m_pGlobal, m_pId, item);
    mdValue.generateNode();
  }

  llvm::StringRef getID() const { return m_mdValue.getID(); }

private:
  subject_type m_pGlobal;
  const char *m_pId;
  T m_mdValue;
};

template <typename T>
class VecTypeHintTupleMDListAccessor {
public:
  VecTypeHintTupleMDListAccessor(llvm::Function *Func, const char *Id)
      : m_mdlist(Func, Id) {}

  typename T::Uitem_type getType() { return m_mdlist.getFirstItem(); }

  typename T::Vitem_type getSign() { return m_mdlist.getSecondItem(); }

  bool hasValue() const { return m_mdlist.hasValue(); }

  llvm::StringRef getID() const { return m_mdlist.getID(); }

private:
  T m_mdlist;
};

template <typename T>
class WorkgroupSizeMDAccessor {
public:
  WorkgroupSizeMDAccessor(llvm::Function *Func, const char *Id)
      : m_mdlist(Func, Id) {}

  typename T::item_type getXDim() { return m_mdlist.getItem(0); }

  typename T::item_type getYDim() { return m_mdlist.getItem(1); }

  typename T::item_type getZDim() { return m_mdlist.getItem(2); }

  bool hasValue() const { return m_mdlist.hasValue(); }

  llvm::StringRef getID() const { return m_mdlist.getID(); }

private:
  T m_mdlist;
};

} // end namespace MetadataAPI
} // end namespace Intel
#endif
