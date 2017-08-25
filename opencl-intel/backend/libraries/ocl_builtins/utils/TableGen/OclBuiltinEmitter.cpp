// vim:ts=2:sw=2:et:
//===- OclBuiltinEmitter.cpp - Generate OCL builtin impl --------*- C++ -*-===//
//
// Copyright (c) 2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//===----------------------------------------------------------------------===//
//
// This tablegen backend is responsible for emitting OCL builtin
// implementations.
//
// OCL builtins consists of 3 components: types, prototypes, and
// implementations.
//
// + Type is either an instant of type (OclType) or a type generator
//   (OclGenType). The latter is used to convert the input type to a output
//   type.
//
// + Prototype (OclBuiltin) is the prototype of a given OCL builtin. It
//   consists of a list of valid types and input/output dags for input/output
//   parameter types/names. Parameter type is either a type or type generator.
//   The list of valid types is called the primary type. If type generator is
//   used, this primary type will be used as the input type to derive the
//   output type.
//
// + Implementation (OclBuitlImpl) is the implementation of a OCL builtin on a
//   given list of types, i.e. a OCL builtin prototype may has more that one
//   implementations on different types.
//
//===----------------------------------------------------------------------===//

#include "OclBuiltinEmitter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/Record.h"
#include <algorithm>
#include <string>
#include <vector>
#include <memory>

using namespace llvm;

static cl::opt<bool>
GenOCLBuiltinPrototype("gen-ocl-proto", cl::Hidden,
                       cl::desc("Generate OCL builtin prototype."), cl::init(false));

static cl::opt<bool>
GenOCLBuiltinVerbose("gen-ocl-verbose", cl::Hidden,
                     cl::desc("Output verbose warning during OCL builtin generation."), cl::init(false));

static cl::opt<bool>
GenOCLBuiltinWerror("gen-ocl-werror", cl::Hidden,
                   cl::desc("Make all warnings into errors."), cl::init(false));

static cl::opt<std::string, false, cl::parser<std::string> >
  GenOCLBuiltinPrefix("gen-ocl-Prefix", cl::NotHidden,
  cl::desc("Add prefix for pre-defined subset of built-ins"), cl::init(""));

static cl::opt<bool>
  GenOCLBuiltinExclude("gen-ocl-Exclude", cl::NotHidden,
  cl::desc("Exclude pre-defined subset of built-ins"), cl::init(false));

#define GENOCL_WARNING(X)  do { } while (0)
/*
#define GENOCL_WARNING(X)                                                         \
  do { errs() << (GenOCLBuiltinWerror ? "ERROR: " : "WARNING: ") << X;            \
    if (GenOCLBuiltinWerror) report_fatal_error("Warning is reported as error."); \
  } while (0)
*/

/// OclType
OclType::OclType(const OclBuiltinDB& DB, const Record* R)
: m_DB(DB)
, m_Record(R)
, m_Name(R->getName())
, m_CType(R->getValueAsString("CTypeName"))
, m_BaseCType(R->getValueAsString("BaseCTypeName"))
, m_VecLength(R->getValueAsInt("VecLength"))
, m_BitWidth(R->getValueAsInt("BitWidth"))
, m_Suffix(R->getValueAsString("Suffix"))
, m_SVMLSuffix(R->getValueAsString("SVMLSuffix"))
, m_SVMLDSuffix(R->getValueAsString("SVMLDSuffix"))
, m_SVMLFSuffix(R->getValueAsString("SVMLFSuffix"))
, m_IsPtr(R->getValueAsBit("IsPtr"))
, m_Native(false)
{
  assert(R->isSubClassOf("OclType") && "Invalid OclType record.");
}

std::string
OclType::getCType(const OclBuiltin* OB, bool NoAS) const
{
  if (!m_IsPtr)
    return m_CType;

  std::string Ret;

  if (OB->hasConst())
    Ret += (Ret.empty() ? "" : " ") + std::string("const");
  if (OB->hasVolatile())
    Ret += (Ret.empty() ? "" : " ") + std::string("volatile");

  Ret += (Ret.empty() ? "" : " ") + (NoAS ? "" : OB->getAS());
  Ret += (Ret.empty() ? "" : " ") + m_CType + " *";

  return Ret;
}

std::string
OclType::getNativeCType(const OclBuiltin* OB) const
{
  return m_DB.getOclType(m_DB.getNextNativeType(m_Name))->getCType(OB);
}

std::string
OclType::getCPattern() const
{
  switch (m_VecLength) {
    case 1: return ".s0";
    case 2: return ".s01";
    case 3: return ".s012";
    case 4: return ".s0123";
    case 8: return ".s01234567";
    case 16: return "";
  }
  GENOCL_WARNING("Invalid vector length(" << m_VecLength << ") is found for type '" << m_Name << "' on rewrite pattern $Pattern.\n");
  return "__invalid_pattern__";
}

std::string
OclType::getCMask() const
{
  switch (m_VecLength) {
    case 1: return "0x01";
    case 2: return "0x03";
    case 3: return "0x07";
    case 4: return "0x0F";
    case 8: return "0xFF";
    case 16: return "0xFFFF";
  }
  GENOCL_WARNING("Invalid vector length(" << m_VecLength << ") is found for type '" << m_Name << "' on rewrite pattern $Mask.\n");
  return "__invalid_mask__";
}

std::string
OclType::getCVecLength() const
{
  switch (m_VecLength) {
    case 1: return "1";
    case 2: return "2";
    case 3: return "3";
    case 4: return "4";
    case 8: return "8";
    case 16: return "16";
  }
  GENOCL_WARNING("Invalid vector length(" << m_VecLength << ") is found for type '" << m_Name << "' on rewrite pattern $VecLength.\n");
  return "__invalid_vec_length__";
}

std::string
OclType::getLoSuffix() const
{
  switch (m_VecLength) {
    case 1:
    case 2: return "";
    case 3:
    case 4: return "2";
    case 8: return "4";
    case 16: return "8";
  }
  GENOCL_WARNING("Invalid vector length(" << m_VecLength << ") is found for type '" << m_Name << "' on rewrite pattern $LoSuffix.\n");
  return "__invalid_vec_length__";
}

std::string
OclType::getHiSuffix() const
{
  switch (m_VecLength) {
    case 1:
    case 2:
    case 3: return "";
    case 4: return "2";
    case 8: return "4";
    case 16: return "8";
  }
  GENOCL_WARNING("Invalid vector length(" << m_VecLength << ") is found for type '" << m_Name << "' on rewrite pattern $HiSuffix.\n");
  return "__invalid_vec_length__";
}

std::string
OclType::getCBitWidth() const
{
  switch (m_BitWidth) {
    case  8: return  "8";
    case 16: return "16";
    case 32: return "32";
    case 64: return "64";
  }
  GENOCL_WARNING("Invalid bit width(" << m_BitWidth << ") is found for type '" << m_Name << "' on rewrite pattern $BitWidth.\n");
  return "__invalid_bit_width__";
}

std::string
OclType::getExpandLoCPattern() const
{
  switch (m_VecLength) {
    case 3: return ".s01";
  }
  return ".lo";
}

std::string
OclType::getExpandHiCPattern() const
{
  switch (m_VecLength) {
    case 3: return ".s2";
  }
  return ".hi";
}

std::string
OclType::getExpandLoCPatternPtr() const
{
  switch (m_VecLength) {
  // According to specification.
  // "The suffixes .lo (or .even) and .hi (or .odd) for a 3-component vector type 
  // operate as if the 3-component vector type is a 4-component vector type 
  // with the value in the w component
  // here we use ->s01 for good look of code since it will be used together with $ExpandHiPatternPtr
    case 3: return "->s01";
  }
  return "->lo";
}

std::string
OclType::getExpandHiCPatternPtr() const
{
  switch (m_VecLength) {
  // We don't use here ->hi since we don't want garbage 
  // to get to s3 component of promoted type4 vector. 
  // Getting garbage ther may lead to performance degradations. Since in case of float we can get there NaNs, denormals, etc  
  case 3: return "->s2";
  }
  return "->hi";
}


/// OclGenType
OclGenType::OclGenType(const OclBuiltinDB& DB, const Record* R)
: OclType(DB, R)
{
  assert(R->isSubClassOf("OclGenType") && "Invalid OclGenType record.");

  ListInit* Tin = R->getValueAsListInit("Tin");
  ListInit* Tout = R->getValueAsListInit("Tout");

  assert(Tin && Tout && Tin->size() > 0 && Tin->size() == Tout->size() && "Invalid OclGenType record.");

  if (!(Tin && Tout && Tin->size() > 0 && Tin->size() == Tout->size())) {
    GENOCL_WARNING("Invalid gentype '" << m_Name << "' is found.\n");
    return;
  }

  for (ListInit::const_iterator I = Tin->begin(),
                                O = Tout->begin(),
                                E = Tin->end(); I != E; ++I, ++O) {
    const std::string& istr = dyn_cast<StringInit>(*I)->getValue();
    const std::string& ostr = dyn_cast<StringInit>(*O)->getValue();
    m_GenMap.insert(std::pair<std::string, std::string>(istr, ostr));
  }
}

const std::string&
OclGenType::getGenType(const std::string& in) const
{
  assert(m_GenMap.find(in) != m_GenMap.end() && "'gentype' not mapped.");
  return m_GenMap.find(in)->second;
}


/// OclBuiltinAttr
OclBuiltinAttr::OclBuiltinAttr(const Record* R)
: m_CAttribute(R->getValueAsString("Attr"))
{
  assert(R->isSubClassOf("OclBuiltinAttr") && "Invalid OclType record.");
}

const char*const INLINE = "inline __attribute__((always_inline))";

OclBuiltinAttr OclBuiltinAttr::CreateInilineAttribute(){
  return OclBuiltinAttr(INLINE);
}

OclBuiltinAttr::OclBuiltinAttr(const std::string& S):m_CAttribute(S){
}

bool OclBuiltinAttr::operator == (const OclBuiltinAttr& A)const{
  if(this == &A)
    return true;
  return m_CAttribute == A.m_CAttribute;
}

/// OclBuiltin
OclBuiltin::OclBuiltin(const OclBuiltinDB& DB, const Record* R)
: m_DB(DB)
, m_Record(R)
, m_Name(R->getNameInitAsString())
, m_CFunc(R->getValueAsString("Name"))
, m_IsDeclOnly(R->getValueAsBit("IsDeclOnly"))
, m_NeedForwardDecl(R->getValueAsBit("NeedForwardDecl"))
, m_NeedToExclude(R->getValueAsBit("Exclude"))
, m_NeedPrefix(R->getValueAsBit("Prefix"))
, m_HasConst(0)
, m_HasVolatile(0)
{
  // Types
  {
    std::vector<Record*> Tys = R->getValueAsListOfDefs("Types");
    assert(Tys.size() > 0 && "Invalid OclBuiltin record with empty type list.");

    std::vector<Record*> Exs = R->getValueAsListOfDefs("ExceptionTypes");
    std::map<const OclType*, int> ExMap;
    for (std::vector<Record*>::const_iterator I = Exs.begin(), E = Exs.end(); I != E; ++I)
      ExMap.insert(std::pair<const OclType*, int>(m_DB.getOclType((*I)->getName()), 1));

    for (std::vector<Record*>::const_iterator I = Tys.begin(), E = Tys.end(); I != E; ++I)
      if (ExMap.end() == ExMap.find(m_DB.getOclType((*I)->getName())))
        m_Types.push_back(m_DB.getOclType((*I)->getName()));

    std::vector<Record*> Ads = R->getValueAsListOfDefs("AdditionTypes");
    for (std::vector<Record*>::const_iterator I = Ads.begin(), E = Ads.end(); I != E; ++I)
      if (ExMap.end() == ExMap.find(m_DB.getOclType((*I)->getName())))
        m_Types.push_back(m_DB.getOclType((*I)->getName()));
  }

  // Outs
  {
    DagInit* Outs = R->getValueAsDag("Outs");
    assert(Outs && "Invalid OclBuiltin record without outs.");

    assert(dyn_cast<DefInit>(Outs->getOperator()) && 
      dyn_cast<DefInit>(Outs->getOperator())->getDef()->getName() == "outs" && 
      "Invalid OclBuiltin record with invalid outputs.");

    for (unsigned i = 0, e = Outs->getNumArgs(); i != e; ++i) {
      const OclType* ArgTy = m_DB.getOclType(dyn_cast<DefInit>(Outs->getArg(i))->getDef()->getName());
      const std::string& ArgName = Outs->getArgNameStr(i);
      m_Outputs.push_back(std::pair<const OclType*, std::string>(ArgTy, ArgName));
    }
  }

  // Ins
  {
    DagInit* Ins = R->getValueAsDag("Ins");
    assert(Ins && "Invalid OclBuiltin record without ins.");

    assert(dyn_cast<DefInit>(Ins->getOperator()) && 
      dyn_cast<DefInit>(Ins->getOperator())->getDef()->getName() == "ins" && 
      "Invalid OclBuiltin record with invalid inputs.");
    for (unsigned i = 0, e = Ins->getNumArgs(); i != e; ++i) {
      const DefInit* DI = dyn_cast<DefInit>(Ins->getArg(i));
      assert(DI && "Invalid built-in 'ins' argument.");

      const OclType* ArgTy = m_DB.getOclType(DI->getDef()->getName());
      const std::string& ArgName = Ins->getArgNameStr(i);
      m_Inputs.push_back(std::pair<const OclType*, std::string>(ArgTy, ArgName));
    }
  }

  // Attrs
  {
    std::vector<Record*> As = R->getValueAsListOfDefs("Attrs");

    std::vector<Record*> Exs = R->getValueAsListOfDefs("ExceptionAttrs");
    std::map<const Record*, int> ExMap;
    for (std::vector<Record*>::const_iterator I = Exs.begin(), E = Exs.end(); I != E; ++I)
      ExMap.insert(std::pair<const Record*, int>(*I, 1));

    for (std::vector<Record*>::const_iterator I = As.begin(), E = As.end(); I != E; ++I) {
      if (ExMap.end() != ExMap.find(*I))
        continue;
      OclBuiltinAttr* Attr = new OclBuiltinAttr(*I);
      m_Attrs.push_back(Attr);
    }

    std::vector<Record*> Ads = R->getValueAsListOfDefs("AdditionAttrs");
    for (std::vector<Record*>::const_iterator I = Ads.begin(), E = Ads.end(); I != E; ++I) {
      if (ExMap.end() != ExMap.find(*I))
        continue;
      OclBuiltinAttr* Attr = new OclBuiltinAttr(*I);
      m_Attrs.push_back(Attr);
    }
  }

  // Optional ASQualifier
  if (R->getValue("ASQualifier"))
    m_AS = R->getValueAsString("ASQualifier");
  // Optional HasConst
  if (R->getValue("HasConst"))
    m_HasConst = R->getValueAsBit("HasConst");
  // Optional HasVolatile
  if (R->getValue("HasVolatile"))
    m_HasVolatile = R->getValueAsBit("HasVolatile");
}


OclBuiltin::~OclBuiltin()
{
  for (std::vector<const OclBuiltinAttr*>::const_iterator I = m_Attrs.begin(), E = m_Attrs.end(); I != E; ++I)
    delete *I;
}

std::string
OclBuiltin::getReturnSym(const std::string& Generator, const std::string& TyName) const
{
  std::string ret;

  if (m_Outputs.size() == 0)
    return "void";

  assert(m_Outputs.size() == 1 && "Unsupported OclBuiltin with more than 1 outputs.");

  const OclType* g = m_DB.getOclType(Generator);
  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  return g->getGenType(GT);
}

std::string
OclBuiltin::getArgumentSym(unsigned i, const std::string& Generator, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const OclType* g = m_DB.getOclType(Generator);
  const std::string& GT =  m_Inputs[i].first->getGenType(TyName);
  return g->getGenType(GT);
}

std::string
OclBuiltin::getReturnCType(const std::string& TyName) const
{
  std::string ret;

  if (m_Outputs.size() == 0)
    return "void";

  assert(m_Outputs.size() == 1 && "Unsupported OclBuiltin with more than 1 outputs.");

  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getReturnBaseCType(const std::string& TyName) const
{
  std::string ret;

  if (m_Outputs.size() == 0)
    return "void";

  assert(m_Outputs.size() == 1 && "Unsupported OclBuiltin with more than 1 outputs.");

  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  assert(T && "Invalid type found.");

  return T->getBaseCType();
}

std::string
OclBuiltin::getReturnCName(const std::string&) const
{
  assert(m_Outputs.size() == 1 && "Illegal return variable name for OclBuiltin without a single output.");

  return m_Outputs[0].second;
}

size_t
OclBuiltin::getReturnVectorLength(const std::string& TyName)const{
  if (m_Outputs.size() == 0)
    return 0;
  assert(m_Outputs.size() == 1 && "Unsupported OclBuiltin with more than 1 outputs.");
  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  return std::atoi(T->getCVecLength().c_str());
}

size_t
OclBuiltin::getNumArguments() const{
  return m_Inputs.size();
}

std::string
OclBuiltin::getArgumentCType(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getPtrArgumentCType(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");
  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  std::string NoPtrGT = GT.substr(1);
  const OclType* T = m_DB.getOclType(NoPtrGT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getArgumentBaseCType(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");
  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  assert(T && "Invalid type found.");
  return T->getBaseCType();
}

std::string
OclBuiltin::getArgumentCVecType(unsigned i, const std::string& TyName, int len) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const std::string& VT = m_DB.getVecType(GT, len);
  const OclType* T = m_DB.getOclType(VT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getArgumentCNoASType(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const OclType* T = m_DB.getOclType(GT);
  assert(T && "Invalid type found.");

  return T->getCType(this, true);
}

std::string
OclBuiltin::getArgumentCGenType(unsigned i, const std::string& Generator, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const OclType* g = m_DB.getOclType(Generator);
  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const std::string& GT2 = g->getGenType(GT);
  const OclType* T = m_DB.getOclType(GT2);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getReturnCGenType(const std::string& Generator, const std::string& TyName) const
{
  const OclType* g = m_DB.getOclType(Generator);
  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  const std::string& GT2 = g->getGenType(GT);
  const OclType* T = m_DB.getOclType(GT2);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getArgumentCName(unsigned i, const std::string&) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  return m_Inputs[i].second;
}

std::string
OclBuiltin::getCFunc(const std::string& TyName) const
{
  static std::map<std::string, std::string> nullMacro;

  const OclType* Ty = m_DB.getOclType(TyName);
  return m_DB.rewritePattern(this, Ty, m_CFunc, nullMacro);
}

std::string
OclBuiltin::getNativeReturnCType(const std::string& TyName) const
{
  assert(m_Outputs.size() == 1 && "Illegal return native type for OclBuiltin without a single output.");

  const std::string& GT = m_Outputs[0].first->getGenType(TyName);
  const std::string& NT = m_DB.getNextNativeType(GT);
  const OclType* T = m_DB.getOclType(NT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getNativeArgumentCType(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const std::string& NT = m_DB.getNextNativeType(GT);
  const OclType* T = m_DB.getOclType(NT);
  assert(T && "Invalid type found.");

  return T->getCType(this);
}

std::string
OclBuiltin::getNativeArgumentCVecLen(unsigned i, const std::string& TyName) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  const std::string& GT = m_Inputs[i].first->getGenType(TyName);
  const std::string& NT = m_DB.getNextNativeType(GT);
  const OclType* T = m_DB.getOclType(NT);
  assert(T && "Invalid type found.");

  return T->getCVecLength();
}

std::string
OclBuiltin::getNativeCFunc(const std::string& TyName) const
{
  return getCFunc(m_DB.getNextNativeType(TyName));
}

std::string
OclBuiltin::getExpandLoCFunc(const std::string& TyName) const
{
  return getCFunc(m_DB.getExpandLoType(TyName));
}

std::string
OclBuiltin::getExpandHiCFunc(const std::string& TyName) const
{
  return getCFunc(m_DB.getExpandHiType(TyName));
}

std::string
OclBuiltin::getCProto(const std::string& TyName, bool isDecl) const
{
  for (std::vector<const OclType*>::const_iterator I = m_Types.begin(),
                                                   E = m_Types.end(); I != E; ++I) {
    if ((*I)->getName() != TyName)
      continue;

    std::string prototype;

    // Return
    prototype += getReturnCType(TyName);

    // Attrs
    for (std::vector<const OclBuiltinAttr*>::const_iterator I = m_Attrs.begin(), E = m_Attrs.end(); I != E; ++I) {
      prototype += " ";
      prototype += (*I)->getCAttr();
    }

    // Name
    prototype += " ";

    if(needPrefix()){
      prototype += GenOCLBuiltinPrefix.getValue();
    }
    prototype += getCFunc(TyName);

    // Arguments
    prototype += "(";
    std::string sep = "";
    for (unsigned i = 0, e = m_Inputs.size(); i != e; ++i) {
      prototype += sep;
      prototype += getArgumentCType(i, TyName);
      if (!isDecl) {
        // Tune the output for function definition to avoid space between '*'
        // and argument name.
        const std::string& GT = m_Inputs[i].first->getGenType(TyName);
        const OclType* T = m_DB.getOclType(GT);
        assert(T && "Invalid type found.");
        if (!T->isPointer())
          prototype += " ";
        prototype += getArgumentCName(i, TyName);
      }
      sep = ", ";
    }
    prototype += ")";
    if (isDecl)
      prototype += ";";

    return prototype;
  }

  return "";
}

bool OclBuiltin::isSvml()const{
  std::string svmlprefix("__ocl_svml");
  return (getName().substr(0, svmlprefix.length()) == svmlprefix);
}

bool OclBuiltin::isOverlodable()const{
  std::vector<const OclBuiltinAttr*>::const_iterator it = m_Attrs.begin(),
  e = m_Attrs.end();
  while (it != e){
    if ("__attribute__((overloadable))" == (*it)->getCAttr())
      return true;
    ++it;
  }
  return false;
}

bool
OclBuiltin::isValidType(const std::string& TyName) const
{
  for (std::vector<const OclType*>::const_iterator I = m_Types.begin(),
                                                   E = m_Types.end(); I != E; ++I)
    if ((*I)->getName() == TyName)
      return true;

  return false;
}

void OclBuiltin::addAttribute(const OclBuiltinAttr& A){
  std::vector<const OclBuiltinAttr*>::iterator it = m_Attrs.begin(),
    e = m_Attrs.end();
  while(it != e){
    const OclBuiltinAttr *LHS = *it++;
    if(*LHS == A)
      return;
  }
  m_Attrs.push_back(new OclBuiltinAttr(A));
}
  
void OclBuiltin::removeAttribute(const OclBuiltinAttr& A){
  std::vector<const OclBuiltinAttr*>::iterator it = m_Attrs.begin(),
    e = m_Attrs.end();
  while(it != e){
    const OclBuiltinAttr *LHS = *it;
    if(*LHS == A){
      m_Attrs.erase(it);
      delete const_cast<OclBuiltinAttr*>(LHS);
      return;
    }
    ++it;
  }
}

bool OclBuiltin::shouldGenerate() const {
  return !(GenOCLBuiltinExclude && needToExclude());
}

/// OclBuiltinImpl
OclBuiltinImpl::OclBuiltinImpl(const OclBuiltinDB& DB, const Record* R)
: m_DB(DB)
, m_Proto(DB.getOclBuiltin(R->getValueAsDef("Builtin")->getName()))
{
  appendImpl(R);
}

OclBuiltinImpl::~OclBuiltinImpl()
{
  for (std::vector<Impl*>::iterator I = m_Impls.begin(), E = m_Impls.end(); I != E; ++I)
    delete *I;
}

static std::string
ConvertCRLF2LF(const std::string& text)
{
  std::string ret;

  size_t cpos = 0, npos;
  do {
    npos = text.find("\r\n", cpos);

    if (npos != std::string::npos) {
      ret += text.substr(cpos, npos - cpos);
      ret += "\n"; // replace with LF
      npos += 2; // skip CRLF
    } else
      ret += text.substr(cpos);

    if (std::string::npos == npos)
      npos = text.size();
    cpos = npos;
  } while (cpos < text.size());

  return ret;
}

static std::string
RemoveCommonLeadingSpaces(const std::string& t)
{
  std::string text = ConvertCRLF2LF(t);

  // How the common leading space is calculated:
  // - if a line has only space and ends with a new line, it's not considered
  //   during leading space calculation.
  // - otherwise, its leading space is counted as the consecutive spaces (only
  //   ' ') from the beginning and the common leading space is the minimal one.

  size_t common_leading_spaces = 0;
  int lines_with_leading_spaces = 0;
  size_t cpos = 0, npos;
  do {
    npos = text.find("\n", cpos);

    size_t newline = 1;
    if (std::string::npos == npos) {
      npos = text.size();
      newline = 0;
    }

    size_t lws = 0;
    while (cpos < npos && ' ' == text[cpos]) {
      ++lws;
      ++cpos;
    }

    if (cpos < npos || !newline) {
      // Only consider line with non-space characters.
      if (lines_with_leading_spaces)
        common_leading_spaces = std::min(common_leading_spaces, lws);
      else
        common_leading_spaces = lws;
      lines_with_leading_spaces++;
    }

    cpos = npos + newline; // Try next line.
  } while (cpos < text.size());

  // Only eligible line is found or the common leading space is 0.
  if (0 == common_leading_spaces)
    return text;

  std::string ret;
  cpos = 0;
  do {
    npos = text.find("\n", cpos);

    size_t newline = 1;
    if (std::string::npos == npos) {
      npos = text.size();
      newline = 0;
    }

    if ((npos - cpos) > common_leading_spaces)
      ret += text.substr(cpos + common_leading_spaces, npos - cpos - common_leading_spaces);

    if (newline)
      ret += "\n";

    cpos = npos + newline;
  } while (cpos < text.size());

  return ret;
}

static std::string removePrefix(const std::string f){
  size_t findex = f.find('_');
  if ( std::string::npos != findex )
    return f.substr( findex+1, f.size()-(1+findex) );
  return f;
}

std::string
OclBuiltinImpl::getCImpl(const std::string& in) const
{
  const OclType* Ty = m_DB.getOclType(in);

  if (!Ty)
    return "";

  for (std::vector<Impl*>::const_iterator I = m_Impls.begin(), E = m_Impls.end(); I != E; ++I) {
    for (std::vector<const OclType*>::const_iterator J = (*I)->m_Types.begin(), E = (*I)->m_Types.end(); J != E; ++J) {
      if (Ty != (*J))
        continue;
      std::string ret;
      if (!(*I)->m_IsDeclOnly && m_Proto->shouldGenerate()) {
        ret += m_Proto->getCProto(in);
        ret += "\n{";
        ret += RemoveCommonLeadingSpaces(
            m_DB.rewritePattern(m_Proto, Ty, (*I)->m_Code, (*I)->m_customMacro));
        ret += "}\n";
      }
      return ret;
    }
  }

  return "";
}

void
OclBuiltinImpl::appendImpl(const Record* R)
{
  for (std::vector<Impl*>::iterator I = m_Impls.begin(), E = m_Impls.end(); I != E; ++I) {
    if ((*I)->m_Record != R)
      continue;
    // If found, put it at the end.
    Impl* impl = (*I);
    m_Impls.erase(I);
    m_Impls.push_back(impl);
    return;
  }

  std::auto_ptr<Impl> impl;
  // Types
  {
    std::vector<Record*> Tys;
    const RecordVal* RV = R->getValue("Types");
    if (VarInit* FI = dyn_cast<VarInit>(RV->getValue())) {
      const RecordVal* IV = m_DB.getRecord()->getValue(FI->getName());
      assert(dyn_cast<ListInit>(IV->getValue()) && "Invalid OclBuiltinImpl record.");
      ListInit* List = dyn_cast<ListInit>(IV->getValue());
      for (unsigned i = 0; i != List->size(); ++i) {
        DefInit* DI = dyn_cast<DefInit>(List->getElement(i));
        assert(DI && "Invalid OclBuiltinImpl record, list is not entirely DefInit.");
        Tys.push_back(DI->getDef());
      }
    } else {
      Tys = R->getValueAsListOfDefs("Types");
    }
    // Empty type list is a way to get back to a previous OclBuiltinImpl for all types.
    if(Tys.size() == 0) return;

    impl.reset(new Impl);
    for (std::vector<Record*>::const_iterator I = Tys.begin(), E = Tys.end(); I != E; ++I) {
      if (!m_Proto->isValidType((*I)->getName())) {
        if (GenOCLBuiltinVerbose)
          GENOCL_WARNING("'" << R->getName() << "' specifies invalid type '" << (*I)->getName() << "'.\n");
        continue;
      }
      impl->m_Types.push_back(m_DB.getOclType((*I)->getName()));
    }
  }

  impl->m_Record = R;
  // IsDeclOnly
  impl->m_IsDeclOnly = R->getValueAsBit("IsDeclOnly");
  // CustomMacro
  {
    ListInit* macroses = R->getValueAsListInit("CustomMacro");
    if(macroses)
    {
      for (ListInit::const_iterator iter = macroses->begin();
        iter != macroses->end(); ++iter) {
        PairInit* pair = dyn_cast<PairInit>(*iter);
        if(pair)
        {
          std::string const& keyword = dyn_cast<StringInit>(pair->getFirst())->getValue();
          std::string const& replacement = dyn_cast<StringInit>(pair->getSecond())->getValue();
          impl->m_customMacro[keyword] = replacement;
        } else {
          GENOCL_WARNING("'" << R->getName() << "' specifes invalid custom macro.\n");
        }
      }
    }
  }
  // Impl
  {
    const RecordVal* RV = R->getValue("Impl");
    if (VarInit* FI = dyn_cast<VarInit>(RV->getValue())) {
      const RecordVal* IV = m_DB.getRecord()->getValue(FI->getName());
      if (StringInit *SI = dyn_cast<StringInit>(IV->getValue()))
        impl->m_Code = SI->getValue();
      else if(CodeInit *CI = dyn_cast<CodeInit>(IV->getValue()))
        impl->m_Code = CI->getValue();
      else
        llvm_unreachable("Invalid OclBuiltinImpl record.");
    } else
      impl->m_Code = R->getValueAsString("Impl");
  }
  m_Impls.push_back(impl.release());
}


/// OclBuiltinDB
OclBuiltinDB::OclBuiltinDB(RecordKeeper& R)
: m_Records(R)
{
  // OclType
  {
    std::vector<Record*> Rs = m_Records.getAllDerivedDefinitions("OclType");
    for (std::vector<Record*>::const_iterator I = Rs.begin(),
                                              E = Rs.end(); I != E; ++I) {
      const Record* Rec = *I;
      OclType* OT = Rec->isSubClassOf("OclGenType")
                    ? new OclGenType(*this, Rec)
                    : new OclType(*this, Rec);
      m_TypeMap[OT->getName()] = OT;
    }
  }

  // OclBuiltin
  {
    std::vector<Record*> Rs = m_Records.getAllDerivedDefinitions("OclBuiltin");
    for (std::vector<Record*>::const_iterator I = Rs.begin(),
                                              E = Rs.end(); I != E; ++I) {
      const Record* Rec = *I;
      OclBuiltin* OB = new OclBuiltin(*this, Rec);
      m_ProtoMap[OB->getName()] = OB;
    }
  }

  // OclBuiltinImpl
  {
    const char*const GENERIC = "Generic";
    std::vector<Record*> Rs = m_Records.getAllDerivedDefinitions(GENERIC);
    const Record* Rec = m_Record = (Rs.size() > 0) ? Rs.front() :
      m_Records.getClass(GENERIC);
    
    // One and only one single instance of OclBuiltins is defined.
    assert(Rs.size() < 2 && "More than 1 Generic are defined!");

    assert(Rec && "Rec is nullptr");

    // Target
    m_Target = Rec->getValueAsString("Target");

    // Prolog & Epilog
    {
      m_Prolog = Rec->getValueAsString("Prolog");
      m_Epilog = Rec->getValueAsString("Epilog");
    }

    // NativeTypes
    {
      std::vector<Record*> Rs = Rec->getValueAsListOfDefs("NativeTypes");
      for (std::vector<Record*>::iterator I = Rs.begin(), E = Rs.end(); I != E; ++I) {
        assert(m_TypeMap.find((*I)->getName()) != m_TypeMap.end() && "NativeTypes refers to invalid types.");
        m_TypeMap.find((*I)->getName())->second->setNative(true);
      }
    }

    // Collect OCL builtin implementations.
    {
      RecordRecTy* OBI = RecordRecTy::get(R.getClass("OclBuiltinImpl"));

      const std::vector<RecordVal>& Values = Rec->getValues();
      for (size_t i = 0, e = Values.size(); i !=e ; ++i) {
        const RecordVal& RV = Values[i];
        Init* Def = RV.getValue();

        // UnsetInit could be converted to any type, skip it first.
        if (dyn_cast<UnsetInit>(Def))
          continue;

        // Not convertible, skip it as well.
        if (!Def->convertInitializerTo(OBI))
          continue;

        const Record* DefRec = dyn_cast<DefInit>(Def)->getDef();
        const OclBuiltin* proto = getOclBuiltin(DefRec->getValueAsDef("Builtin")->getName());

        std::map<const OclBuiltin*, OclBuiltinImpl*>::const_iterator II = m_ImplMap.find(proto);
        if (m_ImplMap.end() == II) {
          OclBuiltinImpl* OI = new OclBuiltinImpl(*this, DefRec);
          assert(proto == OI->getOclBuiltin() && "OclBuiltinImpl prototype mismatches.");
          m_ImplMap[OI->getOclBuiltin()] = OI;
        } else
          II->second->appendImpl(DefRec);
      }

      // Check its super classes.
      ArrayRef<std::pair<Record *, SMRange>> SCs = Rec->getSuperClasses();
      for (ArrayRef<std::pair<Record *, SMRange>>::reverse_iterator
             I = SCs.rbegin(), E = SCs.rend(); I != E; ++I) {
        const Record* Rec = I->first;
        const std::vector<RecordVal>& Values = Rec->getValues();
        for (size_t i = 0, e = Values.size(); i !=e ; ++i) {
          const RecordVal& RV = Values[i];
          Init* Def = RV.getValue();

          // UnsetInit could be converted to any type, skip it first.
          if (dyn_cast<UnsetInit>(Def))
            continue;

          // Not convertible, skip it as well.
          if (!Def->convertInitializerTo(OBI))
            continue;

          const Record* DefRec = dyn_cast<DefInit>(Def)->getDef();
          const OclBuiltin* proto = getOclBuiltin(DefRec->getValueAsDef("Builtin")->getName());

          std::map<const OclBuiltin*, OclBuiltinImpl*>::const_iterator II = m_ImplMap.find(proto);
          if (m_ImplMap.end() == II) {
            OclBuiltinImpl* OI = new OclBuiltinImpl(*this, DefRec);
            assert(proto == OI->getOclBuiltin() && "OclBuiltinImpl prototype mismatches.");
            m_ImplMap[OI->getOclBuiltin()] = OI;
          } else
            II->second->appendImpl(DefRec);
        }
      }
    }
  }
}

OclBuiltinDB::~OclBuiltinDB()
{
  // impls
  for (std::map<const OclBuiltin*, OclBuiltinImpl*>::const_iterator I = m_ImplMap.begin(), E = m_ImplMap.end(); I != E; ++I)
    delete I->second;
  // prototypes
  for (std::map<std::string, OclBuiltin*>::const_iterator I = m_ProtoMap.begin(), E = m_ProtoMap.end(); I != E; ++I)
    delete I->second;
  // types
  for (std::map<std::string, OclType*>::const_iterator I = m_TypeMap.begin(), E = m_TypeMap.end(); I != E; ++I)
    delete I->second;
}

std::string
OclBuiltinDB::rewritePattern(const OclBuiltin* OB, const OclType* OT, const std::string& text,
                             std::map<std::string, std::string> const& customMacro) const
{
  std::string ret;

  size_t cpos = 0;
  size_t dpos = text.find("$", cpos);
  while (dpos != std::string::npos) {
    ret += text.substr(cpos, dpos - cpos);

    cpos = dpos;
    do {
      ++dpos;
      if (dpos >= text.size())
        break;
      // '#' is concatenate mark - exit loop
      if (text[dpos] == '#')
          break;
      // Skip 'alphanum'. 
    } while (isalnum(text[dpos]) );

    std::string pat = text.substr(cpos, dpos - cpos);

    // replace $pat with real text
    std::string val;
    
    bool hardcodedMacro = true;
    std::map<std::string, std::string>::const_iterator iter = customMacro.find(pat);
    if (iter != customMacro.end())
    {
        if(iter->second.size() && iter->second[0] == '$')
        {
          // custom macro could be mapped to a hardcoded macro
          pat = iter->second;
        } else {
          val = iter->second;
          hardcodedMacro = false;
        }
    } 
    
    if(hardcodedMacro)
    {
        if ("$Target" == pat) {
          val = getTarget();
        } else if ("$rtn" == pat.substr(0, 4) || "$rtz" == pat.substr(0, 4) ||
                   "$up" == pat.substr(0, 3) || "$down" == pat.substr(0, 5)) {
          val = getSVMLRounding(pat);
        } else if ("$Suffix" == pat) {
          val = OT->getSuffix();
        } else if ("$LoSuffix" == pat) {
          val = OT->getLoSuffix();
        } else if ("$HiSuffix" == pat) {
          val = OT->getHiSuffix();
        } else if ("$SVMLSuffix" == pat) {
          val = OT->getSVMLSuffix();
        } else if ("$SVMLDSuffix" == pat) {
          val = OT->getSVMLDSuffix();
        } else if ("$SVMLFSuffix" == pat) {
          val = OT->getSVMLFSuffix();
        } else if ("$Pattern" == pat) {
          val = OT->getCPattern();
        } else if ("$Mask" == pat) {
          val = OT->getCMask();
        } else if ("$VecLength" == pat) {
          val = OT->getCVecLength();
        } else if ("$BitWidth" == pat) {
          val = OT->getCBitWidth();
        } else if ("$Func" == pat) {
          val = OB->getCFunc(OT->getName());
        } else if ("$RemovePrefixFunc" == pat) {
          val = removePrefix(OB->getCFunc(OT->getName()));
        } else if ("$ReturnSym" == pat.substr(0, 10) && pat.find("gentype") != std::string::npos) {
          val = OB->getReturnSym(pat.substr(10), OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.find("Sym") != std::string::npos && pat.find("gentype") != std::string::npos) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentSym(i, pat.substr(8), OT->getName());
        } else if ("$ReturnType" == pat) {
          val = OB->getReturnCType(OT->getName());
        } else if ("$ReturnBaseType" == pat) {
          val = OB->getReturnBaseCType(OT->getName());
        } else if ("$ReturnVarName" == pat) {
          val = OB->getReturnCName(OT->getName());
        } else if ("$Return" == pat.substr(0, 7) && pat.substr(7).find("gentype") != std::string::npos) {
          val = OB->getReturnCGenType(pat.substr(7), OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 9 && "Type" == pat.substr(5)) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentCType(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 10 && "Type" == pat.substr(6)) {
          unsigned i = (pat[4] - '0')*10 + (pat[5] - '0');
          val = OB->getArgumentCType(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 13 && "BaseType" == pat.substr(5)) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentBaseCType(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 12 && "VecType" == pat.substr(5)) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentCVecType(i, OT->getName(), OT->getVecLength());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 13 && "VecType" == pat.substr(6)) {
          unsigned i = (pat[4] - '0')*10 + (pat[5] - '0');
          val = OB->getArgumentCVecType(i, OT->getName(), OT->getVecLength());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 13 && "NoASType" == pat.substr(5)) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentCNoASType(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 12 && "VarName" == pat.substr(5)) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentCName(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 13 && "VarName" == pat.substr(6)) {
          unsigned i = (pat[4] - '0')*10 + (pat[5] - '0');
          val = OB->getArgumentCName(i, OT->getName());
        } else if ("$Arg" == pat.substr(0, 4) &&  pat.substr(5).find("gentype") != std::string::npos) {
          unsigned i = pat[4] - '0';
          val = OB->getArgumentCGenType(i, pat.substr(5), OT->getName());
        } else if ("$NativeFunc" == pat) {
          val = OB->getNativeCFunc(OT->getName());
        } else if ("$NativeReturnType" == pat) {
          val = OB->getNativeReturnCType(OT->getName());
        } else if ("$NativeArg" == pat.substr(0, 10) && pat.size() == 15 && "Type" == pat.substr(11)) {
          unsigned i = pat[10] - '0';
          val = OB->getNativeArgumentCType(i, OT->getName());
        } else if (("$PtrArg" == pat.substr(0, 7)) && ("Type" == pat.substr(8,12)) ) {
          unsigned i = pat[7] - '0';
          val = OB->getPtrArgumentCType(i, OT->getName());
        } else if ("$NativeArg" == pat.substr(0, 10) && pat.size() == 20 && "VecLength" == pat.substr(11)) {
          unsigned i = pat[10] - '0';
          val = OB->getNativeArgumentCVecLen(i, OT->getName());
        } else if ("$ExpandLoFunc" == pat) {
          val = OB->getExpandLoCFunc(OT->getName());
        } else if ("$ExpandHiFunc" == pat) {
          val = OB->getExpandHiCFunc(OT->getName());
        } else if ("$ExpandLoPattern" == pat) {
          val = OT->getExpandLoCPattern();
        } else if ("$ExpandHiPattern" == pat) {
          val = OT->getExpandHiCPattern();
        } else if ("$ExpandLoPatternPtr" == pat) {
          val = OT->getExpandLoCPatternPtr();
        } else if ("$ExpandHiPatternPtr" == pat) {
          val = OT->getExpandHiCPatternPtr();
        } else if ("$ExpandLoReturnType" == pat) {
            val = getOclType(getExpandLoType(OT->getName()))->getCType(OB);
        } else if ("$ExpandHiReturnType" == pat) {
            val = getOclType(getExpandHiType(OT->getName()))->getCType(OB);
        } else if ("$ExpandLoSuffix" == pat) {
            val = getOclType(getExpandLoType(OT->getName()))->getSuffix();
        } else if ("$ExpandHiSuffix" == pat) {
            val = getOclType(getExpandHiType(OT->getName()))->getSuffix();
        } else {
          GENOCL_WARNING("Invalid rewrite pattern: '" << pat << "'\n");
        }
    }
    ret += val;

    // if now is '#' concatenate mark - skip it
    if (text[dpos] == '#')
        dpos++;
    cpos = dpos;
    if (dpos >= text.size())
      break;
    // check next $
    dpos = text.find("$", cpos);
  }
  ret += text.substr(cpos);

  return ret;
}

static int
getNextVectorLength(int len)
{
  switch (len) {
    case 1: return 2;
    case 2: return 3;
    case 3: return 4;
    case 4: return 8;
    case 8: return 16;
  }
  return 0;
}

std::string
OclBuiltinDB::getNextNativeType(const std::string& in) const
{
  // FIXME: should avoid putting OCL semantic into backend as long as there's
  // simple and elegant way to put them in *.td files.

  const OclType* OT = getOclType(in);
  if (OT->isNative())
    return in;

  int len = OT->getVecLength();
  while ((len = getNextVectorLength(len))) {
    for (std::map<std::string, OclType*>::const_iterator I = m_TypeMap.begin(), E = m_TypeMap.end(); I != E; ++I) {
      const OclType* T = I->second;
      if (T->isNative() && T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType())
        return T->getName();
    }
  }
  GENOCL_WARNING("No next native type is found for type '" << in << "'\n");
  return "__invalid__";
}

// Get the vector with specified vector length with the corresponding element
// type the given type.
std::string
OclBuiltinDB::getVecType(const std::string& in, int len) const
{
  const OclType* OT = getOclType(in);
  if (OT->getVecLength() == len)
    return in;

  for (std::map<std::string, OclType*>::const_iterator I = m_TypeMap.begin(), E = m_TypeMap.end(); I != E; ++I) {
    const OclType* T = I->second;
    if (!T->isPointer() && T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType())
      return T->getName();
  }
  GENOCL_WARNING("No vector type is found for type '" << in << "'\n");
  return "__invalid__";
}

std::string
OclBuiltinDB::getExpandLoType(const std::string& in) const
{
  const OclType* OT = getOclType(in);

  int len = OT->getVecLength();
  switch (len) {
    default:
      len = len >> 1;
      break;
    case 3:
      len = 2;
      break;
  }

  for (std::map<std::string, OclType*>::const_iterator I = m_TypeMap.begin(), E = m_TypeMap.end(); I != E; ++I) {
    const OclType* T = I->second;
    if (T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType() &&
        T->isPointer() == OT->isPointer())
      return T->getName();
  }

  return "";
}

std::string
OclBuiltinDB::getExpandHiType(const std::string& in) const
{
  const OclType* OT = getOclType(in);

  int len = OT->getVecLength();
  switch (len) {
    default:
      len = len >> 1;
      break;
    case 3:
      len = 1;
      break;
  }

  for (std::map<std::string, OclType*>::const_iterator I = m_TypeMap.begin(), E = m_TypeMap.end(); I != E; ++I) {
    const OclType* T = I->second;
    if (T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType() &&
        T->isPointer() == OT->isPointer())
        return T->getName();
  }

  return "";
}

const OclType*
OclBuiltinDB::getOclType(const std::string& name) const
{
  if (m_TypeMap.find(name) != m_TypeMap.end())
    return m_TypeMap.find(name)->second;
  report_fatal_error("GenType " + name + " is not defined");
  return 0;
}

const OclBuiltin*
OclBuiltinDB::getOclBuiltin(const std::string& name) const
{
  if (m_ProtoMap.find(name) == m_ProtoMap.end())
    return 0;
  return m_ProtoMap.find(name)->second;
}

const OclBuiltinImpl*
OclBuiltinDB::getOclBuiltinImpl(const OclBuiltin* proto) const
{
  if (m_ImplMap.find(proto) == m_ImplMap.end())
    return 0;
  return m_ImplMap.find(proto)->second;
}

std::string
OclBuiltinDB::getSVMLRounding(const std::string& pat) const
{
  // New style SVML rounding name
  // KNC and CPU switched to new style rounding name.
  if (pat.substr(0, 4) == "$rtn") return "rte" + pat.substr(4);
  if (pat.substr(0, 4) == "$rtz") return "rtz" + pat.substr(4);
  if (pat.substr(0, 3) == "$up") return "rtp" + pat.substr(3);
  if (pat.substr(0, 5) == "$down") return "rtn" + pat.substr(5);

  report_fatal_error("Invalid rounding name.");
  return "";
}

/// OclBuiltinEmitter
OclBuiltinEmitter::OclBuiltinEmitter(RecordKeeper& R)
: m_Records(R)
, m_DB(R)
{
}

void
OclBuiltinEmitter::run(raw_ostream& OS)
{
  emitSourceFileHeader("OpenCL Builtins", OS);

  if (!GenOCLBuiltinPrototype)
    OS << RemoveCommonLeadingSpaces(m_DB.getProlog()) << "\n";

  for (OclBuiltinDB::const_proto_iterator I = m_DB.proto_begin(), E = m_DB.proto_end(); I != E; ++I) {
    const OclBuiltin* P = I->second;

    if (!GenOCLBuiltinPrototype && !P->needForwardDecl())
      continue;

    for (OclBuiltin::const_type_iterator J = P->type_begin(), E = P->type_end(); J != E; ++J) {
      if (P->shouldGenerate()) {
        OS << P->getCProto((*J)->getName(), true) << '\n';
      }
    }
    OS << '\n';
  }

  if (GenOCLBuiltinPrototype)
    return;
  else
    OS << '\n';

  for (OclBuiltinDB::const_proto_iterator I = m_DB.proto_begin(), E = m_DB.proto_end(); I != E; ++I) {
    const OclBuiltin* P = I->second;

    if (P->isDeclOnly())
      continue;

    const OclBuiltinImpl* Impl = m_DB.getOclBuiltinImpl(P);
    if (!Impl) {
      GENOCL_WARNING("'" << P->getName() << "' has no implementation, for target:'"
      << m_DB.getTarget() << "'\n");
      continue;
    }

    for (OclBuiltin::const_type_iterator J = P->type_begin(), E = P->type_end(); J != E; ++J) {
      std::string impl = Impl->getCImpl((*J)->getName());
      if (impl.empty()) {
        GENOCL_WARNING("'" << P->getName() << "' has no implementation on type '"
          << (*J)->getName() << "', for target: '" << m_DB.getTarget() << "'\n");
        continue;
      }
      OS << Impl->getCImpl((*J)->getName()) << '\n';
    }
  }

  OS << RemoveCommonLeadingSpaces(m_DB.getEpilog());
}
