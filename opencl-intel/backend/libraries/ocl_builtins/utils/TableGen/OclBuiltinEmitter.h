// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

//===----------------------------------------------------------------------===//
//
// This tablegen backend is responsible for emitting OCL builtin
// implementations.
//
//===----------------------------------------------------------------------===//

#ifndef OCLBUILTIN_EMITTER_H
#define OCLBUILTIN_EMITTER_H

#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"

#include <map>
#include <vector>
#include <string>

namespace llvm {

class OclBuiltin;
class OclBuiltinDB;

/// OclType
class OclType {
public:
  explicit OclType(const OclBuiltinDB&, const Record*);
  virtual ~OclType() {};

  virtual const std::string& getGenType(const std::string&) const { return m_Name; }

  std::string getCPattern() const;

  std::string getCMask() const;

  std::string getCVecLength() const;

  std::string getLoSuffix() const;

  std::string getHiSuffix() const;

  std::string getCBitWidth() const;

  std::string getExpandLoCPattern() const;

  std::string getExpandHiCPattern() const;

  std::string getExpandLoCPatternPtr() const;

  std::string getExpandHiCPatternPtr() const;

  std::string getSGBlockOpSuffix() const;

  std::string getCType(const OclBuiltin*, bool = false) const;

  std::string getNativeCType(const OclBuiltin*) const;

  const std::string& getBaseCType() const { return m_BaseCType; }

  const std::string& getName() const { return m_Name; }

  const std::string& getSuffix() const { return m_Suffix; }

  const std::string& getVTypeSuffix() const { return m_VTypeSuffix; }

  const std::string& getSVMLSuffix() const { return m_SVMLSuffix; }

  const std::string& getSVMLDSuffix() const { return m_SVMLDSuffix; }

  const std::string& getSVMLFSuffix() const { return m_SVMLFSuffix; }

  int getVecLength() const { return m_VecLength; }

  bool isPointer() const { return m_IsPtr; }

  bool isNative() const { return m_Native; }

  void setNative(bool native) { m_Native = native; }

protected:
  const OclBuiltinDB& m_DB;
  const Record* m_Record;
  std::string m_Name;
  std::string m_CType;
  std::string m_BaseCType;
  int m_VecLength;
  int m_BitWidth;
  std::string m_Suffix;
  std::string m_VTypeSuffix;
  std::string m_SVMLSuffix;
  std::string m_SVMLDSuffix;
  std::string m_SVMLFSuffix;
  bool m_IsPtr;
  bool m_Native;
};

/// OclGenType
class OclGenType : public OclType {
public:
  explicit OclGenType(const OclBuiltinDB&, const Record*);

  virtual const std::string& getGenType(const std::string&) const;

protected:
  std::map<std::string, std::string> m_GenMap;
};

/// OclBuiltinAttr
class OclBuiltinAttr {
public:
  explicit OclBuiltinAttr(const Record*);

  const std::string& getCAttr() const { return m_CAttribute; }

  static OclBuiltinAttr CreateInilineAttribute();

  bool operator == (const OclBuiltinAttr&)const;
private:
  explicit OclBuiltinAttr(const std::string&);
protected:
  std::string m_CAttribute;
};

/// OclBuiltin
class OclBuiltin {
public:
  explicit OclBuiltin(const OclBuiltinDB&, const Record*);
  virtual ~OclBuiltin();

  std::string getReturnSym(const std::string&, const std::string&) const;

  std::string getArgumentSym(unsigned, const std::string&, const std::string&) const;

  std::string getReturnCType(const std::string&) const;

  std::string getReturnBaseCType(const std::string&) const;

  std::string getReturnCName(const std::string&) const;

  size_t      getReturnVectorLength(const std::string&)const;

  size_t      getNumArguments() const;

  std::string getArgumentCType(unsigned, const std::string&) const;

  std::string getPtrArgumentCType(unsigned, const std::string&) const;

  std::string getArgumentBaseCType(unsigned i, const std::string& TyName) const;

  std::string getArgumentCVecType(unsigned, const std::string&, int) const;

  std::string getArgumentCNoASType(unsigned, const std::string&) const;

  std::string getArgumentCGenType(unsigned, const std::string&, const std::string&) const;

  std::string getArgumentSGBlockOpSuffix(unsigned, const std::string&) const;

  std::string getReturnCGenType(const std::string& Generator, const std::string& TyName) const;

  std::string getArgumentCName(unsigned, const std::string&) const;

  std::string getCFunc(const std::string&) const;

  std::string getNativeReturnCType(const std::string&) const;

  std::string getNativeArgumentCType(unsigned, const std::string&) const;

  std::string getNativeArgumentCVecLen(unsigned, const std::string&) const;

  std::string getNativeCFunc(const std::string&) const;

  std::string getExpandLoCFunc(const std::string&) const;

  std::string getExpandHiCFunc(const std::string&) const;

  std::string getCProto(const std::string&, bool isDecl = false) const;

  //indicates whether the underlying builtin is really an svml function
  bool isSvml()const;

  bool isOverlodable()const;
  // BUGBUG: isBrokenNameMangling() is a temporary w/around for name mangling in-compat with SPIR (CQ CSSD100017714)
  bool isBrokenNameMangling()const {return strstr(m_Name.c_str(),"work_group_") != NULL;}

  typedef std::vector<const OclType*>::const_iterator const_type_iterator;

  inline const_type_iterator  type_begin() const { return m_Types.begin(); }
  inline const_type_iterator    type_end() const { return m_Types.end(); }
  inline size_t                typeCount() const { return m_Types.size(); }

  bool isValidType(const std::string&) const;

  const std::string& getName() const { return m_Name; }

  bool isDeclOnly() const { return m_IsDeclOnly; }

  bool needForwardDecl() const { return m_NeedForwardDecl; }

  bool needPrefix() const { return m_NeedPrefix; }

  bool needToExclude() const { return m_NeedToExclude; }

  const std::string& getAS() const { return m_AS; }

  bool hasConst() const { return m_HasConst; }

  bool hasVolatile() const { return m_HasVolatile; }

  void addAttribute(const OclBuiltinAttr&);
  
  void removeAttribute(const OclBuiltinAttr&);

  bool shouldGenerate() const; 

protected:
  const OclBuiltinDB& m_DB;
  const Record* m_Record;
  std::string m_Name;
  std::string m_CFunc;
  bool m_IsDeclOnly;
  bool m_NeedForwardDecl;
  bool m_NeedToExclude;
  bool m_NeedPrefix;
  std::string m_AS;
  bool m_HasConst;
  bool m_HasVolatile;
  std::vector<const OclType*> m_Types;
  std::vector<const OclBuiltinAttr*> m_Attrs;
  std::vector<std::pair<const OclType*, std::string> > m_Outputs;
  std::vector<std::pair<const OclType*, std::string> > m_Inputs;
};

/// OclBuiltinImpl
class OclBuiltinImpl {
public:
  explicit OclBuiltinImpl(const OclBuiltinDB&, const Record*);
  virtual ~OclBuiltinImpl();

  std::string getCImpl(const std::string&) const;

  void appendImpl(const Record*);

  const OclBuiltin* getOclBuiltin() const { return m_Proto; }

protected:
  const OclBuiltinDB& m_DB;
  const OclBuiltin* m_Proto;


  struct Impl {
    const Record* m_Record;
    std::vector<const OclType*> m_Types;
    std::map<std::string, std::string> m_customMacro;
    std::string m_Code;
    bool m_IsDeclOnly;
  };
  std::vector<Impl*> m_Impls;
};

/// OclBuiltinDB
class OclBuiltinDB {
public:
  explicit OclBuiltinDB(RecordKeeper&);
  virtual ~OclBuiltinDB();

  std::string rewritePattern(const OclBuiltin*, const OclType*, const std::string&,
                             std::map<std::string, std::string> const&) const;

  std::string getNextNativeType(const std::string&) const;

  std::string getVecType(const std::string&, int len) const;

  std::string getExpandLoType(const std::string&) const;

  std::string getExpandHiType(const std::string&) const;

  const OclType* getOclType(const std::string&) const;

  const OclBuiltin* getOclBuiltin(const std::string&) const;

  const OclBuiltinImpl* getOclBuiltinImpl(const OclBuiltin*) const;

  typedef std::map<std::string, OclBuiltin*>::const_iterator const_proto_iterator;

  inline const_proto_iterator proto_begin() const { return m_ProtoMap.begin(); }
  inline const_proto_iterator   proto_end() const { return m_ProtoMap.end(); }
  inline size_t protoCount() const { return m_ProtoMap.size(); };

  const RecordKeeper& getRecords() const { return m_Records; }

  const Record* getRecord() const { return m_Record; }

  const std::string& getTarget() const { return m_Target; }

  const std::string& getProlog() const { return m_Prolog; }

  const std::string& getEpilog() const { return m_Epilog; }

  std::string getSVMLRounding(const std::string&) const;

protected:
  RecordKeeper& m_Records;
  const Record* m_Record;
  std::string m_Target;
  std::string m_Prolog;
  std::string m_Epilog;
  std::map<std::string, OclType*> m_TypeMap;
  std::map<std::string, OclBuiltin*> m_ProtoMap;
  std::map<const OclBuiltin*, OclBuiltinImpl*> m_ImplMap;
};

/// OclBuiltinEmitter
class OclBuiltinEmitter {
public:
  explicit OclBuiltinEmitter(RecordKeeper&);

  void run(raw_ostream&);

protected:
  RecordKeeper& m_Records;
  OclBuiltinDB m_DB;
};

} // End namespace llvm

#endif // OCLBUILTIN_EMITTER_H
