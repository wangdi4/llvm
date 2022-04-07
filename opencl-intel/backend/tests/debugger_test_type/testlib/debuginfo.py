import re
import struct

from debugservermessages_pb2 import *


#
# Public API
#

class DebugInfoError(Exception): pass


def set_sizeof_size_t(s):
    """ Set the size of size_t, used for size computations.
    """
    global _sizeof_size_t
    _sizeof_size_t = s


def var_info_type_as_string(varinfo):
    """ Given a VarInfo message, analyze its type descriptor and return the
        type as a string.
    """
    return _descriptor_as_string(varinfo.type_descriptor)


def var_info_type_size(varinfo):
    """ Given a VarInfo message, compute the size (in bytes) of the type it
        represents.
    """
    return _descriptor_type_size(varinfo.type_descriptor)


def var_info_memory_range(varinfo):
    """ Given a VarInfo message, return the memory range (start_addr, end_addr)
        this variable occupies in the target's memory.
    """
    start_addr = varinfo.address
    varsize = var_info_type_size(varinfo)
    assert varsize > 0 # unknown types aren't valid here
    return start_addr, start_addr + varsize - 1


def var_info_value(varinfo, databuf):
    """ Given a VarInfo message and the data buffer read from the target's
        memory, compute the value of the variable and return it as a string.
    """
    return _descriptor_value(varinfo.type_descriptor, databuf)


#
# Private APIs and data
#

# defaults to None, so set_sizeof_size_t must be called.
_sizeof_size_t = None


#
# Functions for string representations of types
#

def _descriptor_as_string(descr):
    """ Build the string type name encoded in the given descriptor.
    """
    if descr.tag == VarTypeDescriptor.BASIC:
        return descr.type_basic.name
    elif descr.tag == VarTypeDescriptor.TYPEDEF:
        return descr.type_typedef.name
    elif descr.tag == VarTypeDescriptor.VECTOR:
        return descr.type_vector.name
    elif descr.tag == VarTypeDescriptor.POINTER:
        return _descriptor_pointer_as_string(descr.type_pointer)
    elif descr.tag == VarTypeDescriptor.ARRAY:
        return _descriptor_array_as_string(descr.type_array)
    elif descr.tag == VarTypeDescriptor.STRUCT:
        return _descriptor_struct_as_string(descr.type_struct)
    elif descr.tag == VarTypeDescriptor.ENUM:
        return _descriptor_enum_as_string(descr.type_enum)
    else:
        return '<unknown>'


def _descriptor_basic_as_string(descr):
    """ String representation of a VarTypeBasic
    """
    if descr.tag == VarTypeBasic.BOOLEAN:
        return 'bool'
    elif descr.tag == VarTypeBasic.FLOAT:
        if descr.size_nbits == 64:
            return 'double'
        else:
            return 'float'
    elif descr.tag == VarTypeBasic.SIGNED_CHAR:
        return 'char'
    elif descr.tag == VarTypeBasic.UNSIGNED_CHAR:
        return 'unsigned char'
    elif descr.tag in (VarTypeBasic.SIGNED, VarTypeBasic.UNSIGNED):
        s = 'unsigned ' if descr.tag == VarTypeBasic.UNSIGNED else ''
        return s + {'16': 'short',
                    '32': 'int',
                    '64': 'long'}.get(str(descr.size_nbits), '<unknown>')
    else:
        return '<unknown>'


def _descriptor_struct_as_string(descr):
    """ String representation of a VarTypeStruct
    """
    s = 'struct '
    s += (descr.name if len(descr.name) > 0 else '<unnamed>')
    return s


def _descriptor_enum_as_string(descr):
    """ String representation of a VarTypeEnum
    """
    s = 'enum '
    s += (descr.name if len(descr.name) > 0 else '<unnamed>')
    return s


def _descriptor_pointer_as_string(descr):
    """ String representation of a VarTypePointer
    """
    pointee_str = _descriptor_as_string(descr.pointee)
    return pointee_str + '*'


def _descriptor_array_as_string(descr):
    """ String representation of a VarTypeArray
    """
    element_str = _descriptor_as_string(descr.element)
    return element_str + ''.join(('[%s]' % dim) for dim in descr.dimensions)


#
# Functions for computing the size of types
#

def _descriptor_type_size(descr):
    """ Compute the size of the type represented by the given descriptor.
    """
    if descr.tag == VarTypeDescriptor.BASIC:
        assert descr.type_basic.size_nbits % 8 == 0
        return descr.type_basic.size_nbits / 8
    elif descr.tag == VarTypeDescriptor.TYPEDEF:
        return _descriptor_type_size(descr.type_typedef.original_type)
    elif descr.tag == VarTypeDescriptor.VECTOR:
        return _descriptor_vector_size(descr.type_vector)
    elif descr.tag == VarTypeDescriptor.POINTER:
        return _sizeof_size_t
    elif descr.tag == VarTypeDescriptor.ARRAY:
        return _descriptor_array_size(descr.type_array)
    elif descr.tag == VarTypeDescriptor.STRUCT:
        return _descriptor_struct_size(descr.type_struct)
    #~ elif descr.tag == VarTypeDescriptor.ENUM:
        #~ return _descriptor_enum_as_string(descr.type_enum)
    else:
        return 0


def _descriptor_array_size(descr):
    """ Compute the size of the type represented by a VarTypeArray
    """
    total_size = _descriptor_type_size(descr.element)
    for dim in descr.dimensions:
        total_size *= dim
    return total_size


def _descriptor_vector_size(descr):
    """ Compute the size of the type represented by a VarTypeVector
    """
    base_typename, length = _parse_vector_name(descr.name)
    return _vector_base_types[base_typename] * length


def _descriptor_struct_size(descr):
    """ Compute the size of the type represented by VarTypeStruct
    """
    total_size = 0
    for member in descr.members:
        total_size += member.size_nbits // 8
    return total_size

#
# Functions for computing variable values
#

def _descriptor_value(descr, databuf):
    """ Compute the value of the variable represented by the given descriptor
        and stored in databuf.
    """
    if descr.tag == VarTypeDescriptor.BASIC:
        return _descriptor_basic_value(descr.type_basic, databuf)
    elif descr.tag == VarTypeDescriptor.TYPEDEF:
        return _descriptor_value(descr.type_typedef.original_type, databuf)
    elif descr.tag == VarTypeDescriptor.VECTOR:
        return _descriptor_vector_value(descr.type_vector, databuf)
    elif descr.tag == VarTypeDescriptor.POINTER:
        if _sizeof_size_t == 4:
            return _scalar_value('unsigned int', databuf)
        else:
            return _scalar_value('unsigned long', databuf)
    elif descr.tag == VarTypeDescriptor.ARRAY:
        return _descriptor_array_value(descr.type_array, databuf)
    elif descr.tag == VarTypeDescriptor.STRUCT:
        return _descriptor_struct_value(descr.type_struct, databuf)
    #~ elif descr.tag == VarTypeDescriptor.ENUM:
        #~ return _descriptor_enum_as_string(descr.type_enum)
    else:
        return '<unknown>'


def _descriptor_basic_value(descr, databuf):
    """ Compute the value of a basic variable.
    """
    typename = _descriptor_basic_as_string(descr)
    return _scalar_value(typename, databuf)


def _descriptor_vector_value(descr, databuf):
    """ Compute the value of a vector variable. The vector value is a ','-joined
        string of the values of its elements.
    """
    base_typename, length = _parse_vector_name(descr.name)
    elem_size = _vector_base_types[base_typename]

    vals = []
    for i in range(length):
        databuf_part = databuf[elem_size * i : elem_size * (i + 1)]
        vals.append(_scalar_value(base_typename, databuf_part))
    return ','.join(vals)


def _descriptor_array_value(descr, databuf):
    """ Compute the value of an array variable. The array value is a '|'-joined
        string of the values of its elements, enclosed in '[]'.
        Supports multi-dimensional arrays - the returned value is represented
        with nested []s.
    """
    from operator import mul
    elem_size = _descriptor_type_size(descr.element)
    elem_values = []
    total_num_elems = reduce(mul, descr.dimensions)
    for i in range(total_num_elems):
        elem_data = databuf[elem_size * i : elem_size * (i + 1)]
        elem_values.append(_descriptor_value(descr.element, elem_data))
    str = ''
    for i in range(total_num_elems):
        amount_of_open_parentheses = 0
        amount_of_close_parentheses  = 0
        current_dim_multiply = 1
        # current_dim_multiply stands for the size of the multiply of all
        # previous and current dimensions
        #
        for dim in reversed(descr.dimensions):
            current_dim_multiply *= dim
            if (i % current_dim_multiply == 0):
                amount_of_open_parentheses += 1
            if ((i + 1) % current_dim_multiply == 0):
                amount_of_close_parentheses += 1
        str += (amount_of_open_parentheses * '[' +
                (amount_of_open_parentheses == 0) * '|' +
                elem_values[i] +
                amount_of_close_parentheses * ']')
    return str


def _descriptor_struct_value(descr, databuf):
    """ Compute the value of a struct variable. The struct value is a '|'-joined
        string of the values of its elements, enclosed in '{}'

        Note: somewhat simplistic, assuming packed structs without bit-fields.
    """
    vals = []
    for member in descr.members:
        member_offset = member.offset_nbits // 8
        databuf_part = databuf[member_offset : member_offset + member.size_nbits // 8]
        vals.append(_descriptor_value(member.type, databuf_part))
    return '{' + '|'.join(vals) + '}'


# Map scalar type names to struct formats.
# this is an initial mapping that's being later post-processed to add some more
# formats automatically
#
_scalar_format_map = {
    'bool':         '?',
    'char':         'b',
    'short':        'h',
    'int':          'i',
    'long':         'q',   # OCL longs are 64-bit
    'float':        'f',
    'double':       'd',
}

# For these types, add 'signed' and 'unsigned' versions
for name in ['char', 'short', 'int', 'long']:
    format = _scalar_format_map[name]
    _scalar_format_map['signed ' + name] = format
    _scalar_format_map['unsigned ' + name] = format.upper()
    _scalar_format_map['u' + name] = format.upper()


def _scalar_value(typename, databuf):
    """ Compute value of a scalar type, given its type name in a string (for
        example, 'float', 'unsigned int', etc.)
    """
    if typename not in _scalar_format_map:
        raise DebugInfoError('invalid typename "%s"' % typename)

    try:
        format = _scalar_format_map[typename]
        value = struct.unpack('<' + format, databuf)[0]
        return str(value).lower()
    except struct.error, err:
        raise DebugInfoError('%s [format = "%s", databuf size = %s]' % (
                err, '<' + format, len(databuf)))


#
# Functions and data for parsing vector names
#

_vector_base_types = dict(
    char=1, uchar=1,
    short=2, ushort=2,
    int=4, uint=4,
    long=8, ulong=8,
    float=4,
    double=8)

_vector_lengths = set([2, 3, 4, 8, 16])


def _parse_vector_name(name):
    """ Given a vector type name (i.e. "float3"), parse it to its constituents
        and return a (base type, length) pair (i.e. 'float', 3)

        Raise DebugInfoError for an invalid vector name
    """
    mo = re.match('([a-zA-Z]+)(\d+)', name)
    if mo:
        base, length = mo.group(1), int(mo.group(2))
        if base in _vector_base_types and length in _vector_lengths:
            return base, length
    raise DebugInfoError('invalid vector type name %s' % name)
