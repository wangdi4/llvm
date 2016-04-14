/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
<%namespace name="utils" file="MetaDataApiIterators.def.mak"/>
<%namespace file="MetaDataApi.def.mak" import="*"/>
#include "MetaDataApi.h"

namespace Intel
{
<%utils:iterate_structs args="typename">
<%type=schema[typename]%>
///
// Ctor - loads the ${class_name(typename)} from the given metadata node
//
${class_name(typename)}::${class_name(typename)}(const llvm::MDNode* pNode, bool hasId):
    _Mybase(pNode, hasId),
    <%utils:iterate_struct_elements parent="${type}" args="element">
    %if schema[element['metatype']]['is_container'] == False:
    ${member_name(element)}(get${element['name']}Node(pNode)),
    %elif schema[element['metatype']]['container_type'] == 'struct':
        %if element['location'] == 'named':
    ${member_name(element)}(${class_name(element['metatype'])}::get(get${element['name']}Node(pNode), true)),\
        %else:
    ${member_name(element)}(${class_name(element['metatype'])}::get(get${element['name']}Node(pNode), false)),\
        %endif
    %else:
        %if element['location'] == 'named':
    ${member_name(element)}(get${element['name']}Node(pNode), true),\
        %else:
    ${member_name(element)}(get${element['name']}Node(pNode), false),\
        %endif
    %endif
    </%utils:iterate_struct_elements>
    m_pNode(pNode)
{}

///
// Default Ctor - creates the empty, not named ${class_name(typename)} object
//
${class_name(typename)}::${class_name(typename)}():
    <%utils:iterate_struct_elements parent="${type}" args="element">
    %if element['location'] == 'named':
        %if schema[element['metatype']]['is_container'] == False:
    ${member_name(element)}("${element['key']}"),\
        %elif schema[element['metatype']]['container_type'] == 'struct':
    ${member_name(element)}(${member_type(element)}::ObjectType::get("${element['key']}")),\
        %else:
    ${member_name(element)}("${element['key']}"),\
        %endif
    %else:
        %if schema[element['metatype']]['is_container'] == True and schema[element['metatype']]['container_type'] == 'struct':
    ${member_name(element)}(${class_name(element['metatype'])}::get()),\
        %endif
    %endif
    </%utils:iterate_struct_elements>
    m_pNode(NULL)
{}

///
// Ctor - creates the empty, named ${class_name(typename)} object
//
${class_name(typename)}::${class_name(typename)}(const char* name):
    _Mybase(name),
    <%utils:iterate_struct_elements parent="${type}" args="element">
    %if element['location'] == 'named':
        %if schema[element['metatype']]['is_container'] == False:
    ${member_name(element)}("${element['key']}"),\
        %elif schema[element['metatype']]['container_type'] == 'struct':
    ${member_name(element)}(${member_type(element)}::ObjectType::get("${element['key']}")),\
        %else:
    ${member_name(element)}("${element['key']}"),\
        %endif
    %else:
        %if schema[element['metatype']]['is_container'] == True and schema[element['metatype']]['container_type'] == 'struct':
    ${member_name(element)}(${class_name(element['metatype'])}::get()),\
        %endif
    %endif
    </%utils:iterate_struct_elements>
    m_pNode(NULL)
{}

///
// Returns true if any of the ${class_name(typename)}`s members has changed
bool ${class_name(typename)}::dirty() const
{
    <%utils:iterate_struct_elements parent="${type}" args="element">
    if( ${member_name(element)}.dirty() )
    {
        return true;
    }\
    </%utils:iterate_struct_elements>
    return false;
}

///
// Discards the changes done to the ${class_name(typename)} instance
void ${class_name(typename)}::discardChanges()
{
    <%utils:iterate_struct_elements parent="${type}" args="element">
    ${member_name(element)}.discardChanges();\
    </%utils:iterate_struct_elements>
}


///
// Generates the new MDNode hierarchy for the given structure
llvm::Value* ${class_name(typename)}::generateNode(llvm::LLVMContext& context) const
{
    llvm::SmallVector< llvm::Value*, 5> args;

    llvm::Value* pIDNode = _Mybase::generateNode(context);
    if( NULL != pIDNode )
    {
        args.push_back(pIDNode);
    }

    <%utils:iterate_struct_elements parent="${type}" args="element">
    args.push_back( ${member_name(element)}.generateNode(context));\
    </%utils:iterate_struct_elements>

    return llvm::MDNode::get(context, args);
}

///
// Saves the structure changes to the given MDNode
void ${class_name(typename)}::save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
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

    <%utils:iterate_struct_elements parent="${type}" args="element">
    ${member_name(element)}.save(context, llvm::cast<llvm::MDNode>(get${element['name']}Node(pNode)));\
    </%utils:iterate_struct_elements>
}

<%utils:iterate_struct_elements parent="${type}" args="element">
%if schema[element['metatype']]['is_container'] == False:
llvm::Value* ${class_name(typename)}::get${element['name']}Node( const llvm::MDNode* pParentNode) const
%else:
llvm::MDNode* ${class_name(typename)}::get${element['name']}Node( const llvm::MDNode* pParentNode) const
%endif
{
    if( !pParentNode )
    {
        return NULL;
    }

    unsigned int offset = _Mybase::getStartIndex();
%if element['location'] == 'positional':
    %if schema[element['metatype']]['is_container'] == False:
    return pParentNode->getOperand(${element['index']} + offset);
    %else:
    return llvm::dyn_cast<llvm::MDNode>(pParentNode->getOperand(${element['index']} + offset));
    %endif
%else:
    for(NodeIterator i = NodeIterator(pParentNode, ${first_named_index(type)}+offset), e = NodeIterator(pParentNode); i != e; ++i )
    {
        if( isNamedNode(i.get(), "${element['key']}") )
        {
    %if schema[element['metatype']]['is_container'] == False:
            return i.get();
    %else:
            return llvm::dyn_cast<llvm::MDNode>(i.get());
    %endif
        }
    }
    return NULL;
%endif
}
</%utils:iterate_struct_elements>
</%utils:iterate_structs>


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
