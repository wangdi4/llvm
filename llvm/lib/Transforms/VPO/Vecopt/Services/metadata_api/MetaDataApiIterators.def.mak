## iterates the top level structs and generates the caller body
<%def name='iterate_structs()' filter='trim'>
    %for typename in schema:
        <% type=schema[typename] %>
        %if typename != 'root' and type['is_container'] == True and type['container_type'] == 'struct':
    ${caller.body(typename=typename)}
        %endif
    %endfor
</%def>
## iterates the given struct elements and generates the caller body
<%def name='iterate_struct_elements(parent)' filter='trim'>
    %for element in parent['elements']:
    ${caller.body(element=element)}\
    %endfor
</%def>