/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/


#include "MetaDataApi.h"

namespace Intel
{
///
// Ctor - loads the FunctionInfoMetaData from the given metadata node
//
FunctionInfoMetaData::FunctionInfoMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_FuncPtrCall(getFuncPtrCallNode(pNode)),
        
    m_HasRecursion(getHasRecursionNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named FunctionInfoMetaData object
//
FunctionInfoMetaData::FunctionInfoMetaData():
    m_FuncPtrCall("func_ptr_call"),        
    m_HasRecursion("detect_recursion"),
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named FunctionInfoMetaData object
//
FunctionInfoMetaData::FunctionInfoMetaData(const char* name):
    _Mybase(name),
    m_FuncPtrCall("func_ptr_call"),        
    m_HasRecursion("detect_recursion"),
    m_pNode(NULL)
{}

///
// Returns true if any of the FunctionInfoMetaData`s members has changed
bool FunctionInfoMetaData::dirty() const
{
    if( m_FuncPtrCall.dirty() )
    {
        return true;
    }        
    if( m_HasRecursion.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the FunctionInfoMetaData instance
void FunctionInfoMetaData::discardChanges()
{
    m_FuncPtrCall.discardChanges();        
    m_HasRecursion.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* FunctionInfoMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_FuncPtrCall.generateNode(context));        
    args.push_back( m_HasRecursion.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void FunctionInfoMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_FuncPtrCall.save(context, llvm::cast<llvm::MDNode>(getFuncPtrCallNode(pNode)));        
    m_HasRecursion.save(context, llvm::cast<llvm::MDNode>(getHasRecursionNode(pNode)));
}

llvm::Value* FunctionInfoMetaData::getFuncPtrCallNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "func_ptr_call") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* FunctionInfoMetaData::getHasRecursionNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "detect_recursion") )
        {
            return i.get();
        }
    }
    return NULL;
}

        
        
        
    

///
// Ctor - loads the StatValueListContainerMetaData from the given metadata node
//
StatValueListContainerMetaData::StatValueListContainerMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_StatValueList(getStatValueListNode(pNode), false),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named StatValueListContainerMetaData object
//
StatValueListContainerMetaData::StatValueListContainerMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named StatValueListContainerMetaData object
//
StatValueListContainerMetaData::StatValueListContainerMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the StatValueListContainerMetaData`s members has changed
bool StatValueListContainerMetaData::dirty() const
{
    if( m_StatValueList.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the StatValueListContainerMetaData instance
void StatValueListContainerMetaData::discardChanges()
{
    m_StatValueList.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* StatValueListContainerMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_StatValueList.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void StatValueListContainerMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_StatValueList.save(context, llvm::cast<llvm::MDNode>(getStatValueListNode(pNode)));
}

llvm::MDNode* StatValueListContainerMetaData::getStatValueListNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return llvm::dyn_cast<llvm::MDNode>(pParentNode->getOperand(0 + offset));
}

        
        
        
        
        
        
        
    

///
// Ctor - loads the ModuleStatInfoMetaData from the given metadata node
//
ModuleStatInfoMetaData::ModuleStatInfoMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_RunTimeVersion(getRunTimeVersionNode(pNode)),
        
    m_WorkloadName(getWorkloadNameNode(pNode)),
        
    m_ModuleName(getModuleNameNode(pNode)),
        
    m_ExecTime(getExecTimeNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named ModuleStatInfoMetaData object
//
ModuleStatInfoMetaData::ModuleStatInfoMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named ModuleStatInfoMetaData object
//
ModuleStatInfoMetaData::ModuleStatInfoMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the ModuleStatInfoMetaData`s members has changed
bool ModuleStatInfoMetaData::dirty() const
{
    if( m_RunTimeVersion.dirty() )
    {
        return true;
    }        
    if( m_WorkloadName.dirty() )
    {
        return true;
    }        
    if( m_ModuleName.dirty() )
    {
        return true;
    }        
    if( m_ExecTime.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the ModuleStatInfoMetaData instance
void ModuleStatInfoMetaData::discardChanges()
{
    m_RunTimeVersion.discardChanges();        
    m_WorkloadName.discardChanges();        
    m_ModuleName.discardChanges();        
    m_ExecTime.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* ModuleStatInfoMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_RunTimeVersion.generateNode(context));        
    args.push_back( m_WorkloadName.generateNode(context));        
    args.push_back( m_ModuleName.generateNode(context));        
    args.push_back( m_ExecTime.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void ModuleStatInfoMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_RunTimeVersion.save(context, llvm::cast<llvm::MDNode>(getRunTimeVersionNode(pNode)));        
    m_WorkloadName.save(context, llvm::cast<llvm::MDNode>(getWorkloadNameNode(pNode)));        
    m_ModuleName.save(context, llvm::cast<llvm::MDNode>(getModuleNameNode(pNode)));        
    m_ExecTime.save(context, llvm::cast<llvm::MDNode>(getExecTimeNode(pNode)));
}

llvm::Value* ModuleStatInfoMetaData::getRunTimeVersionNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}
    
llvm::Value* ModuleStatInfoMetaData::getWorkloadNameNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(1 + offset);
}
    
llvm::Value* ModuleStatInfoMetaData::getModuleNameNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(2 + offset);
}
    
llvm::Value* ModuleStatInfoMetaData::getExecTimeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(3 + offset);
}

        
    

///
// Ctor - loads the StatValueItemMetaData from the given metadata node
//
StatValueItemMetaData::StatValueItemMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_Name(getNameNode(pNode)),
        
    m_Value(getValueNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named StatValueItemMetaData object
//
StatValueItemMetaData::StatValueItemMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named StatValueItemMetaData object
//
StatValueItemMetaData::StatValueItemMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the StatValueItemMetaData`s members has changed
bool StatValueItemMetaData::dirty() const
{
    if( m_Name.dirty() )
    {
        return true;
    }        
    if( m_Value.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the StatValueItemMetaData instance
void StatValueItemMetaData::discardChanges()
{
    m_Name.discardChanges();        
    m_Value.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* StatValueItemMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_Name.generateNode(context));        
    args.push_back( m_Value.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void StatValueItemMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_Name.save(context, llvm::cast<llvm::MDNode>(getNameNode(pNode)));        
    m_Value.save(context, llvm::cast<llvm::MDNode>(getValueNode(pNode)));
}

llvm::Value* StatValueItemMetaData::getNameNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}
    
llvm::Value* StatValueItemMetaData::getValueNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(1 + offset);
}

        
        
        
        
        
        
    

///
// Ctor - loads the ModuleInfoMetaData from the given metadata node
//
ModuleInfoMetaData::ModuleInfoMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_GlobalVariableTotalSize(getGlobalVariableTotalSizeNode(pNode)),
        
    m_GAScounter(getGAScounterNode(pNode)),
        
    m_GASwarnings(getGASwarningsNode(pNode), true),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named ModuleInfoMetaData object
//
ModuleInfoMetaData::ModuleInfoMetaData():
    m_GlobalVariableTotalSize("global_variable_total_size"),        
    m_GAScounter("gen_addr_space_pointer_counter"),        
    m_GASwarnings("gen_addr_space_pointer_warnings"),
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named ModuleInfoMetaData object
//
ModuleInfoMetaData::ModuleInfoMetaData(const char* name):
    _Mybase(name),
    m_GlobalVariableTotalSize("global_variable_total_size"),        
    m_GAScounter("gen_addr_space_pointer_counter"),        
    m_GASwarnings("gen_addr_space_pointer_warnings"),
    m_pNode(NULL)
{}

///
// Returns true if any of the ModuleInfoMetaData`s members has changed
bool ModuleInfoMetaData::dirty() const
{
    if( m_GlobalVariableTotalSize.dirty() )
    {
        return true;
    }        
    if( m_GAScounter.dirty() )
    {
        return true;
    }        
    if( m_GASwarnings.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the ModuleInfoMetaData instance
void ModuleInfoMetaData::discardChanges()
{
    m_GlobalVariableTotalSize.discardChanges();        
    m_GAScounter.discardChanges();        
    m_GASwarnings.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* ModuleInfoMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_GlobalVariableTotalSize.generateNode(context));        
    args.push_back( m_GAScounter.generateNode(context));        
    args.push_back( m_GASwarnings.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void ModuleInfoMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_GlobalVariableTotalSize.save(context, llvm::cast<llvm::MDNode>(getGlobalVariableTotalSizeNode(pNode)));        
    m_GAScounter.save(context, llvm::cast<llvm::MDNode>(getGAScounterNode(pNode)));        
    m_GASwarnings.save(context, llvm::cast<llvm::MDNode>(getGASwarningsNode(pNode)));
}

llvm::Value* ModuleInfoMetaData::getGlobalVariableTotalSizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "global_variable_total_size") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* ModuleInfoMetaData::getGAScounterNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "gen_addr_space_pointer_counter") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::MDNode* ModuleInfoMetaData::getGASwarningsNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "gen_addr_space_pointer_warnings") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}

        
    

///
// Ctor - loads the StrWrapperMetaData from the given metadata node
//
StrWrapperMetaData::StrWrapperMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_value(getvalueNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named StrWrapperMetaData object
//
StrWrapperMetaData::StrWrapperMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named StrWrapperMetaData object
//
StrWrapperMetaData::StrWrapperMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the StrWrapperMetaData`s members has changed
bool StrWrapperMetaData::dirty() const
{
    if( m_value.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the StrWrapperMetaData instance
void StrWrapperMetaData::discardChanges()
{
    m_value.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* StrWrapperMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_value.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void StrWrapperMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_value.save(context, llvm::cast<llvm::MDNode>(getvalueNode(pNode)));
}

llvm::Value* StrWrapperMetaData::getvalueNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}

        
        
    

///
// Ctor - loads the KernelInfoMetaData from the given metadata node
//
KernelInfoMetaData::KernelInfoMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_LocalBufferSize(getLocalBufferSizeNode(pNode)),
        
    m_BarrierBufferSize(getBarrierBufferSizeNode(pNode)),
        
    m_KernelExecutionLength(getKernelExecutionLengthNode(pNode)),
        
    m_MaxWGDimensions(getMaxWGDimensionsNode(pNode)),
        
    m_KernelHasBarrier(getKernelHasBarrierNode(pNode)),
        
    m_KernelHasGlobalSync(getKernelHasGlobalSyncNode(pNode)),
        
    m_NoBarrierPath(getNoBarrierPathNode(pNode)),
        
    m_VectorizedKernel(getVectorizedKernelNode(pNode)),
        
    m_VectorizedWidth(getVectorizedWidthNode(pNode)),
        
    m_KernelWrapper(getKernelWrapperNode(pNode)),
        
    m_ScalarizedKernel(getScalarizedKernelNode(pNode)),
        
    m_BlockLiteralSize(getBlockLiteralSizeNode(pNode)),
        
    m_PrivateMemorySize(getPrivateMemorySizeNode(pNode)),
        
    m_VectorizationDimension(getVectorizationDimensionNode(pNode)),
        
    m_CanUniteWorkgroups(getCanUniteWorkgroupsNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named KernelInfoMetaData object
//
KernelInfoMetaData::KernelInfoMetaData():
    m_LocalBufferSize("local_buffer_size"),        
    m_BarrierBufferSize("barrier_buffer_size"),        
    m_KernelExecutionLength("kernel_execution_length"),        
    m_MaxWGDimensions("max_wg_dimensions"),        
    m_KernelHasBarrier("kernel_has_barrier"),        
    m_KernelHasGlobalSync("kernel_has_global_sync"),        
    m_NoBarrierPath("no_barrier_path"),        
    m_VectorizedKernel("vectorized_kernel"),        
    m_VectorizedWidth("vectorized_width"),        
    m_KernelWrapper("kernel_wrapper"),        
    m_ScalarizedKernel("scalarized_kernel"),        
    m_BlockLiteralSize("block_literal_size"),        
    m_PrivateMemorySize("private_memory_size"),        
    m_VectorizationDimension("vectorization_dimension"),        
    m_CanUniteWorkgroups("can_unite_workgroups"),
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named KernelInfoMetaData object
//
KernelInfoMetaData::KernelInfoMetaData(const char* name):
    _Mybase(name),
    m_LocalBufferSize("local_buffer_size"),        
    m_BarrierBufferSize("barrier_buffer_size"),        
    m_KernelExecutionLength("kernel_execution_length"),        
    m_MaxWGDimensions("max_wg_dimensions"),        
    m_KernelHasBarrier("kernel_has_barrier"),        
    m_KernelHasGlobalSync("kernel_has_global_sync"),        
    m_NoBarrierPath("no_barrier_path"),        
    m_VectorizedKernel("vectorized_kernel"),        
    m_VectorizedWidth("vectorized_width"),        
    m_KernelWrapper("kernel_wrapper"),        
    m_ScalarizedKernel("scalarized_kernel"),        
    m_BlockLiteralSize("block_literal_size"),        
    m_PrivateMemorySize("private_memory_size"),        
    m_VectorizationDimension("vectorization_dimension"),        
    m_CanUniteWorkgroups("can_unite_workgroups"),
    m_pNode(NULL)
{}

///
// Returns true if any of the KernelInfoMetaData`s members has changed
bool KernelInfoMetaData::dirty() const
{
    if( m_LocalBufferSize.dirty() )
    {
        return true;
    }        
    if( m_BarrierBufferSize.dirty() )
    {
        return true;
    }        
    if( m_KernelExecutionLength.dirty() )
    {
        return true;
    }        
    if( m_MaxWGDimensions.dirty() )
    {
        return true;
    }        
    if( m_KernelHasBarrier.dirty() )
    {
        return true;
    }        
    if( m_KernelHasGlobalSync.dirty() )
    {
        return true;
    }        
    if( m_NoBarrierPath.dirty() )
    {
        return true;
    }        
    if( m_VectorizedKernel.dirty() )
    {
        return true;
    }        
    if( m_VectorizedWidth.dirty() )
    {
        return true;
    }        
    if( m_KernelWrapper.dirty() )
    {
        return true;
    }        
    if( m_ScalarizedKernel.dirty() )
    {
        return true;
    }        
    if( m_BlockLiteralSize.dirty() )
    {
        return true;
    }        
    if( m_PrivateMemorySize.dirty() )
    {
        return true;
    }        
    if( m_VectorizationDimension.dirty() )
    {
        return true;
    }        
    if( m_CanUniteWorkgroups.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the KernelInfoMetaData instance
void KernelInfoMetaData::discardChanges()
{
    m_LocalBufferSize.discardChanges();        
    m_BarrierBufferSize.discardChanges();        
    m_KernelExecutionLength.discardChanges();        
    m_MaxWGDimensions.discardChanges();        
    m_KernelHasBarrier.discardChanges();        
    m_KernelHasGlobalSync.discardChanges();        
    m_NoBarrierPath.discardChanges();        
    m_VectorizedKernel.discardChanges();        
    m_VectorizedWidth.discardChanges();        
    m_KernelWrapper.discardChanges();        
    m_ScalarizedKernel.discardChanges();        
    m_BlockLiteralSize.discardChanges();        
    m_PrivateMemorySize.discardChanges();        
    m_VectorizationDimension.discardChanges();        
    m_CanUniteWorkgroups.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* KernelInfoMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_LocalBufferSize.generateNode(context));        
    args.push_back( m_BarrierBufferSize.generateNode(context));        
    args.push_back( m_KernelExecutionLength.generateNode(context));        
    args.push_back( m_MaxWGDimensions.generateNode(context));        
    args.push_back( m_KernelHasBarrier.generateNode(context));        
    args.push_back( m_KernelHasGlobalSync.generateNode(context));        
    args.push_back( m_NoBarrierPath.generateNode(context));        
    args.push_back( m_VectorizedKernel.generateNode(context));        
    args.push_back( m_VectorizedWidth.generateNode(context));        
    args.push_back( m_KernelWrapper.generateNode(context));        
    args.push_back( m_ScalarizedKernel.generateNode(context));        
    args.push_back( m_BlockLiteralSize.generateNode(context));        
    args.push_back( m_PrivateMemorySize.generateNode(context));        
    args.push_back( m_VectorizationDimension.generateNode(context));        
    args.push_back( m_CanUniteWorkgroups.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void KernelInfoMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_LocalBufferSize.save(context, llvm::cast<llvm::MDNode>(getLocalBufferSizeNode(pNode)));        
    m_BarrierBufferSize.save(context, llvm::cast<llvm::MDNode>(getBarrierBufferSizeNode(pNode)));        
    m_KernelExecutionLength.save(context, llvm::cast<llvm::MDNode>(getKernelExecutionLengthNode(pNode)));        
    m_MaxWGDimensions.save(context, llvm::cast<llvm::MDNode>(getMaxWGDimensionsNode(pNode)));        
    m_KernelHasBarrier.save(context, llvm::cast<llvm::MDNode>(getKernelHasBarrierNode(pNode)));        
    m_KernelHasGlobalSync.save(context, llvm::cast<llvm::MDNode>(getKernelHasGlobalSyncNode(pNode)));        
    m_NoBarrierPath.save(context, llvm::cast<llvm::MDNode>(getNoBarrierPathNode(pNode)));        
    m_VectorizedKernel.save(context, llvm::cast<llvm::MDNode>(getVectorizedKernelNode(pNode)));        
    m_VectorizedWidth.save(context, llvm::cast<llvm::MDNode>(getVectorizedWidthNode(pNode)));        
    m_KernelWrapper.save(context, llvm::cast<llvm::MDNode>(getKernelWrapperNode(pNode)));        
    m_ScalarizedKernel.save(context, llvm::cast<llvm::MDNode>(getScalarizedKernelNode(pNode)));        
    m_BlockLiteralSize.save(context, llvm::cast<llvm::MDNode>(getBlockLiteralSizeNode(pNode)));        
    m_PrivateMemorySize.save(context, llvm::cast<llvm::MDNode>(getPrivateMemorySizeNode(pNode)));        
    m_VectorizationDimension.save(context, llvm::cast<llvm::MDNode>(getVectorizationDimensionNode(pNode)));        
    m_CanUniteWorkgroups.save(context, llvm::cast<llvm::MDNode>(getCanUniteWorkgroupsNode(pNode)));
}

llvm::Value* KernelInfoMetaData::getLocalBufferSizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "local_buffer_size") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getBarrierBufferSizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "barrier_buffer_size") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getKernelExecutionLengthNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_execution_length") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getMaxWGDimensionsNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "max_wg_dimensions") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getKernelHasBarrierNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_has_barrier") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getKernelHasGlobalSyncNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_has_global_sync") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getNoBarrierPathNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "no_barrier_path") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getVectorizedKernelNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "vectorized_kernel") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getVectorizedWidthNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "vectorized_width") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getKernelWrapperNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_wrapper") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getScalarizedKernelNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "scalarized_kernel") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getBlockLiteralSizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "block_literal_size") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getPrivateMemorySizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "private_memory_size") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getVectorizationDimensionNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "vectorization_dimension") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::Value* KernelInfoMetaData::getCanUniteWorkgroupsNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 0+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "can_unite_workgroups") )
        {
            return i.get();
        }
    }
    return NULL;
}

        
    

///
// Ctor - loads the KernelMetaData from the given metadata node
//
KernelMetaData::KernelMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_Function(getFunctionNode(pNode)),
        
    m_VecTypeHint(getVecTypeHintNode(pNode)),
        
    m_WorkGroupSizeHint(WorkGroupSizeMetaData::get(getWorkGroupSizeHintNode(pNode), true)),        
    m_ReqdWorkGroupSize(WorkGroupSizeMetaData::get(getReqdWorkGroupSizeNode(pNode), true)),        
    m_AddressSpace(getAddressSpaceNode(pNode), true),        
    m_ArgAccessQualifier(getArgAccessQualifierNode(pNode), true),        
    m_ArgTypes(getArgTypesNode(pNode), true),        
    m_ArgTypeQualifier(getArgTypeQualifierNode(pNode), true),        
    m_ArgName(getArgNameNode(pNode), true),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named KernelMetaData object
//
KernelMetaData::KernelMetaData():
    m_VecTypeHint("vec_type_hint"),        
    m_WorkGroupSizeHint(WorkGroupSizeMetaDataHandle::ObjectType::get("work_group_size_hint")),        
    m_ReqdWorkGroupSize(WorkGroupSizeMetaDataHandle::ObjectType::get("reqd_work_group_size")),        
    m_AddressSpace("kernel_arg_addr_space"),        
    m_ArgAccessQualifier("kernel_arg_access_qual"),        
    m_ArgTypes("kernel_arg_type"),        
    m_ArgTypeQualifier("kernel_arg_type_qual"),        
    m_ArgName("kernel_arg_name"),
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named KernelMetaData object
//
KernelMetaData::KernelMetaData(const char* name):
    _Mybase(name),
    m_VecTypeHint("vec_type_hint"),        
    m_WorkGroupSizeHint(WorkGroupSizeMetaDataHandle::ObjectType::get("work_group_size_hint")),        
    m_ReqdWorkGroupSize(WorkGroupSizeMetaDataHandle::ObjectType::get("reqd_work_group_size")),        
    m_AddressSpace("kernel_arg_addr_space"),        
    m_ArgAccessQualifier("kernel_arg_access_qual"),        
    m_ArgTypes("kernel_arg_type"),        
    m_ArgTypeQualifier("kernel_arg_type_qual"),        
    m_ArgName("kernel_arg_name"),
    m_pNode(NULL)
{}

///
// Returns true if any of the KernelMetaData`s members has changed
bool KernelMetaData::dirty() const
{
    if( m_Function.dirty() )
    {
        return true;
    }        
    if( m_VecTypeHint.dirty() )
    {
        return true;
    }        
    if( m_WorkGroupSizeHint.dirty() )
    {
        return true;
    }        
    if( m_ReqdWorkGroupSize.dirty() )
    {
        return true;
    }        
    if( m_AddressSpace.dirty() )
    {
        return true;
    }        
    if( m_ArgAccessQualifier.dirty() )
    {
        return true;
    }        
    if( m_ArgTypes.dirty() )
    {
        return true;
    }        
    if( m_ArgTypeQualifier.dirty() )
    {
        return true;
    }        
    if( m_ArgName.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the KernelMetaData instance
void KernelMetaData::discardChanges()
{
    m_Function.discardChanges();        
    m_VecTypeHint.discardChanges();        
    m_WorkGroupSizeHint.discardChanges();        
    m_ReqdWorkGroupSize.discardChanges();        
    m_AddressSpace.discardChanges();        
    m_ArgAccessQualifier.discardChanges();        
    m_ArgTypes.discardChanges();        
    m_ArgTypeQualifier.discardChanges();        
    m_ArgName.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* KernelMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_Function.generateNode(context));        
    args.push_back( m_VecTypeHint.generateNode(context));        
    args.push_back( m_WorkGroupSizeHint.generateNode(context));        
    args.push_back( m_ReqdWorkGroupSize.generateNode(context));        
    args.push_back( m_AddressSpace.generateNode(context));        
    args.push_back( m_ArgAccessQualifier.generateNode(context));        
    args.push_back( m_ArgTypes.generateNode(context));        
    args.push_back( m_ArgTypeQualifier.generateNode(context));        
    args.push_back( m_ArgName.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void KernelMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_Function.save(context, llvm::cast<llvm::MDNode>(getFunctionNode(pNode)));        
    m_VecTypeHint.save(context, llvm::cast<llvm::MDNode>(getVecTypeHintNode(pNode)));        
    m_WorkGroupSizeHint.save(context, llvm::cast<llvm::MDNode>(getWorkGroupSizeHintNode(pNode)));        
    m_ReqdWorkGroupSize.save(context, llvm::cast<llvm::MDNode>(getReqdWorkGroupSizeNode(pNode)));        
    m_AddressSpace.save(context, llvm::cast<llvm::MDNode>(getAddressSpaceNode(pNode)));        
    m_ArgAccessQualifier.save(context, llvm::cast<llvm::MDNode>(getArgAccessQualifierNode(pNode)));        
    m_ArgTypes.save(context, llvm::cast<llvm::MDNode>(getArgTypesNode(pNode)));        
    m_ArgTypeQualifier.save(context, llvm::cast<llvm::MDNode>(getArgTypeQualifierNode(pNode)));        
    m_ArgName.save(context, llvm::cast<llvm::MDNode>(getArgNameNode(pNode)));
}

llvm::Value* KernelMetaData::getFunctionNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}
    
llvm::Value* KernelMetaData::getVecTypeHintNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "vec_type_hint") )
        {
            return i.get();
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getWorkGroupSizeHintNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "work_group_size_hint") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getReqdWorkGroupSizeNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "reqd_work_group_size") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getAddressSpaceNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_arg_addr_space") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getArgAccessQualifierNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_arg_access_qual") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getArgTypesNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_arg_type") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getArgTypeQualifierNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_arg_type_qual") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}
    
llvm::MDNode* KernelMetaData::getArgNameNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    for(NodeIterator i = NodeIterator(pParentNode, 1+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "kernel_arg_name") )
        {
            return llvm::dyn_cast<llvm::MDNode>(i.get());
        }
    }
    return NULL;
}

        
        
        
        
    

///
// Ctor - loads the WorkGroupSizeMetaData from the given metadata node
//
WorkGroupSizeMetaData::WorkGroupSizeMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_XDim(getXDimNode(pNode)),
        
    m_YDim(getYDimNode(pNode)),
        
    m_ZDim(getZDimNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named WorkGroupSizeMetaData object
//
WorkGroupSizeMetaData::WorkGroupSizeMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named WorkGroupSizeMetaData object
//
WorkGroupSizeMetaData::WorkGroupSizeMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the WorkGroupSizeMetaData`s members has changed
bool WorkGroupSizeMetaData::dirty() const
{
    if( m_XDim.dirty() )
    {
        return true;
    }        
    if( m_YDim.dirty() )
    {
        return true;
    }        
    if( m_ZDim.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the WorkGroupSizeMetaData instance
void WorkGroupSizeMetaData::discardChanges()
{
    m_XDim.discardChanges();        
    m_YDim.discardChanges();        
    m_ZDim.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* WorkGroupSizeMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_XDim.generateNode(context));        
    args.push_back( m_YDim.generateNode(context));        
    args.push_back( m_ZDim.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void WorkGroupSizeMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_XDim.save(context, llvm::cast<llvm::MDNode>(getXDimNode(pNode)));        
    m_YDim.save(context, llvm::cast<llvm::MDNode>(getYDimNode(pNode)));        
    m_ZDim.save(context, llvm::cast<llvm::MDNode>(getZDimNode(pNode)));
}

llvm::Value* WorkGroupSizeMetaData::getXDimNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}
    
llvm::Value* WorkGroupSizeMetaData::getYDimNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(1 + offset);
}
    
llvm::Value* WorkGroupSizeMetaData::getZDimNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(2 + offset);
}

        
        
    

///
// Ctor - loads the StrCMetaData from the given metadata node
//
StrCMetaData::StrCMetaData(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    m_str(getstrNode(pNode)),
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named StrCMetaData object
//
StrCMetaData::StrCMetaData():
    
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named StrCMetaData object
//
StrCMetaData::StrCMetaData(const char* name):
    _Mybase(name),
    
    m_pNode(NULL)
{}

///
// Returns true if any of the StrCMetaData`s members has changed
bool StrCMetaData::dirty() const
{
    if( m_str.dirty() )
    {
        return true;
    }
    return false;
}

///
// Discards the changes done to the StrCMetaData instance
void StrCMetaData::discardChanges()
{
    m_str.discardChanges();
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* StrCMetaData::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    args.push_back( m_str.generateNode(context));

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void StrCMetaData::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
{
    assert( pNode && "The target node should be valid pointer");

    // we assume that underlying metadata node has not changed under our foot
    if( pNode == m_pNode && !dirty() )
    {
        return;
    }
    // check that we could save the new information to the given node without regenerating it
    if( !compatibleWith(pNode) )
    {
        pNode->replaceAllUsesWith(generateNode(context));
        return;
    }

    m_str.save(context, llvm::cast<llvm::MDNode>(getstrNode(pNode)));
}

llvm::Value* StrCMetaData::getstrNode( const llvm::MDNode* pParentNode) const
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
    return pParentNode->getOperand(0 + offset);
}


///
// dtor
MetaDataUtils::~MetaDataUtils()
{
    assert(!dirty() && "There were changes in the metadata hierarchy. Either save or discardChanges should be called");
}

bool isNamedNode(const llvm::Value* pNode, const char* name)
{
    const llvm::MDNode* pMDNode = llvm::dyn_cast<llvm::MDNode>(pNode);

    if( !pMDNode )
    {
        return false;
    }

    if( pMDNode->getNumOperands() < 1 )
    {
        return false;
    }

    const llvm::MDString* pIdNode = llvm::dyn_cast<const llvm::MDString>(pMDNode->getOperand(0));
    if( !pIdNode )
    {
        return false;
    }

    llvm::StringRef id = pIdNode->getString();
    if( id.compare(name) )
    {
        return false;
    }
    return true;
}



}
