/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
<%namespace name="utils" file="MetaDataApiIterators.def.mak"/>
<%namespace file="MetaDataApi.def.mak" import="*"/>

#ifndef METADATAAPI_H
#define METADATAAPI_H

#include "MetaDataApiUtils.h"

## Start generating the MetaDataAPI
namespace Intel
{
//typedefs and forward declarations
<%utils:iterate_structs args="typename">
class ${class_name(typename)};
typedef MetaObjectHandle<${class_name(typename)}> ${class_name(typename)}Handle; \
</%utils:iterate_structs>

<%utils:iterate_structs args="typename">
<%type=schema[typename]%>\
///
// Read/Write the ${typename} structure from/to LLVM metadata
//
class ${class_name(typename)}:public IMetaDataObject
{
public:
    typedef ${class_name(typename)} _Myt;
    typedef IMetaDataObject _Mybase;
    // typedefs for data member types
    <%utils:iterate_struct_elements parent="${type}" args="element">
    %if schema[element['metatype']]['is_container'] == False:
    typedef ${member_type(element)}::value_type ${member_typedef(element)};\
    %elif schema[element['metatype']]['container_type'] == 'list':
    typedef ${member_type(element)} ${member_typedef(element)};\
    %endif
    </%utils:iterate_struct_elements>

public:
    ///
    // Factory method - creates the ${class_name(typename)} from the given metadata node
    //
    static _Myt* get(const llvm::MDNode* pNode, bool hasId = false)
    {
        return new _Myt(pNode, hasId);
    }

    ///
    // Factory method - create the default empty ${class_name(typename)} object
    static _Myt* get()
    {
        return new _Myt();
    }

    ///
    // Factory method - create the default empty named ${class_name(typename)} object
    static _Myt* get(const char* name)
    {
        return new _Myt(name);
    }


    ///
    // Ctor - loads the ${class_name(typename)} from the given metadata node
    //
    ${class_name(typename)}(const llvm::MDNode* pNode, bool hasId);

    ///
    // Default Ctor - creates the empty, not named ${class_name(typename)} object
    //
    ${class_name(typename)}();

    ///
    // Ctor - creates the empty, named ${class_name(typename)} object
    //
    ${class_name(typename)}(const char* name);

    <%utils:iterate_struct_elements parent="${type}" args="element">
    <% meta_type = schema[element['metatype']] %>
    /// ${element['name']} related methods
    %if meta_type['is_container'] == False:
    ${member_typedef(element)} get${element['name']}()
    {
        return ${member_name(element)}.get();
    }

    void set${element['name']}( const ${member_typedef(element)}& val)
    {
        ${member_name(element)}.set(val);
    }

    bool is${element['name']}HasValue() const
    {
        return ${member_name(element)}.hasValue();
    }

    %elif meta_type['container_type'] == 'struct':
    ${member_type(element)} get${element['name']}()
    {
        return ${member_name(element)};
    }

    %elif meta_type['container_type'] == 'list':
    ${member_typedef(element)}::iterator begin_${element['name']}()
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::iterator end_${element['name']}()
    {
        return ${member_name(element)}.end();
    }

    ${member_typedef(element)}::const_iterator begin_${element['name']}() const
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::const_iterator end_${element['name']}() const
    {
        return ${member_name(element)}.end();
    }

    size_t size_${element['name']}()  const
    {
        return ${member_name(element)}.size();
    }

    bool empty_${element['name']}()  const
    {
        return ${member_name(element)}.empty();
    }

    bool is${element['name']}HasValue() const
    {
        return ${member_name(element)}.hasValue();
    }

    ${member_typedef(element)}::item_type get${element['name']}Item( size_t index )
    {
        return ${member_name(element)}.getItem(index);
    }

    void set${element['name']}Item( size_t index, const ${member_typedef(element)}::item_type& item  )
    {
        return ${member_name(element)}.setItem(index, item);
    }

    void add${element['name']}Item(const ${member_typedef(element)}::item_type& val)
    {
        ${member_name(element)}.push_back(val);
    }

    ${member_typedef(element)}::iterator erase${element['name']}Item(${member_typedef(element)}::iterator i)
    {
        return ${member_name(element)}.erase(i);
    }
    %endif
    </%utils:iterate_struct_elements>

    ///
    // Returns true if any of the ${class_name(typename)}`s members has changed
    bool dirty() const;

    ///
    // Returns true if the structure was loaded from the metadata or was changed
    bool hasValue() const
    {
        return NULL != m_pNode || dirty();
    }

    ///
    // Discards the changes done to the ${class_name(typename)} instance
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

    <%utils:iterate_struct_elements parent="${type}" args="element">
    %if schema[element['metatype']]['is_container'] == False:
    llvm::Value* get${element['name']}Node( const llvm::MDNode* pParentNode) const;
    %else:
    llvm::MDNode* get${element['name']}Node( const llvm::MDNode* pParentNode) const;
    %endif
    </%utils:iterate_struct_elements>

private:
    // data members
    <%utils:iterate_struct_elements parent="${type}" args="element">
    ${member_type(element)} ${member_name(element)};\
    </%utils:iterate_struct_elements>
    // parent node
    const llvm::MDNode* m_pNode;
};\
</%utils:iterate_structs>


class MetaDataUtils
{
public:
    // typedefs for the data members types
    <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
    %if schema[element['metatype']]['container_type'] == 'list':
    typedef ${root_member_type(element)} ${member_typedef(element)};\
    %elif schema[element['metatype']]['container_type'] == 'map':
    typedef ${root_member_type(element)} ${member_typedef(element)};\
    %endif
    </%utils:iterate_struct_elements>

public:
    MetaDataUtils(llvm::Module* pModule):
        <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
        ${member_name(element)}(pModule->getOrInsertNamedMetadata("${element['key']}")),\
        </%utils:iterate_struct_elements>
        m_pModule(pModule)
    {
    }

    ~MetaDataUtils();

    <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
    <% meta_type = schema[element['metatype']] %>
    /// ${element['name']} related methods
    %if meta_type['is_container'] == False:
    /// !!!Error!!!: top nodes can_t be of a simple type
    %elif meta_type['container_type'] == 'list':
    ${member_typedef(element)}::iterator begin_${element['name']}()
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::iterator end_${element['name']}()
    {
        return ${member_name(element)}.end();
    }

    ${member_typedef(element)}::const_iterator begin_${element['name']}() const
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::const_iterator end_${element['name']}() const
    {
        return ${member_name(element)}.end();
    }

    size_t size_${element['name']}()  const
    {
        return ${member_name(element)}.size();
    }

    bool empty_${element['name']}()  const
    {
        return ${member_name(element)}.empty();
    }

    bool is${element['name']}HasValue() const
    {
        return ${member_name(element)}.hasValue();
    }

    ${member_typedef(element)}::item_type get${element['name']}Item( size_t index )
    {
        return ${member_name(element)}.getItem(index);
    }

    void set${element['name']}Item( size_t index, const ${member_typedef(element)}::item_type& item  )
    {
        return ${member_name(element)}.setItem(index, item);
    }

    void add${element['name']}Item(const ${member_typedef(element)}::item_type& val)
    {
        ${member_name(element)}.push_back(val);
    }

    ${member_typedef(element)}::iterator erase${element['name']}Item(${member_typedef(element)}::iterator it)
    {
        return ${member_name(element)}.erase(it);
    }

    %elif meta_type['container_type'] == 'map':
    ${member_typedef(element)}::iterator begin_${element['name']}()
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::iterator end_${element['name']}()
    {
        return ${member_name(element)}.end();
    }

    ${member_typedef(element)}::const_iterator begin_${element['name']}() const
    {
        return ${member_name(element)}.begin();
    }

    ${member_typedef(element)}::const_iterator end_${element['name']}() const
    {
        return ${member_name(element)}.end();
    }

    size_t size_${element['name']}()  const
    {
        return ${member_name(element)}.size();
    }

    bool empty_${element['name']}()  const
    {
        return ${member_name(element)}.empty();
    }

    bool is${element['name']}HasValue() const
    {
        return ${member_name(element)}.hasValue();
    }

    ${member_typedef(element)}::item_type get${element['name']}Item( const ${member_typedef(element)}::key_type& index )
    {
        return ${member_name(element)}.getItem(index);
    }

    ${member_typedef(element)}::item_type getOrInsert${element['name']}Item( const ${member_typedef(element)}::key_type& index )
    {
        return ${member_name(element)}.getOrInsertItem(index);
    }

    void set${element['name']}Item( const ${member_typedef(element)}::key_type& index, const ${member_typedef(element)}::item_type& item  )
    {
        return ${member_name(element)}.setItem(index, item);
    }

    ${member_typedef(element)}::iterator find${element['name']}Item(const ${member_typedef(element)}::key_type& key)
    {
        return ${member_name(element)}.find(key);
    }

    void erase${element['name']}Item(${member_typedef(element)}::iterator it)
    {
        ${member_name(element)}.erase(it);
    }

    %elif meta_type['container_type'] == 'struct':
        assert(false && "Currently top level struct are not supported");
    %endif
    </%utils:iterate_struct_elements>

    bool dirty()
    {
        <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
        if( ${member_name(element)}.dirty() )
        {
            return true;
        }\
        </%utils:iterate_struct_elements>
        return false;
    }

    void save(llvm::LLVMContext& context)
    {
        <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
        if( ${member_name(element)}.dirty() )
        {
            llvm::NamedMDNode* pNode = m_pModule->getOrInsertNamedMetadata("${element['key']}");
            ${member_name(element)}.save(context, pNode);
        }\
        </%utils:iterate_struct_elements>
        discardChanges();
    }

    void discardChanges()
    {
        <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
        ${member_name(element)}.discardChanges();\
        </%utils:iterate_struct_elements>
    }

private:
    // data members
    <%utils:iterate_struct_elements parent="${schema['root']}" args="element">
    ${root_member_type(element)} ${member_name(element)};\
    </%utils:iterate_struct_elements>
    llvm::Module* m_pModule;
};


} //namespace
#endif
