#include "debuginfo_utils.h"
#include "debugservermessages.pb.h"
#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Metadata.h"
#include <iostream>


using namespace std;
using namespace llvm;
using namespace debugservermessages;


// Does the given type name describe a OpenCL vector?
//
static bool type_name_is_vector(string type_name)
{
    // Currently using a really stupid algorithm - if the type name ends with
    // a number, it describes a vector.
    //
    size_t len = type_name.size();
    return len > 0 && type_name.find_last_of("0123456789") == len - 1;
}


// Given a vector type name, split it to a <base type, length> pair.
//
static pair<string, size_t> split_vector_name(string type_name)
{
    size_t base_end_i = type_name.find_first_of("0123456789");

    if (base_end_i == string::npos)
        return make_pair("", 0);
    else {
        size_t len;
        stringstream ss(type_name.substr(base_end_i));
        ss >> len;
        assert(ss);

        return make_pair(type_name.substr(0, base_end_i), len);
    }
}


// Create a string representation for a vector of base type T and length N,
// with its value packed at address addr.
//
template<typename T> 
static inline std::string stringify_vector(size_t N, void* addr)
{
    string s = "";
    T* ptr = static_cast<T*>(addr);
    for (size_t i = 0; i < N; ++i) {
        s += stringify(*ptr);
        if (i < N - 1) s += ",";
        ptr++;
    }
    return s;
}


static string describe_vector_value(const DIType& di_type, void* addr, string type_name)
{
    pair<string, size_t> basetype_and_len = split_vector_name(type_name);
    string basetype = basetype_and_len.first;
    size_t vector_len = basetype_and_len.second;

    // Had we cared about performance here, this would do better in a lookup
    // table.
    //
    if (basetype == "char")
        return stringify_vector<int8_t>(vector_len, addr);
    else if (basetype == "uchar")
        return stringify_vector<uint8_t>(vector_len, addr);
    else if (basetype == "short")
        return stringify_vector<int16_t>(vector_len, addr);
    else if (basetype == "ushort")
        return stringify_vector<uint16_t>(vector_len, addr);
    else if (basetype == "int")
        return stringify_vector<int32_t>(vector_len, addr);
    else if (basetype == "uint")
        return stringify_vector<uint32_t>(vector_len, addr);
    else if (basetype == "long")
        return stringify_vector<int64_t>(vector_len, addr);
    else if (basetype == "ulong")
        return stringify_vector<uint64_t>(vector_len, addr);
    else if (basetype == "float")
        return stringify_vector<float>(vector_len, addr);
    else if (basetype == "double")
        return stringify_vector<double>(vector_len, addr);
    
    return "<unknown>";
}


static string DescribeArrayType(const DICompositeType& di_type)
{
    DIType derived_from_type = di_type.getTypeDerivedFrom();
    string type_str = DescribeVarType(derived_from_type);
    
    DIArray ranges_array = di_type.getTypeArray();
    for (unsigned i = 0; i < ranges_array.getNumElements(); ++i) {
        DIDescriptor elem = ranges_array.getElement(i);
        assert(elem.isSubrange());
        DISubrange subrange_elem(static_cast<MDNode*>(elem));
        uint64_t high_range = subrange_elem.getHi();

        type_str += "[" + stringify(high_range + 1) + "]";
    }

    return type_str;
}


string DescribeVarType(const DIType& di_type)
{
    if (di_type.isBasicType()) {
        DIBasicType basic_type_descriptor(static_cast<MDNode*>(di_type));
        return basic_type_descriptor.getName().str();
    }
    else if (di_type.isCompositeType()) {
        if (di_type.getTag() == dwarf::DW_TAG_array_type) {
            DICompositeType composite(static_cast<MDNode*>(di_type));
            return DescribeArrayType(composite);
        }
    }
    else if (di_type.isDerivedType()) {
        DIDerivedType derived_type_descriptor(static_cast<MDNode*>(di_type));
        DIType derived_from = derived_type_descriptor.getTypeDerivedFrom();

        if (di_type.getTag() == dwarf::DW_TAG_pointer_type) {
            string derived_from_str = DescribeVarType(derived_from);
            return derived_from_str + "*";
        }
        else if (di_type.getTag() == dwarf::DW_TAG_typedef) {
            return derived_type_descriptor.getName().str();
        }
    }
    return "<unknown>";
}


string DescribeVarValue(const DIType& di_type, void* addr, string type_name)
{
    if (di_type.isBasicType()) {
        DIBasicType basic_type_descriptor(static_cast<MDNode*>(di_type));

        switch (basic_type_descriptor.getEncoding()) {
            case dwarf::DW_ATE_boolean: 
                {
                    bool value = *(static_cast<bool*>(addr));
                    return stringify(value);
                }
            case dwarf::DW_ATE_signed_char:
                {
                    int8_t value = *(static_cast<int8_t*>(addr));
                    return stringify(value);
                }
            case dwarf::DW_ATE_unsigned_char:
                {
                    uint8_t value = *(static_cast<uint8_t*>(addr));
                    return stringify(value);
                }
            case dwarf::DW_ATE_float:
                {
                    double value;
                    if (basic_type_descriptor.getSizeInBits() == 32)
                        value = *(static_cast<float*>(addr));
                    else
                        value = *(static_cast<double*>(addr));
                    return stringify(value);
                }
            case dwarf::DW_ATE_signed:
                {
                    int64_t value = 0;
                    switch (basic_type_descriptor.getSizeInBits()) 
                    {
                        case 8:
                            value = *(static_cast<int8_t*>(addr));
                            break;
                        case 16:
                            value = *(static_cast<int16_t*>(addr));
                            break;
                        case 32:
                            value = *(static_cast<int32_t*>(addr));
                            break;
                        case 64:
                            value = *(static_cast<int64_t*>(addr));
                            break;
                        default:
                            assert(0 && "Unexpected bit size");
                    }
                    return stringify(value);
                }
            case dwarf::DW_ATE_unsigned:
                {
                    uint64_t value = 0;
                    switch (basic_type_descriptor.getSizeInBits()) 
                    {
                        case 8:
                            value = *(static_cast<uint8_t*>(addr));
                            break;
                        case 16:
                            value = *(static_cast<uint16_t*>(addr));
                            break;
                        case 32:
                            value = *(static_cast<uint32_t*>(addr));
                            break;
                        case 64:
                            value = *(static_cast<uint64_t*>(addr));
                            break;
                        default:
                            assert(0 && "Unexpected bit size");
                    }
                    return stringify(value);
                }
        }
    }
    else if (di_type.isDerivedType()) {
        DIDerivedType derived_type_descriptor(static_cast<MDNode*>(di_type));
        DIType derived_from = derived_type_descriptor.getTypeDerivedFrom();

        switch (di_type.getTag()) {
            case dwarf::DW_TAG_typedef:
                if (type_name_is_vector(type_name)) {
                    return describe_vector_value(di_type, addr, type_name);
                }
                else {
                    return DescribeVarValue(derived_from, addr);
                }
            case dwarf::DW_TAG_const_type:
            case dwarf::DW_TAG_volatile_type:
                return DescribeVarValue(derived_from, addr);
            case dwarf::DW_TAG_pointer_type:
                {
                    // addr is the address where the pointer is stored. Take
                    // the pointer from there and return its value.
                    //
                    uint64_t* ptr_addr = *(static_cast<uint64_t**>(addr));
                    return stringify(reinterpret_cast<uint64_t>(ptr_addr));
                }
            case dwarf::DW_TAG_array_type:
                {
                    // Arrays are represented by their actual address, so this
                    // is passed directly
                    //
                    uint64_t value = reinterpret_cast<uint64_t>(addr);
                    return stringify(value);
                }
            default:
                break;
        }
    }

    return "<unknown>";
}


static VarTypeDescriptor GenerateVarTypeBasic(const DIBasicType& di_type)
{
    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::BASIC);
    VarTypeBasic* descriptor_basic = descriptor.mutable_type_basic();

    // Decipher the basic type tag. 
    // Note: these are magic constants since LLVM's debug info headers don't
    // have them defined anywhere, so we're following the documentation.
    //
    VarTypeBasic::Tag basic_tag;
    switch (di_type.getEncoding()) {
        case 2:
            basic_tag = VarTypeBasic::BOOLEAN; break;
        case 4:
            basic_tag = VarTypeBasic::FLOAT; break;
        case 5:
            basic_tag = VarTypeBasic::SIGNED; break;
        case 6:
            basic_tag = VarTypeBasic::SIGNED_CHAR; break;
        case 7:
            basic_tag = VarTypeBasic::UNSIGNED; break;
        case 8:
            basic_tag = VarTypeBasic::UNSIGNED_CHAR; break;
        default:
            basic_tag = VarTypeBasic::UNKNOWN;
    }
    descriptor_basic->set_tag(basic_tag);
    descriptor_basic->set_size_nbits(di_type.getSizeInBits());
    descriptor_basic->set_name(di_type.getName());

    return descriptor;
}


static VarTypeDescriptor GenerateVarTypePointer(const DIType& di_pointee)
{
    VarTypeDescriptor pointee_descriptor = 
        GenerateVarTypeDescriptor(di_pointee);
    VarTypePointer pointer_descriptor;
    pointer_descriptor.mutable_pointee()->CopyFrom(pointee_descriptor);

    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::POINTER);
    descriptor.mutable_type_pointer()->CopyFrom(pointer_descriptor);
    return descriptor;
}


static VarTypeDescriptor GenerateVarTypeTypedef(
    const DIDerivedType& di_type, const DIType& di_derived_from)
{
    VarTypeDescriptor descriptor;

    if (!static_cast<MDNode*>(di_derived_from)) {
        // Vectors are currently represented in Clang as derived typedef types but
        // without the "derived from" information, only a name.
        // So if the derived_from information is missing, this is a vector.
        //
        VarTypeVector vector_descriptor;
        vector_descriptor.set_name(di_type.getName());
        descriptor.set_tag(VarTypeDescriptor::VECTOR);
        descriptor.mutable_type_vector()->CopyFrom(vector_descriptor);
    }
    else {
        // Otherwise, it's a normal typedef.
        //
        VarTypeDescriptor original = GenerateVarTypeDescriptor(di_derived_from);
        VarTypeTypedef typedef_descriptor;
        typedef_descriptor.set_name(di_type.getName());
        typedef_descriptor.mutable_original_type()->CopyFrom(original);
        descriptor.set_tag(VarTypeDescriptor::TYPEDEF);
        descriptor.mutable_type_typedef()->CopyFrom(typedef_descriptor);
    }

    return descriptor;
}


static VarTypeDescriptor GenerateUnknownVarType()
{
    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::UNKNOWN);
    return descriptor;
}


static VarTypeDescriptor GenerateVarTypeEnum(const DICompositeType& di_enum)
{
    VarTypeEnum enum_descriptor;
    enum_descriptor.set_name(di_enum.getName());

    DIArray di_enum_members = di_enum.getTypeArray();
    for (unsigned i = 0; i < di_enum_members.getNumElements(); ++i) {
        DIDescriptor di_member_i = di_enum_members.getElement(i);
        assert(di_member_i.isEnumerator());

        DIEnumerator di_enumerator_i(static_cast<MDNode*>(di_member_i));
        VarTypeEnum::EnumEntry* entry = enum_descriptor.add_entries();
        entry->set_name(di_enumerator_i.getName().str());
        entry->set_value(di_enumerator_i.getEnumValue());
    }

    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::ENUM);
    descriptor.mutable_type_enum()->CopyFrom(enum_descriptor);
    return descriptor;
}


static VarTypeDescriptor GenerateVarTypeArray(const DICompositeType& di_array)
{
    DIType di_element = di_array.getTypeDerivedFrom();

    VarTypeDescriptor element_descriptor = 
        GenerateVarTypeDescriptor(di_element);
    VarTypeArray array_descriptor;
    array_descriptor.mutable_element()->CopyFrom(element_descriptor);

    // For a multi-dimensional array there may be multiple ranges
    //
    DIArray di_ranges = di_array.getTypeArray();
    for (unsigned i = 0; i < di_ranges.getNumElements(); ++i) {
        DIDescriptor di_range_i = di_ranges.getElement(i);
        assert(di_range_i.isSubrange());
        DISubrange di_subrange(static_cast<MDNode*>(di_range_i));
        uint64_t high_range = di_subrange.getHi();
        array_descriptor.add_dimensions(high_range + 1);
    }

    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::ARRAY);
    descriptor.mutable_type_array()->CopyFrom(array_descriptor);
    return descriptor;
}


VarTypeDescriptor GenerateVarTypeStruct(const DICompositeType& di_struct)
{
    VarTypeStruct struct_descriptor;
    struct_descriptor.set_name(di_struct.getName());

    DIArray di_struct_members = di_struct.getTypeArray();
    for (unsigned i = 0; i < di_struct_members.getNumElements(); ++i) {
        // Each element in this type array is a DW_TAG_member, which is a 
        // derived type.
        //
        DIDescriptor di_member_i = di_struct_members.getElement(i);
        assert(di_member_i.isDerivedType() && 
            di_member_i.getTag() == dwarf::DW_TAG_member);
        DIDerivedType di_member_derived(static_cast<MDNode*>(di_member_i));

        VarTypeStruct::StructMember* member = struct_descriptor.add_members();
        member->set_name(di_member_derived.getName());
        member->set_align_nbits(di_member_derived.getAlignInBits());
        member->set_size_nbits(di_member_derived.getSizeInBits());
        member->set_offset_nbits(di_member_derived.getOffsetInBits());
        DIType member_type = di_member_derived.getTypeDerivedFrom();
        member->mutable_type()->CopyFrom(GenerateVarTypeDescriptor(member_type));
    }

    VarTypeDescriptor descriptor;
    descriptor.set_tag(VarTypeDescriptor::STRUCT);
    descriptor.mutable_type_struct()->CopyFrom(struct_descriptor);
    return descriptor;
}


VarTypeDescriptor GenerateVarTypeDescriptor(const DIType& di_type)
{
    if (di_type.isBasicType()) {
        DIBasicType di_basic_type(static_cast<MDNode*>(di_type));
        return GenerateVarTypeBasic(di_basic_type);
    }
    else if (di_type.isCompositeType()) {
        // isCompositeType() check must precede isDerivedType() because LLVM's
        // debug info considers composite types to also be derived.
        //
        DICompositeType di_composite(static_cast<MDNode*>(di_type));

        if (di_type.getTag() == dwarf::DW_TAG_array_type) {
            return GenerateVarTypeArray(di_composite);
        }
        else if (di_type.getTag() == dwarf::DW_TAG_enumeration_type) {
            return GenerateVarTypeEnum(di_composite);
        }
        else if (di_type.getTag() == dwarf::DW_TAG_structure_type) {
            return GenerateVarTypeStruct(di_composite);
        }
    }
    else if (di_type.isDerivedType()) {
        DIDerivedType di_derived(static_cast<MDNode*>(di_type));
        DIType di_derived_from = di_derived.getTypeDerivedFrom();

        if (di_type.getTag() == dwarf::DW_TAG_pointer_type) {
            return GenerateVarTypePointer(di_derived_from);
        }
        else if (   di_type.getTag() == dwarf::DW_TAG_volatile_type ||
                    di_type.getTag() == dwarf::DW_TAG_restrict_type ||
                    di_type.getTag() == dwarf::DW_TAG_const_type) {
            // Just ignore the modifier, recursively propagating the type 
            // hiding below.
            //
            return GenerateVarTypeDescriptor(di_derived_from);
        }
        else if (di_type.getTag() == dwarf::DW_TAG_typedef) {
            return GenerateVarTypeTypedef(di_derived, di_derived_from);
        }
    }

    // Return an "unknown" type by default
    //
    return GenerateUnknownVarType();
}

