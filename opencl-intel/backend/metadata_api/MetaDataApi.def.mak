## custom filter that capitalize the given text
<%!
    def title(text):
        return text.title()
%>
## generates the class name for the given typename
<%def name='class_name(typename)' filter='trim'>
    ${typename}MetaData
</%def>
## generate the member name for the given struct element
<%def name='member_name(element)' filter='trim'>
    m_${element['name']}
</%def>
## generates the cpp type of the list item for the given typename (type that is used when the given typename is used as a type of a list item )
<%def name='list_item_type(meta_typename)' filter='trim'>
    <% meta_type = schema[meta_typename] %>
    %if meta_type['is_container'] == False:
        ${meta_type['type']}
    %elif meta_type['container_type'] == 'struct':
        ${class_name(meta_typename)}Handle
    %elif meta_type['container_type'] == 'list':
        !!!Error!!! Cant detect the native type for the given meta typename ${meta_typename}
    %endif
</%def>
## generate the cpp typename for the element in the struct
<%def name='member_type(element)' filter='trim'>
    <% meta_type = schema[element['metatype']] %>
    %if meta_type['is_container'] == False:
        %if element['location'] == 'named':
            NamedMetaDataValue<${meta_type['type']}>
        %else:
            MetaDataValue<${meta_type['type']}>
        %endif
    %elif meta_type['container_type'] == 'list':
        <% item_meta_typename = meta_type['item_type'] %>
            MetaDataList<${list_item_type(item_meta_typename)}>
    %elif meta_type['container_type'] == 'struct':
            ${class_name(element['metatype'])}Handle
    %endif
</%def>
## generate the typedef name for the element type in the struct
<%def name='member_typedef(element)' filter='trim'>
    <% meta_type = schema[element['metatype']] %>
    %if meta_type['is_container'] == False:
    ${element['name']}Type
    %elif meta_type['container_type'] == 'list':
    ${element['name']}List
    %elif meta_type['container_type'] == 'map':
    ${element['name']}Map
    %endif
</%def>
## generate the cpp typename for the element in the root struct or list
<%def name='root_member_type(element)' filter='trim'>
    <% meta_type = schema[element['metatype']] %>
    %if meta_type['is_container'] == False:
        /* ERROR!!! Non container types cant be used as a top level meta data nodes */
    %elif meta_type['container_type'] == 'list':
        NamedMDNodeList<${list_item_type(meta_type['item_type'])}>
    %elif meta_type['container_type'] == 'map':
        NamedMetaDataMap<${list_item_type(meta_type['key_type'])}, ${list_item_type(meta_type['item_type'])}>
    %elif meta_type['container_type'] == 'struct':
        /* ERROR!!! Top level metadata structs are not supported */
    %endif
</%def>
## calculates and output the first non-positional index for the given struct
<%def name='first_named_index(parent)' filter='trim'>
    <% max = -1 %>
    %for element in parent['elements']:
        %if element['location'] == 'positional':
            %if max < int(element['index']):
                <% max = int(element['index']) %>
            %endif
        %endif
    %endfor
    <% max = max + 1 %>
    ${max}
</%def>
