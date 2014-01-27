/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/



#ifndef METADATAAPI_H
#define METADATAAPI_H

#include "MetaDataApiUtils.h"

namespace Intel
{
//typedefs and forward declarations
class ModuleInfoMetaData;
typedef MetaObjectHandle<ModuleInfoMetaData> ModuleInfoMetaDataHandle; 
        
        
    
class FunctionInfoMetaData;
typedef MetaObjectHandle<FunctionInfoMetaData> FunctionInfoMetaDataHandle; 
        
        
        
        
        
        
        
        
        
        
        
        
        
    
class StrWrapperMetaData;
typedef MetaObjectHandle<StrWrapperMetaData> StrWrapperMetaDataHandle; 
        
        
    
class KernelInfoMetaData;
typedef MetaObjectHandle<KernelInfoMetaData> KernelInfoMetaDataHandle; 
        
    
class KernelMetaData;
typedef MetaObjectHandle<KernelMetaData> KernelMetaDataHandle; 
        
        
        
    
class WorkGroupSizeMetaData;
typedef MetaObjectHandle<WorkGroupSizeMetaData> WorkGroupSizeMetaDataHandle;

///
// Read/Write the ModuleInfo structure from/to LLVM metadata
//
class ModuleInfoMetaData:public IMetaDataObject
{
public:
    typedef ModuleInfoMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef NamedMetaDataValue<int32_t>::value_type GAScounterType;        
    typedef MetaDataList<int32_t> GASwarningsList;

public:
    ///
    // Factory method - creates the ModuleInfoMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty ModuleInfoMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named ModuleInfoMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the ModuleInfoMetaData from the given metadata node
    //
    ModuleInfoMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named ModuleInfoMetaData object
    //
    ModuleInfoMetaData();

    ///
    // Ctor - creates the empty, named ModuleInfoMetaData object
    //
    ModuleInfoMetaData(const char* name);

    /// GAScounter related methods
    GAScounterType getGAScounter()
    {
        return m_GAScounter.get();
    }

    void setGAScounter( const GAScounterType& val)
    {
        m_GAScounter.set(val);
    }

    bool isGAScounterHasValue() const
    {
        return m_GAScounter.hasValue();
    }

        
    
    /// GASwarnings related methods
    GASwarningsList::iterator begin_GASwarnings()
    {
        return m_GASwarnings.begin();
    }

    GASwarningsList::iterator end_GASwarnings()
    {
        return m_GASwarnings.end();
    }

    GASwarningsList::const_iterator begin_GASwarnings() const
    {
        return m_GASwarnings.begin();
    }

    GASwarningsList::const_iterator end_GASwarnings() const
    {
        return m_GASwarnings.end();
    }

    size_t size_GASwarnings()  const
    {
        return m_GASwarnings.size();
    }

    bool empty_GASwarnings()  const
    {
        return m_GASwarnings.empty();
    }

    bool isGASwarningsHasValue() const
    {
        return m_GASwarnings.hasValue();
    }

    GASwarningsList::item_type getGASwarningsItem( size_t index )
    {
        return m_GASwarnings.getItem(index);
    }

    void setGASwarningsItem( size_t index, const GASwarningsList::item_type& item  )
    {
        return m_GASwarnings.setItem(index, item);
    }

    void addGASwarningsItem(const GASwarningsList::item_type& val)
    {
        m_GASwarnings.push_back(val);
    }

    GASwarningsList::iterator eraseGASwarningsItem(GASwarningsList::iterator i)
    {
        return m_GASwarnings.erase(i);
    }

    ///
    // Returns true if any of the ModuleInfoMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the ModuleInfoMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getGAScounterNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getGASwarningsNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    NamedMetaDataValue<int32_t> m_GAScounter;        
    MetaDataList<int32_t> m_GASwarnings;
    // parent node
    const llvm::MDNode* m_pNode;
};
        
        
    
///
// Read/Write the FunctionInfo structure from/to LLVM metadata
//
class FunctionInfoMetaData:public IMetaDataObject
{
public:
    typedef FunctionInfoMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef NamedMetaDataValue<bool>::value_type FuncPtrCallType;        
    typedef NamedMetaDataValue<bool>::value_type HasRecursionType;

public:
    ///
    // Factory method - creates the FunctionInfoMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty FunctionInfoMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named FunctionInfoMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the FunctionInfoMetaData from the given metadata node
    //
    FunctionInfoMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named FunctionInfoMetaData object
    //
    FunctionInfoMetaData();

    ///
    // Ctor - creates the empty, named FunctionInfoMetaData object
    //
    FunctionInfoMetaData(const char* name);

    /// FuncPtrCall related methods
    FuncPtrCallType getFuncPtrCall()
    {
        return m_FuncPtrCall.get();
    }

    void setFuncPtrCall( const FuncPtrCallType& val)
    {
        m_FuncPtrCall.set(val);
    }

    bool isFuncPtrCallHasValue() const
    {
        return m_FuncPtrCall.hasValue();
    }

        
    
    /// HasRecursion related methods
    HasRecursionType getHasRecursion()
    {
        return m_HasRecursion.get();
    }

    void setHasRecursion( const HasRecursionType& val)
    {
        m_HasRecursion.set(val);
    }

    bool isHasRecursionHasValue() const
    {
        return m_HasRecursion.hasValue();
    }

    ///
    // Returns true if any of the FunctionInfoMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the FunctionInfoMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getFuncPtrCallNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getHasRecursionNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    NamedMetaDataValue<bool> m_FuncPtrCall;        
    NamedMetaDataValue<bool> m_HasRecursion;
    // parent node
    const llvm::MDNode* m_pNode;
};
        
        
        
        
        
        
        
        
        
        
        
        
        
    
///
// Read/Write the StrWrapper structure from/to LLVM metadata
//
class StrWrapperMetaData:public IMetaDataObject
{
public:
    typedef StrWrapperMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef MetaDataValue<std::string>::value_type valueType;

public:
    ///
    // Factory method - creates the StrWrapperMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty StrWrapperMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named StrWrapperMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the StrWrapperMetaData from the given metadata node
    //
    StrWrapperMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named StrWrapperMetaData object
    //
    StrWrapperMetaData();

    ///
    // Ctor - creates the empty, named StrWrapperMetaData object
    //
    StrWrapperMetaData(const char* name);

    /// value related methods
    valueType getvalue()
    {
        return m_value.get();
    }

    void setvalue( const valueType& val)
    {
        m_value.set(val);
    }

    bool isvalueHasValue() const
    {
        return m_value.hasValue();
    }

    ///
    // Returns true if any of the StrWrapperMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the StrWrapperMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getvalueNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    MetaDataValue<std::string> m_value;
    // parent node
    const llvm::MDNode* m_pNode;
};
        
        
    
///
// Read/Write the KernelInfo structure from/to LLVM metadata
//
class KernelInfoMetaData:public IMetaDataObject
{
public:
    typedef KernelInfoMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef NamedMetaDataValue<int32_t>::value_type LocalBufferSizeType;        
    typedef NamedMetaDataValue<int32_t>::value_type BarrierBufferSizeType;        
    typedef NamedMetaDataValue<int32_t>::value_type KernelExecutionLengthType;        
    typedef NamedMetaDataValue<int32_t>::value_type MaxWGDimensionsType;        
    typedef NamedMetaDataValue<bool>::value_type KernelHasBarrierType;        
    typedef NamedMetaDataValue<bool>::value_type NoBarrierPathType;        
    typedef NamedMetaDataValue<llvm::Function>::value_type VectorizedKernelType;        
    typedef NamedMetaDataValue<int32_t>::value_type VectorizedWidthType;        
    typedef NamedMetaDataValue<llvm::Function>::value_type KernelWrapperType;        
    typedef NamedMetaDataValue<llvm::Function>::value_type ScalarizedKernelType;        
    typedef NamedMetaDataValue<int32_t>::value_type BlockLiteralSizeType;

public:
    ///
    // Factory method - creates the KernelInfoMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty KernelInfoMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named KernelInfoMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the KernelInfoMetaData from the given metadata node
    //
    KernelInfoMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named KernelInfoMetaData object
    //
    KernelInfoMetaData();

    ///
    // Ctor - creates the empty, named KernelInfoMetaData object
    //
    KernelInfoMetaData(const char* name);

    /// LocalBufferSize related methods
    LocalBufferSizeType getLocalBufferSize()
    {
        return m_LocalBufferSize.get();
    }

    void setLocalBufferSize( const LocalBufferSizeType& val)
    {
        m_LocalBufferSize.set(val);
    }

    bool isLocalBufferSizeHasValue() const
    {
        return m_LocalBufferSize.hasValue();
    }

        
    
    /// BarrierBufferSize related methods
    BarrierBufferSizeType getBarrierBufferSize()
    {
        return m_BarrierBufferSize.get();
    }

    void setBarrierBufferSize( const BarrierBufferSizeType& val)
    {
        m_BarrierBufferSize.set(val);
    }

    bool isBarrierBufferSizeHasValue() const
    {
        return m_BarrierBufferSize.hasValue();
    }

        
    
    /// KernelExecutionLength related methods
    KernelExecutionLengthType getKernelExecutionLength()
    {
        return m_KernelExecutionLength.get();
    }

    void setKernelExecutionLength( const KernelExecutionLengthType& val)
    {
        m_KernelExecutionLength.set(val);
    }

    bool isKernelExecutionLengthHasValue() const
    {
        return m_KernelExecutionLength.hasValue();
    }

        
    
    /// MaxWGDimensions related methods
    MaxWGDimensionsType getMaxWGDimensions()
    {
        return m_MaxWGDimensions.get();
    }

    void setMaxWGDimensions( const MaxWGDimensionsType& val)
    {
        m_MaxWGDimensions.set(val);
    }

    bool isMaxWGDimensionsHasValue() const
    {
        return m_MaxWGDimensions.hasValue();
    }

        
    
    /// KernelHasBarrier related methods
    KernelHasBarrierType getKernelHasBarrier()
    {
        return m_KernelHasBarrier.get();
    }

    void setKernelHasBarrier( const KernelHasBarrierType& val)
    {
        m_KernelHasBarrier.set(val);
    }

    bool isKernelHasBarrierHasValue() const
    {
        return m_KernelHasBarrier.hasValue();
    }

        
    
    /// NoBarrierPath related methods
    NoBarrierPathType getNoBarrierPath()
    {
        return m_NoBarrierPath.get();
    }

    void setNoBarrierPath( const NoBarrierPathType& val)
    {
        m_NoBarrierPath.set(val);
    }

    bool isNoBarrierPathHasValue() const
    {
        return m_NoBarrierPath.hasValue();
    }

        
    
    /// VectorizedKernel related methods
    VectorizedKernelType getVectorizedKernel()
    {
        return m_VectorizedKernel.get();
    }

    void setVectorizedKernel( const VectorizedKernelType& val)
    {
        m_VectorizedKernel.set(val);
    }

    bool isVectorizedKernelHasValue() const
    {
        return m_VectorizedKernel.hasValue();
    }

        
    
    /// VectorizedWidth related methods
    VectorizedWidthType getVectorizedWidth()
    {
        return m_VectorizedWidth.get();
    }

    void setVectorizedWidth( const VectorizedWidthType& val)
    {
        m_VectorizedWidth.set(val);
    }

    bool isVectorizedWidthHasValue() const
    {
        return m_VectorizedWidth.hasValue();
    }

        
    
    /// KernelWrapper related methods
    KernelWrapperType getKernelWrapper()
    {
        return m_KernelWrapper.get();
    }

    void setKernelWrapper( const KernelWrapperType& val)
    {
        m_KernelWrapper.set(val);
    }

    bool isKernelWrapperHasValue() const
    {
        return m_KernelWrapper.hasValue();
    }

        
    
    /// ScalarizedKernel related methods
    ScalarizedKernelType getScalarizedKernel()
    {
        return m_ScalarizedKernel.get();
    }

    void setScalarizedKernel( const ScalarizedKernelType& val)
    {
        m_ScalarizedKernel.set(val);
    }

    bool isScalarizedKernelHasValue() const
    {
        return m_ScalarizedKernel.hasValue();
    }

        
    
    /// BlockLiteralSize related methods
    BlockLiteralSizeType getBlockLiteralSize()
    {
        return m_BlockLiteralSize.get();
    }

    void setBlockLiteralSize( const BlockLiteralSizeType& val)
    {
        m_BlockLiteralSize.set(val);
    }

    bool isBlockLiteralSizeHasValue() const
    {
        return m_BlockLiteralSize.hasValue();
    }

    ///
    // Returns true if any of the KernelInfoMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the KernelInfoMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getLocalBufferSizeNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getBarrierBufferSizeNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getKernelExecutionLengthNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getMaxWGDimensionsNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getKernelHasBarrierNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getNoBarrierPathNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getVectorizedKernelNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getVectorizedWidthNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getKernelWrapperNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getScalarizedKernelNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getBlockLiteralSizeNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    NamedMetaDataValue<int32_t> m_LocalBufferSize;        
    NamedMetaDataValue<int32_t> m_BarrierBufferSize;        
    NamedMetaDataValue<int32_t> m_KernelExecutionLength;        
    NamedMetaDataValue<int32_t> m_MaxWGDimensions;        
    NamedMetaDataValue<bool> m_KernelHasBarrier;        
    NamedMetaDataValue<bool> m_NoBarrierPath;        
    NamedMetaDataValue<llvm::Function> m_VectorizedKernel;        
    NamedMetaDataValue<int32_t> m_VectorizedWidth;        
    NamedMetaDataValue<llvm::Function> m_KernelWrapper;        
    NamedMetaDataValue<llvm::Function> m_ScalarizedKernel;        
    NamedMetaDataValue<int32_t> m_BlockLiteralSize;
    // parent node
    const llvm::MDNode* m_pNode;
};
        
    
///
// Read/Write the Kernel structure from/to LLVM metadata
//
class KernelMetaData:public IMetaDataObject
{
public:
    typedef KernelMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef MetaDataValue<llvm::Function>::value_type FunctionType;        
    typedef NamedMetaDataValue<llvm::UndefValue>::value_type VecTypeHintType;        
        
        
    typedef MetaDataList<int32_t> AddressSpaceList;        
    typedef MetaDataList<std::string> ArgAccessQualifierList;        
    typedef MetaDataList<std::string> ArgTypesList;        
    typedef MetaDataList<std::string> ArgTypeQualifierList;        
    typedef MetaDataList<std::string> ArgNameList;

public:
    ///
    // Factory method - creates the KernelMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty KernelMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named KernelMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the KernelMetaData from the given metadata node
    //
    KernelMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named KernelMetaData object
    //
    KernelMetaData();

    ///
    // Ctor - creates the empty, named KernelMetaData object
    //
    KernelMetaData(const char* name);

    /// Function related methods
    FunctionType getFunction()
    {
        return m_Function.get();
    }

    void setFunction( const FunctionType& val)
    {
        m_Function.set(val);
    }

    bool isFunctionHasValue() const
    {
        return m_Function.hasValue();
    }

        
    
    /// VecTypeHint related methods
    VecTypeHintType getVecTypeHint()
    {
        return m_VecTypeHint.get();
    }

    void setVecTypeHint( const VecTypeHintType& val)
    {
        m_VecTypeHint.set(val);
    }

    bool isVecTypeHintHasValue() const
    {
        return m_VecTypeHint.hasValue();
    }

        
    
    /// WorkGroupSizeHint related methods
    WorkGroupSizeMetaDataHandle getWorkGroupSizeHint()
    {
        return m_WorkGroupSizeHint;
    }

        
    
    /// ReqdWorkGroupSize related methods
    WorkGroupSizeMetaDataHandle getReqdWorkGroupSize()
    {
        return m_ReqdWorkGroupSize;
    }

        
    
    /// AddressSpace related methods
    AddressSpaceList::iterator begin_AddressSpace()
    {
        return m_AddressSpace.begin();
    }

    AddressSpaceList::iterator end_AddressSpace()
    {
        return m_AddressSpace.end();
    }

    AddressSpaceList::const_iterator begin_AddressSpace() const
    {
        return m_AddressSpace.begin();
    }

    AddressSpaceList::const_iterator end_AddressSpace() const
    {
        return m_AddressSpace.end();
    }

    size_t size_AddressSpace()  const
    {
        return m_AddressSpace.size();
    }

    bool empty_AddressSpace()  const
    {
        return m_AddressSpace.empty();
    }

    bool isAddressSpaceHasValue() const
    {
        return m_AddressSpace.hasValue();
    }

    AddressSpaceList::item_type getAddressSpaceItem( size_t index )
    {
        return m_AddressSpace.getItem(index);
    }

    void setAddressSpaceItem( size_t index, const AddressSpaceList::item_type& item  )
    {
        return m_AddressSpace.setItem(index, item);
    }

    void addAddressSpaceItem(const AddressSpaceList::item_type& val)
    {
        m_AddressSpace.push_back(val);
    }

    AddressSpaceList::iterator eraseAddressSpaceItem(AddressSpaceList::iterator i)
    {
        return m_AddressSpace.erase(i);
    }
        
    
    /// ArgAccessQualifier related methods
    ArgAccessQualifierList::iterator begin_ArgAccessQualifier()
    {
        return m_ArgAccessQualifier.begin();
    }

    ArgAccessQualifierList::iterator end_ArgAccessQualifier()
    {
        return m_ArgAccessQualifier.end();
    }

    ArgAccessQualifierList::const_iterator begin_ArgAccessQualifier() const
    {
        return m_ArgAccessQualifier.begin();
    }

    ArgAccessQualifierList::const_iterator end_ArgAccessQualifier() const
    {
        return m_ArgAccessQualifier.end();
    }

    size_t size_ArgAccessQualifier()  const
    {
        return m_ArgAccessQualifier.size();
    }

    bool empty_ArgAccessQualifier()  const
    {
        return m_ArgAccessQualifier.empty();
    }

    bool isArgAccessQualifierHasValue() const
    {
        return m_ArgAccessQualifier.hasValue();
    }

    ArgAccessQualifierList::item_type getArgAccessQualifierItem( size_t index )
    {
        return m_ArgAccessQualifier.getItem(index);
    }

    void setArgAccessQualifierItem( size_t index, const ArgAccessQualifierList::item_type& item  )
    {
        return m_ArgAccessQualifier.setItem(index, item);
    }

    void addArgAccessQualifierItem(const ArgAccessQualifierList::item_type& val)
    {
        m_ArgAccessQualifier.push_back(val);
    }

    ArgAccessQualifierList::iterator eraseArgAccessQualifierItem(ArgAccessQualifierList::iterator i)
    {
        return m_ArgAccessQualifier.erase(i);
    }
        
    
    /// ArgTypes related methods
    ArgTypesList::iterator begin_ArgTypes()
    {
        return m_ArgTypes.begin();
    }

    ArgTypesList::iterator end_ArgTypes()
    {
        return m_ArgTypes.end();
    }

    ArgTypesList::const_iterator begin_ArgTypes() const
    {
        return m_ArgTypes.begin();
    }

    ArgTypesList::const_iterator end_ArgTypes() const
    {
        return m_ArgTypes.end();
    }

    size_t size_ArgTypes()  const
    {
        return m_ArgTypes.size();
    }

    bool empty_ArgTypes()  const
    {
        return m_ArgTypes.empty();
    }

    bool isArgTypesHasValue() const
    {
        return m_ArgTypes.hasValue();
    }

    ArgTypesList::item_type getArgTypesItem( size_t index )
    {
        return m_ArgTypes.getItem(index);
    }

    void setArgTypesItem( size_t index, const ArgTypesList::item_type& item  )
    {
        return m_ArgTypes.setItem(index, item);
    }

    void addArgTypesItem(const ArgTypesList::item_type& val)
    {
        m_ArgTypes.push_back(val);
    }

    ArgTypesList::iterator eraseArgTypesItem(ArgTypesList::iterator i)
    {
        return m_ArgTypes.erase(i);
    }
        
    
    /// ArgTypeQualifier related methods
    ArgTypeQualifierList::iterator begin_ArgTypeQualifier()
    {
        return m_ArgTypeQualifier.begin();
    }

    ArgTypeQualifierList::iterator end_ArgTypeQualifier()
    {
        return m_ArgTypeQualifier.end();
    }

    ArgTypeQualifierList::const_iterator begin_ArgTypeQualifier() const
    {
        return m_ArgTypeQualifier.begin();
    }

    ArgTypeQualifierList::const_iterator end_ArgTypeQualifier() const
    {
        return m_ArgTypeQualifier.end();
    }

    size_t size_ArgTypeQualifier()  const
    {
        return m_ArgTypeQualifier.size();
    }

    bool empty_ArgTypeQualifier()  const
    {
        return m_ArgTypeQualifier.empty();
    }

    bool isArgTypeQualifierHasValue() const
    {
        return m_ArgTypeQualifier.hasValue();
    }

    ArgTypeQualifierList::item_type getArgTypeQualifierItem( size_t index )
    {
        return m_ArgTypeQualifier.getItem(index);
    }

    void setArgTypeQualifierItem( size_t index, const ArgTypeQualifierList::item_type& item  )
    {
        return m_ArgTypeQualifier.setItem(index, item);
    }

    void addArgTypeQualifierItem(const ArgTypeQualifierList::item_type& val)
    {
        m_ArgTypeQualifier.push_back(val);
    }

    ArgTypeQualifierList::iterator eraseArgTypeQualifierItem(ArgTypeQualifierList::iterator i)
    {
        return m_ArgTypeQualifier.erase(i);
    }
        
    
    /// ArgName related methods
    ArgNameList::iterator begin_ArgName()
    {
        return m_ArgName.begin();
    }

    ArgNameList::iterator end_ArgName()
    {
        return m_ArgName.end();
    }

    ArgNameList::const_iterator begin_ArgName() const
    {
        return m_ArgName.begin();
    }

    ArgNameList::const_iterator end_ArgName() const
    {
        return m_ArgName.end();
    }

    size_t size_ArgName()  const
    {
        return m_ArgName.size();
    }

    bool empty_ArgName()  const
    {
        return m_ArgName.empty();
    }

    bool isArgNameHasValue() const
    {
        return m_ArgName.hasValue();
    }

    ArgNameList::item_type getArgNameItem( size_t index )
    {
        return m_ArgName.getItem(index);
    }

    void setArgNameItem( size_t index, const ArgNameList::item_type& item  )
    {
        return m_ArgName.setItem(index, item);
    }

    void addArgNameItem(const ArgNameList::item_type& val)
    {
        m_ArgName.push_back(val);
    }

    ArgNameList::iterator eraseArgNameItem(ArgNameList::iterator i)
    {
        return m_ArgName.erase(i);
    }

    ///
    // Returns true if any of the KernelMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the KernelMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getFunctionNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getVecTypeHintNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getWorkGroupSizeHintNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getReqdWorkGroupSizeNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getAddressSpaceNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getArgAccessQualifierNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getArgTypesNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getArgTypeQualifierNode( const llvm::MDNode* pParentNode) const;
        
    llvm::MDNode* getArgNameNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    MetaDataValue<llvm::Function> m_Function;        
    NamedMetaDataValue<llvm::UndefValue> m_VecTypeHint;        
    WorkGroupSizeMetaDataHandle m_WorkGroupSizeHint;        
    WorkGroupSizeMetaDataHandle m_ReqdWorkGroupSize;        
    MetaDataList<int32_t> m_AddressSpace;        
    MetaDataList<std::string> m_ArgAccessQualifier;        
    MetaDataList<std::string> m_ArgTypes;        
    MetaDataList<std::string> m_ArgTypeQualifier;        
    MetaDataList<std::string> m_ArgName;
    // parent node
    const llvm::MDNode* m_pNode;
};
        
        
        
    
///
// Read/Write the WorkGroupSize structure from/to LLVM metadata
//
class WorkGroupSizeMetaData:public IMetaDataObject
{
public:
    typedef WorkGroupSizeMetaData _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    typedef MetaDataValue<int32_t>::value_type XDimType;        
    typedef MetaDataValue<int32_t>::value_type YDimType;        
    typedef MetaDataValue<int32_t>::value_type ZDimType;

public:
    ///
    // Factory method - creates the WorkGroupSizeMetaData from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty WorkGroupSizeMetaData object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named WorkGroupSizeMetaData object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the WorkGroupSizeMetaData from the given metadata node
    //
    WorkGroupSizeMetaData(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named WorkGroupSizeMetaData object
    //
    WorkGroupSizeMetaData();

    ///
    // Ctor - creates the empty, named WorkGroupSizeMetaData object
    //
    WorkGroupSizeMetaData(const char* name);

    /// XDim related methods
    XDimType getXDim()
    {
        return m_XDim.get();
    }

    void setXDim( const XDimType& val)
    {
        m_XDim.set(val);
    }

    bool isXDimHasValue() const
    {
        return m_XDim.hasValue();
    }

        
    
    /// YDim related methods
    YDimType getYDim()
    {
        return m_YDim.get();
    }

    void setYDim( const YDimType& val)
    {
        m_YDim.set(val);
    }

    bool isYDimHasValue() const
    {
        return m_YDim.hasValue();
    }

        
    
    /// ZDim related methods
    ZDimType getZDim()
    {
        return m_ZDim.get();
    }

    void setZDim( const ZDimType& val)
    {
        m_ZDim.set(val);
    }

    bool isZDimHasValue() const
    {
        return m_ZDim.hasValue();
    }

    ///
    // Returns true if any of the WorkGroupSizeMetaData`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the WorkGroupSizeMetaData instance
    void discardChanges();

    ///
    // Generates the new MDNode hierarchy for the given structure
    llvm::Value* generateNode(llvm::LLVMContext& context) const;

    ///
    // Saves the structure changes to the given MDNode
    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const;

private:
    ///
    // Returns true if the given MDNode could be saved to without replacement
    bool compatibleWith( const llvm::MDNode* pNode) const
    {
        return false;
    }

private:
    typedef MetaDataIterator<llvm::Value> NodeIterator;

    llvm::Value* getXDimNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getYDimNode( const llvm::MDNode* pParentNode) const;
        
    llvm::Value* getZDimNode( const llvm::MDNode* pParentNode) const;

private:
    // data members
    MetaDataValue<int32_t> m_XDim;        
    MetaDataValue<int32_t> m_YDim;        
    MetaDataValue<int32_t> m_ZDim;
    // parent node
    const llvm::MDNode* m_pNode;
};


class MetaDataUtils
{
public:
    // typedefs for the data members types
    typedef NamedMDNodeList<KernelMetaDataHandle> KernelsList;        
    typedef NamedMDNodeList<int32_t> SpirVersionList;        
    typedef NamedMDNodeList<int32_t> OpenCLVersionList;        
    typedef NamedMDNodeList<int32_t> UsedExtentionsList;        
    typedef NamedMDNodeList<std::string> CoreFeaturesList;        
    typedef NamedMDNodeList<StrWrapperMetaDataHandle> CompilerOptionsList;        
    typedef NamedMetaDataMap<llvm::Function, KernelInfoMetaDataHandle> KernelsInfoMap;        
    typedef NamedMDNodeList<ModuleInfoMetaDataHandle> ModuleInfoListList;        
    typedef NamedMetaDataMap<llvm::Function, FunctionInfoMetaDataHandle> FunctionsInfoMap;

public:
    MetaDataUtils(llvm::Module* pModule):
        m_Kernels(pModule->getOrInsertNamedMetadata("opencl.kernels")),            
        m_SpirVersion(pModule->getOrInsertNamedMetadata("opencl.spir.version")),            
        m_OpenCLVersion(pModule->getOrInsertNamedMetadata("opencl.ocl.version")),            
        m_UsedExtentions(pModule->getOrInsertNamedMetadata("opencl.used.extensions")),            
        m_CoreFeatures(pModule->getOrInsertNamedMetadata("opencl.used.optional.core.features")),            
        m_CompilerOptions(pModule->getOrInsertNamedMetadata("opencl.compiler.options")),            
        m_KernelsInfo(pModule->getOrInsertNamedMetadata("opencl.kernel_info")),            
        m_ModuleInfoList(pModule->getOrInsertNamedMetadata("opencl.module_info_list")),            
        m_FunctionsInfo(pModule->getOrInsertNamedMetadata("llvm.functions_info")),
        m_pModule(pModule)
    {
    }

    ~MetaDataUtils();

    /// Kernels related methods
    KernelsList::iterator begin_Kernels()
    {
        return m_Kernels.begin();
    }

    KernelsList::iterator end_Kernels()
    {
        return m_Kernels.end();
    }

    KernelsList::const_iterator begin_Kernels() const
    {
        return m_Kernels.begin();
    }

    KernelsList::const_iterator end_Kernels() const
    {
        return m_Kernels.end();
    }

    size_t size_Kernels()  const
    {
        return m_Kernels.size();
    }

    bool empty_Kernels()  const
    {
        return m_Kernels.empty();
    }

    bool isKernelsHasValue() const
    {
        return m_Kernels.hasValue();
    }

    KernelsList::item_type getKernelsItem( size_t index )
    {
        return m_Kernels.getItem(index);
    }

    void setKernelsItem( size_t index, const KernelsList::item_type& item  )
    {
        return m_Kernels.setItem(index, item);
    }

    void addKernelsItem(const KernelsList::item_type& val)
    {
        m_Kernels.push_back(val);
    }

    KernelsList::iterator eraseKernelsItem(KernelsList::iterator it)
    {
        return m_Kernels.erase(it);
    }

        
    
    /// SpirVersion related methods
    SpirVersionList::iterator begin_SpirVersion()
    {
        return m_SpirVersion.begin();
    }

    SpirVersionList::iterator end_SpirVersion()
    {
        return m_SpirVersion.end();
    }

    SpirVersionList::const_iterator begin_SpirVersion() const
    {
        return m_SpirVersion.begin();
    }

    SpirVersionList::const_iterator end_SpirVersion() const
    {
        return m_SpirVersion.end();
    }

    size_t size_SpirVersion()  const
    {
        return m_SpirVersion.size();
    }

    bool empty_SpirVersion()  const
    {
        return m_SpirVersion.empty();
    }

    bool isSpirVersionHasValue() const
    {
        return m_SpirVersion.hasValue();
    }

    SpirVersionList::item_type getSpirVersionItem( size_t index )
    {
        return m_SpirVersion.getItem(index);
    }

    void setSpirVersionItem( size_t index, const SpirVersionList::item_type& item  )
    {
        return m_SpirVersion.setItem(index, item);
    }

    void addSpirVersionItem(const SpirVersionList::item_type& val)
    {
        m_SpirVersion.push_back(val);
    }

    SpirVersionList::iterator eraseSpirVersionItem(SpirVersionList::iterator it)
    {
        return m_SpirVersion.erase(it);
    }

        
    
    /// OpenCLVersion related methods
    OpenCLVersionList::iterator begin_OpenCLVersion()
    {
        return m_OpenCLVersion.begin();
    }

    OpenCLVersionList::iterator end_OpenCLVersion()
    {
        return m_OpenCLVersion.end();
    }

    OpenCLVersionList::const_iterator begin_OpenCLVersion() const
    {
        return m_OpenCLVersion.begin();
    }

    OpenCLVersionList::const_iterator end_OpenCLVersion() const
    {
        return m_OpenCLVersion.end();
    }

    size_t size_OpenCLVersion()  const
    {
        return m_OpenCLVersion.size();
    }

    bool empty_OpenCLVersion()  const
    {
        return m_OpenCLVersion.empty();
    }

    bool isOpenCLVersionHasValue() const
    {
        return m_OpenCLVersion.hasValue();
    }

    OpenCLVersionList::item_type getOpenCLVersionItem( size_t index )
    {
        return m_OpenCLVersion.getItem(index);
    }

    void setOpenCLVersionItem( size_t index, const OpenCLVersionList::item_type& item  )
    {
        return m_OpenCLVersion.setItem(index, item);
    }

    void addOpenCLVersionItem(const OpenCLVersionList::item_type& val)
    {
        m_OpenCLVersion.push_back(val);
    }

    OpenCLVersionList::iterator eraseOpenCLVersionItem(OpenCLVersionList::iterator it)
    {
        return m_OpenCLVersion.erase(it);
    }

        
    
    /// UsedExtentions related methods
    UsedExtentionsList::iterator begin_UsedExtentions()
    {
        return m_UsedExtentions.begin();
    }

    UsedExtentionsList::iterator end_UsedExtentions()
    {
        return m_UsedExtentions.end();
    }

    UsedExtentionsList::const_iterator begin_UsedExtentions() const
    {
        return m_UsedExtentions.begin();
    }

    UsedExtentionsList::const_iterator end_UsedExtentions() const
    {
        return m_UsedExtentions.end();
    }

    size_t size_UsedExtentions()  const
    {
        return m_UsedExtentions.size();
    }

    bool empty_UsedExtentions()  const
    {
        return m_UsedExtentions.empty();
    }

    bool isUsedExtentionsHasValue() const
    {
        return m_UsedExtentions.hasValue();
    }

    UsedExtentionsList::item_type getUsedExtentionsItem( size_t index )
    {
        return m_UsedExtentions.getItem(index);
    }

    void setUsedExtentionsItem( size_t index, const UsedExtentionsList::item_type& item  )
    {
        return m_UsedExtentions.setItem(index, item);
    }

    void addUsedExtentionsItem(const UsedExtentionsList::item_type& val)
    {
        m_UsedExtentions.push_back(val);
    }

    UsedExtentionsList::iterator eraseUsedExtentionsItem(UsedExtentionsList::iterator it)
    {
        return m_UsedExtentions.erase(it);
    }

        
    
    /// CoreFeatures related methods
    CoreFeaturesList::iterator begin_CoreFeatures()
    {
        return m_CoreFeatures.begin();
    }

    CoreFeaturesList::iterator end_CoreFeatures()
    {
        return m_CoreFeatures.end();
    }

    CoreFeaturesList::const_iterator begin_CoreFeatures() const
    {
        return m_CoreFeatures.begin();
    }

    CoreFeaturesList::const_iterator end_CoreFeatures() const
    {
        return m_CoreFeatures.end();
    }

    size_t size_CoreFeatures()  const
    {
        return m_CoreFeatures.size();
    }

    bool empty_CoreFeatures()  const
    {
        return m_CoreFeatures.empty();
    }

    bool isCoreFeaturesHasValue() const
    {
        return m_CoreFeatures.hasValue();
    }

    CoreFeaturesList::item_type getCoreFeaturesItem( size_t index )
    {
        return m_CoreFeatures.getItem(index);
    }

    void setCoreFeaturesItem( size_t index, const CoreFeaturesList::item_type& item  )
    {
        return m_CoreFeatures.setItem(index, item);
    }

    void addCoreFeaturesItem(const CoreFeaturesList::item_type& val)
    {
        m_CoreFeatures.push_back(val);
    }

    CoreFeaturesList::iterator eraseCoreFeaturesItem(CoreFeaturesList::iterator it)
    {
        return m_CoreFeatures.erase(it);
    }

        
    
    /// CompilerOptions related methods
    CompilerOptionsList::iterator begin_CompilerOptions()
    {
        return m_CompilerOptions.begin();
    }

    CompilerOptionsList::iterator end_CompilerOptions()
    {
        return m_CompilerOptions.end();
    }

    CompilerOptionsList::const_iterator begin_CompilerOptions() const
    {
        return m_CompilerOptions.begin();
    }

    CompilerOptionsList::const_iterator end_CompilerOptions() const
    {
        return m_CompilerOptions.end();
    }

    size_t size_CompilerOptions()  const
    {
        return m_CompilerOptions.size();
    }

    bool empty_CompilerOptions()  const
    {
        return m_CompilerOptions.empty();
    }

    bool isCompilerOptionsHasValue() const
    {
        return m_CompilerOptions.hasValue();
    }

    CompilerOptionsList::item_type getCompilerOptionsItem( size_t index )
    {
        return m_CompilerOptions.getItem(index);
    }

    void setCompilerOptionsItem( size_t index, const CompilerOptionsList::item_type& item  )
    {
        return m_CompilerOptions.setItem(index, item);
    }

    void addCompilerOptionsItem(const CompilerOptionsList::item_type& val)
    {
        m_CompilerOptions.push_back(val);
    }

    CompilerOptionsList::iterator eraseCompilerOptionsItem(CompilerOptionsList::iterator it)
    {
        return m_CompilerOptions.erase(it);
    }

        
    
    /// KernelsInfo related methods
    KernelsInfoMap::iterator begin_KernelsInfo()
    {
        return m_KernelsInfo.begin();
    }

    KernelsInfoMap::iterator end_KernelsInfo()
    {
        return m_KernelsInfo.end();
    }

    KernelsInfoMap::const_iterator begin_KernelsInfo() const
    {
        return m_KernelsInfo.begin();
    }

    KernelsInfoMap::const_iterator end_KernelsInfo() const
    {
        return m_KernelsInfo.end();
    }

    size_t size_KernelsInfo()  const
    {
        return m_KernelsInfo.size();
    }

    bool empty_KernelsInfo()  const
    {
        return m_KernelsInfo.empty();
    }

    bool isKernelsInfoHasValue() const
    {
        return m_KernelsInfo.hasValue();
    }

    KernelsInfoMap::item_type getKernelsInfoItem( const KernelsInfoMap::key_type& index )
    {
        return m_KernelsInfo.getItem(index);
    }

    KernelsInfoMap::item_type getOrInsertKernelsInfoItem( const KernelsInfoMap::key_type& index )
    {
        return m_KernelsInfo.getOrInsertItem(index);
    }

    void setKernelsInfoItem( const KernelsInfoMap::key_type& index, const KernelsInfoMap::item_type& item  )
    {
        return m_KernelsInfo.setItem(index, item);
    }

    KernelsInfoMap::iterator findKernelsInfoItem(const KernelsInfoMap::key_type& key)
    {
        return m_KernelsInfo.find(key);
    }

    void eraseKernelsInfoItem(KernelsInfoMap::iterator it)
    {
        m_KernelsInfo.erase(it);
    }

        
    
    /// ModuleInfoList related methods
    ModuleInfoListList::iterator begin_ModuleInfoList()
    {
        return m_ModuleInfoList.begin();
    }

    ModuleInfoListList::iterator end_ModuleInfoList()
    {
        return m_ModuleInfoList.end();
    }

    ModuleInfoListList::const_iterator begin_ModuleInfoList() const
    {
        return m_ModuleInfoList.begin();
    }

    ModuleInfoListList::const_iterator end_ModuleInfoList() const
    {
        return m_ModuleInfoList.end();
    }

    size_t size_ModuleInfoList()  const
    {
        return m_ModuleInfoList.size();
    }

    bool empty_ModuleInfoList()  const
    {
        return m_ModuleInfoList.empty();
    }

    bool isModuleInfoListHasValue() const
    {
        return m_ModuleInfoList.hasValue();
    }

    ModuleInfoListList::item_type getModuleInfoListItem( size_t index )
    {
        return m_ModuleInfoList.getItem(index);
    }

    void setModuleInfoListItem( size_t index, const ModuleInfoListList::item_type& item  )
    {
        return m_ModuleInfoList.setItem(index, item);
    }

    void addModuleInfoListItem(const ModuleInfoListList::item_type& val)
    {
        m_ModuleInfoList.push_back(val);
    }

    ModuleInfoListList::iterator eraseModuleInfoListItem(ModuleInfoListList::iterator it)
    {
        return m_ModuleInfoList.erase(it);
    }

        
    
    /// FunctionsInfo related methods
    FunctionsInfoMap::iterator begin_FunctionsInfo()
    {
        return m_FunctionsInfo.begin();
    }

    FunctionsInfoMap::iterator end_FunctionsInfo()
    {
        return m_FunctionsInfo.end();
    }

    FunctionsInfoMap::const_iterator begin_FunctionsInfo() const
    {
        return m_FunctionsInfo.begin();
    }

    FunctionsInfoMap::const_iterator end_FunctionsInfo() const
    {
        return m_FunctionsInfo.end();
    }

    size_t size_FunctionsInfo()  const
    {
        return m_FunctionsInfo.size();
    }

    bool empty_FunctionsInfo()  const
    {
        return m_FunctionsInfo.empty();
    }

    bool isFunctionsInfoHasValue() const
    {
        return m_FunctionsInfo.hasValue();
    }

    FunctionsInfoMap::item_type getFunctionsInfoItem( const FunctionsInfoMap::key_type& index )
    {
        return m_FunctionsInfo.getItem(index);
    }

    FunctionsInfoMap::item_type getOrInsertFunctionsInfoItem( const FunctionsInfoMap::key_type& index )
    {
        return m_FunctionsInfo.getOrInsertItem(index);
    }

    void setFunctionsInfoItem( const FunctionsInfoMap::key_type& index, const FunctionsInfoMap::item_type& item  )
    {
        return m_FunctionsInfo.setItem(index, item);
    }

    FunctionsInfoMap::iterator findFunctionsInfoItem(const FunctionsInfoMap::key_type& key)
    {
        return m_FunctionsInfo.find(key);
    }

    void eraseFunctionsInfoItem(FunctionsInfoMap::iterator it)
    {
        m_FunctionsInfo.erase(it);
    }

    bool dirty()
    {
        if( m_Kernels.dirty() )
        {
            return true;
        }            
        if( m_SpirVersion.dirty() )
        {
            return true;
        }            
        if( m_OpenCLVersion.dirty() )
        {
            return true;
        }            
        if( m_UsedExtentions.dirty() )
        {
            return true;
        }            
        if( m_CoreFeatures.dirty() )
        {
            return true;
        }            
        if( m_CompilerOptions.dirty() )
        {
            return true;
        }            
        if( m_KernelsInfo.dirty() )
        {
            return true;
        }            
        if( m_ModuleInfoList.dirty() )
        {
            return true;
        }            
        if( m_FunctionsInfo.dirty() )
        {
            return true;
        }
        return false;
    }

    void save(llvm::LLVMContext& context)
    {
        if( m_Kernels.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.kernels");
            m_Kernels.save(context, pNode);
        }            
        if( m_SpirVersion.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.spir.version");
            m_SpirVersion.save(context, pNode);
        }            
        if( m_OpenCLVersion.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.ocl.version");
            m_OpenCLVersion.save(context, pNode);
        }            
        if( m_UsedExtentions.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.used.extensions");
            m_UsedExtentions.save(context, pNode);
        }            
        if( m_CoreFeatures.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.used.optional.core.features");
            m_CoreFeatures.save(context, pNode);
        }            
        if( m_CompilerOptions.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.compiler.options");
            m_CompilerOptions.save(context, pNode);
        }            
        if( m_KernelsInfo.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.kernel_info");
            m_KernelsInfo.save(context, pNode);
        }            
        if( m_ModuleInfoList.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("opencl.module_info_list");
            m_ModuleInfoList.save(context, pNode);
        }            
        if( m_FunctionsInfo.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("llvm.functions_info");
            m_FunctionsInfo.save(context, pNode);
        }
        discardChanges();
    }

    void discardChanges()
    {
        m_Kernels.discardChanges();            
        m_SpirVersion.discardChanges();            
        m_OpenCLVersion.discardChanges();            
        m_UsedExtentions.discardChanges();            
        m_CoreFeatures.discardChanges();            
        m_CompilerOptions.discardChanges();            
        m_KernelsInfo.discardChanges();            
        m_ModuleInfoList.discardChanges();            
        m_FunctionsInfo.discardChanges();
    }

private:
    // data members
    NamedMDNodeList<KernelMetaDataHandle> m_Kernels;        
    NamedMDNodeList<int32_t> m_SpirVersion;        
    NamedMDNodeList<int32_t> m_OpenCLVersion;        
    NamedMDNodeList<int32_t> m_UsedExtentions;        
    NamedMDNodeList<std::string> m_CoreFeatures;        
    NamedMDNodeList<StrWrapperMetaDataHandle> m_CompilerOptions;        
    NamedMetaDataMap<llvm::Function, KernelInfoMetaDataHandle> m_KernelsInfo;        
    NamedMDNodeList<ModuleInfoMetaDataHandle> m_ModuleInfoList;        
    NamedMetaDataMap<llvm::Function, FunctionInfoMetaDataHandle> m_FunctionsInfo;
    llvm::Module* m_pModule;
};


} //namespace
#endif
