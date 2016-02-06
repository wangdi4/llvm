/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Refcount.h"
#include <string>
#include <vector>

#ifndef __PARAMETER_TYPE_H__
#define __PARAMETER_TYPE_H__

//The Type class hierarchy models the different types in OCL.

namespace reflection {

  enum TypePrimitiveEnum {
    PRIMITIVE_FIRST,
    PRIMITIVE_BOOL = PRIMITIVE_FIRST,
    PRIMITIVE_UCHAR,
    PRIMITIVE_CHAR,
    PRIMITIVE_USHORT,
    PRIMITIVE_SHORT,
    PRIMITIVE_UINT,
    PRIMITIVE_INT,
    PRIMITIVE_ULONG,
    PRIMITIVE_LONG,
    PRIMITIVE_HALF,
    PRIMITIVE_FLOAT,
    PRIMITIVE_DOUBLE,
    PRIMITIVE_VOID,
    PRIMITIVE_VAR_ARG,
    PRIMITIVE_STRUCT_FIRST,
    PRIMITIVE_STRUCT_LAST = PRIMITIVE_STRUCT_FIRST+1,
    PRIMITIVE_LAST = PRIMITIVE_STRUCT_LAST,
    PRIMITIVE_NONE,
    //keep this at the end
    PRIMITIVE_NUM = PRIMITIVE_NONE
  };

  enum TypeEnum {
    TYPE_ID_PRIMITIVE,
    TYPE_ID_POINTER,
    TYPE_ID_VECTOR,
    TYPE_ID_BLOCK,
    TYPE_ID_STRUCTURE
  };

  enum TypeAttributeEnum {
    ATTR_RESTRICT,
    ATTR_VOLATILE,
    ATTR_CONST,
    ATTR_NONE,
    ATTR_NUM = ATTR_NONE
  };

  //Forward declaration for abstract structure
  struct ParamType;
  typedef intel::RefCount<ParamType> RefParamType;
  typedef std::vector<ParamType*> DuplicatedTypeList;

  //Forward declaration for abstract structure
  struct TypeVisitor;

  struct ParamType {
    ///@brief Constructor
    ///@param TypeEnum type id
    ParamType(TypeEnum typeId) : m_typeId(typeId) {}

    ///@brief Destructor
    virtual ~ParamType() {}

    /// Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    virtual void accept(TypeVisitor*) const = 0;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    virtual std::string toString() const = 0;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    virtual bool equals(const ParamType*) const = 0;

    /// Common Base-Class Methods ///

    ///@brief returns type id of underlying type.
    ///@return type id
    TypeEnum getTypeId() const {
      return m_typeId;
    }

  private:
    // @brief Default Constructor
    ParamType();

  protected:
    ///an enumeration to identify the type id of this instance
    TypeEnum m_typeId;
  };


  struct PrimitiveType : public ParamType {
    ///an enumeration to identify the type id of this class
    const static TypeEnum enumTy;

    ///@brief Constructor
    ///@param TypePrimitiveEnum primitive id
    PrimitiveType(TypePrimitiveEnum);

    /// Implementation of Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    void accept(TypeVisitor*) const;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    std::string toString() const;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    bool equals(const ParamType*) const;

    /// Non-Common Methods ///

    ///@brief returns the primitive enumeration of the type.
    ///@return primitive type
    TypePrimitiveEnum getPrimitive() const {
      return m_primitive;
    }

  protected:
    ///an enumeration to identify the primitive type
    TypePrimitiveEnum m_primitive;
  };

  struct PointerType: public ParamType {
    ///an enumeration to identify the type id of this class
    const static TypeEnum enumTy;

    ///@brief Constructor
    ///@param RefParamType the type of pointee (that the pointer points at).
    PointerType(const RefParamType type);

    /// Implementation of Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    void accept(TypeVisitor*) const;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    std::string toString() const;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    bool equals(const ParamType*) const;

    /// Non-Common Methods ///

    ///@brief returns the type the pointer is pointing at.
    ///@return pointee type
    const RefParamType& getPointee() const {
      return m_pType;
    }

    ///@brief appends an attribute to the attribute list of the type.
    ///@param TypeAttributeEnum attribute id
    void addAttribute(TypeAttributeEnum attr) {
      m_attributes.push_back(attr);
    }

    ///@brief returns the attribute list.
    ///@return attribute list
    const std::vector<TypeAttributeEnum>& getAttributes() const {
      return m_attributes;
    }

  private:
    ///the type this pointer is pointing at
    RefParamType m_pType;
    ///attributes attached to this type
    std::vector<TypeAttributeEnum> m_attributes;
  };

  struct VectorType : public ParamType {
    ///an enumeration to identify the type id of this class
    const static TypeEnum enumTy;

    ///@brief Constructor
    ///@param RefParamType the type of each scalar element in the vector.
    ///@param int the length of the vector
    VectorType(const RefParamType type, int len);

    /// Implementation of Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    void accept(TypeVisitor*) const;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    std::string toString() const;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    bool equals(const ParamType*) const;

    /// Non-Common Methods ///

    ///@brief returns the type the vector is packing.
    ///@return scalar type
    const RefParamType& getScalarType() const {
      return m_pType;
    }

    ///@brief returns the length of the vector type.
    ///@return vector type length
    int getLength() const {
      return m_len;
    }

  private:
    ///the scalar type of this vector type
    RefParamType m_pType;
    ///the length of the vector
    int m_len;
  };

  struct BlockType : public ParamType {
    ///an enumeration to identify the type id of this class
    const static TypeEnum enumTy;

    ///@brief Constructor
    BlockType();

    /// Implementation of Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    void accept(TypeVisitor*) const;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    std::string toString() const;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    bool equals(const ParamType*) const;

    /// Non-Common Methods ///

    ///@brief returns the number of parameters of the block.
    ///@return parameters count
    unsigned int getNumOfParams() const {
      return (unsigned int)m_params.size();
    }

    ///@brief returns the type of parameter "index" of the block.
    // @param index the sequential number of the queried parameter 
    ///@return parameter type
    const RefParamType& getParam(unsigned int index) const {
      assert(m_params.size() > index && "index is OOB");
      return m_params[index];
    }

    ///@brief set the type of parameter "index" of the block.
    // @param index the sequential number of the queried parameter 
    // @param type the parameter type
    void setParam(unsigned int index, RefParamType type) {
      if(index < getNumOfParams()) {
        m_params[index] = type;
      }
      else if (index == getNumOfParams()) {
        m_params.push_back(type);
      }
      else {
        assert(false && "index is OOB");
      }
    }

  protected:
    ///an enumeration to identify the primitive type
    std::vector<RefParamType> m_params;
  };

  struct UserDefinedType : public ParamType {
    ///an enumeration to identify the type id of this class
    const static TypeEnum enumTy;

    ///@brief Constructor
    UserDefinedType(const std::string&);

    /// Implementation of Abstract Methods ///

    ///@brief visitor service method. (see TypeVisitor for more details).
    ///       When overridden in subclasses, preform a 'double dispatch' to the
    ///       appropriate visit method in the given visitor.
    ///@param TypeVisitor type visitor
    void accept(TypeVisitor*) const;

    ///@brief returns a string representation of the underlying type.
    ///@return type as string
    std::string toString() const;

    ///@brief returns true if given param type is equal to this type.
    ///@param ParamType given param type
    ///@return true if given param type is equal to this type and false otherwise
    bool equals(const ParamType*) const;

  protected:
    ///the name of the user defined type
    std::string m_name;
  };


  ///@brief TypeVisitor can be overridden so an object of static type Type* will
  ///       dispatch the correct visit method according to its dynamic type.
  struct TypeVisitor{
    virtual void visit(const PrimitiveType*)   = 0;
    virtual void visit(const VectorType*)      = 0;
    virtual void visit(const PointerType*)     = 0;
    virtual void visit(const BlockType*)       = 0;
    virtual void visit(const UserDefinedType*) = 0;
  };

  ///@brief template dynamic cast function for ParamType derived classes
  ///@param ParamType given param type
  ///@return required casting type if given param type is an instance if
  //         that type, NULL otherwise.
  template <typename T>
  T* dyn_cast(ParamType* pType) {
    assert(pType && "dyn_cast does not support casting of NULL");
    return (T::enumTy == pType->getTypeId()) ? (T*)pType : NULL;
  }

  ///@brief template dynamic cast function for ParamType derived classes
  ///       (the constant version)
  ///@param ParamType given param type
  ///@return required casting type if given param type is an instance if
  //         that type, NULL otherwise.
  template <typename T>
  const T* dyn_cast(const ParamType* pType) {
    assert(pType && "dyn_cast does not support casting of NULL");
    return (T::enumTy == pType->getTypeId()) ? (const T*)pType : NULL;
  }

} //namespace reflection
#endif //__PARAMETER_TYPE_H__
