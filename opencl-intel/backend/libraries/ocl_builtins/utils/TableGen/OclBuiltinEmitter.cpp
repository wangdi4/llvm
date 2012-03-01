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


#define GENOCL_WARNING(X)                                                         \
  do { errs() << (GenOCLBuiltinWerror ? "ERROR: " : "WARNING: ") << X;            \
    if (GenOCLBuiltinWerror) report_fatal_error("Warning is reported as error."); \
  } while (0)

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


/// OclGenType
OclGenType::OclGenType(const OclBuiltinDB& DB, const Record* R)
: OclType(DB, R)
{
  assert(R->isSubClassOf("OclGenType") && "Invalid OclGenType record.");

  ListInit* Tin = R->getValueAsListInit("Tin");
  ListInit* Tout = R->getValueAsListInit("Tout");

  assert(Tin && Tout && Tin->getSize() > 0 && Tin->getSize() == Tout->getSize() && "Invalid OclGenType record.");

  if (!(Tin && Tout && Tin->getSize() > 0 && Tin->getSize() == Tout->getSize())) {
    GENOCL_WARNING("Invalid gentype '" << m_Name << "' is found.\n");
    return;
  }

  for (ListInit::const_iterator I = Tin->begin(),
                                O = Tout->begin(),
                                E = Tin->end(); I != E; ++I, ++O) {
    const std::string& istr = dynamic_cast<StringInit*>(*I)->getValue();
    const std::string& ostr = dynamic_cast<StringInit*>(*O)->getValue();
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


/// OclBuiltin
OclBuiltin::OclBuiltin(const OclBuiltinDB& DB, const Record* R)
: m_DB(DB)
, m_Record(R)
, m_Name(R->getName())
, m_CFunc(R->getValueAsString("Name"))
, m_IsDeclOnly(R->getValueAsBit("IsDeclOnly"))
, m_NeedForwardDecl(R->getValueAsBit("NeedForwardDecl"))
, m_HasConst(0)
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

    assert(dynamic_cast<DefInit*>(Outs->getOperator()) && 
      dynamic_cast<DefInit*>(Outs->getOperator())->getDef()->getName() == "outs" && 
      "Invalid OclBuiltin record with invalid outputs.");

    for (unsigned i = 0, e = Outs->getNumArgs(); i != e; ++i) {
      const OclType* ArgTy = m_DB.getOclType(dynamic_cast<DefInit*>(Outs->getArg(i))->getDef()->getName());
      const std::string& ArgName = Outs->getArgName(i);
      m_Outputs.push_back(std::pair<const OclType*, std::string>(ArgTy, ArgName));
    }
  }

  // Ins
  {
    DagInit* Ins = R->getValueAsDag("Ins");
    assert(Ins && "Invalid OclBuiltin record without ins.");

    assert(dynamic_cast<DefInit*>(Ins->getOperator()) && 
      dynamic_cast<DefInit*>(Ins->getOperator())->getDef()->getName() == "ins" && 
      "Invalid OclBuiltin record with invalid outputs.");
    for (unsigned i = 0, e = Ins->getNumArgs(); i != e; ++i) {
      const OclType* ArgTy = m_DB.getOclType(dynamic_cast<DefInit*>(Ins->getArg(i))->getDef()->getName());
      const std::string& ArgName = Ins->getArgName(i);
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
}


OclBuiltin::~OclBuiltin()
{
  for (std::vector<const OclBuiltinAttr*>::const_iterator I = m_Attrs.begin(), E = m_Attrs.end(); I != E; ++I)
    delete *I;
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
OclBuiltin::getReturnCName(const std::string&) const
{
  assert(m_Outputs.size() == 1 && "Illegal return variable name for OclBuiltin without a single output.");

  return m_Outputs[0].second;
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
OclBuiltin::getArgumentCName(unsigned i, const std::string&) const
{
  assert(i < m_Inputs.size() && "Argument index is out of bound.");

  return m_Inputs[i].second;
}

std::string
OclBuiltin::getCFunc(const std::string& TyName) const
{
  const OclType* Ty = m_DB.getOclType(TyName);
  return m_DB.rewritePattern(0, Ty, m_CFunc);
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

bool
OclBuiltin::isValidType(const std::string& TyName) const
{
  for (std::vector<const OclType*>::const_iterator I = m_Types.begin(),
                                                   E = m_Types.end(); I != E; ++I)
    if ((*I)->getName() == TyName)
      return true;

  return false;
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
      if (!(*I)->m_IsDeclOnly) {
        ret += m_Proto->getCProto(in);
        ret += "\n{";
        ret += RemoveCommonLeadingSpaces(m_DB.rewritePattern(m_Proto, Ty, (*I)->m_Code));
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

  Impl* impl = new Impl;
  impl->m_Record = R;
  // IsDeclOnly
  impl->m_IsDeclOnly = R->getValueAsBit("IsDeclOnly");
  // Types
  {
    std::vector<Record*> Tys;
    const RecordVal* RV = R->getValue("Types");
    if (VarInit* FI = dynamic_cast<VarInit*>(RV->getValue())) {
      const RecordVal* IV = m_DB.getRecord()->getValue(FI->getName());
      assert(dynamic_cast<ListInit*>(IV->getValue()) && "Invalid OclBuiltinImpl record.");
      ListInit* List = dynamic_cast<ListInit*>(IV->getValue());
      for (unsigned i = 0; i != List->getSize(); ++i) {
        DefInit* DI = dynamic_cast<DefInit*>(List->getElement(i));
        assert(DI && "Invalid OclBuiltinImpl record, list is not entirely DefInit.");
        Tys.push_back(DI->getDef());
      }
    } else
      Tys = R->getValueAsListOfDefs("Types");
    assert(Tys.size() > 0 && "Invalid OclBuiltinImpl record with empty type list.");

    for (std::vector<Record*>::const_iterator I = Tys.begin(), E = Tys.end(); I != E; ++I) {
      if (!m_Proto->isValidType((*I)->getName())) {
        if (GenOCLBuiltinVerbose)
          GENOCL_WARNING("'" << R->getName() << "' specifies invalid type '" << (*I)->getName() << "'.\n");
        continue;
      }
      impl->m_Types.push_back(m_DB.getOclType((*I)->getName()));
    }
  }
  // Impl
  {
    const RecordVal* RV = R->getValue("Impl");
    if (VarInit* FI = dynamic_cast<VarInit*>(RV->getValue())) {
      const RecordVal* IV = m_DB.getRecord()->getValue(FI->getName());
      assert(dynamic_cast<CodeInit*>(IV->getValue()) && "Invalid OclBuiltinImpl record.");
      impl->m_Code = dynamic_cast<CodeInit*>(IV->getValue())->getValue();
    } else
      impl->m_Code = R->getValueAsCode("Impl");
  }
  m_Impls.push_back(impl);
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
    std::vector<Record*> Rs = m_Records.getAllDerivedDefinitions("Generic");

    // One and only one single instance of OclBuiltins is defined.
    assert(Rs.size() > 0 && "No Generic is defined!");
    assert(Rs.size() < 2 && "More than 1 Generic are defined!");

    const Record* Rec = m_Record = Rs.front();

    // Target
    m_Target = Rec->getValueAsString("Target");

    // Prolog & Epilog
    {
      m_Prolog = Rec->getValueAsCode("Prolog");
      m_Epilog = Rec->getValueAsCode("Epilog");
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
        if (dynamic_cast<UnsetInit*>(Def))
          continue;

        // Not convertible, skip it as well.
        if (!Def->convertInitializerTo(OBI))
          continue;

        const Record* DefRec = dynamic_cast<DefInit*>(Def)->getDef();
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
      const std::vector<Record*>& SCs = Rec->getSuperClasses();
      for (std::vector<Record*>::const_reverse_iterator I = SCs.rbegin(), E = SCs.rend(); I != E; ++I) {
        const Record* Rec = *I;
        const std::vector<RecordVal>& Values = Rec->getValues();
        for (size_t i = 0, e = Values.size(); i !=e ; ++i) {
          const RecordVal& RV = Values[i];
          Init* Def = RV.getValue();

          // UnsetInit could be converted to any type, skip it first.
          if (dynamic_cast<UnsetInit*>(Def))
            continue;

          // Not convertible, skip it as well.
          if (!Def->convertInitializerTo(OBI))
            continue;

          const Record* DefRec = dynamic_cast<DefInit*>(Def)->getDef();
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
OclBuiltinDB::rewritePattern(const OclBuiltin* OB, const OclType* OT, const std::string& text) const
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
      // Skip 'alphanum'
    } while (isalnum(text[dpos]));

    std::string pat = text.substr(cpos, dpos - cpos);

    // replace $pat with real text
    std::string val;

    if ("$Target" == pat) {
      val = getTarget();
    } else if ("$Suffix" == pat) {
      val = OT->getSuffix();
    } else if ("$SVMLSuffix" == pat) {
      val = OT->getSVMLSuffix();
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
    } else if ("$ReturnType" == pat) {
      val = OB->getReturnCType(OT->getName());
    } else if ("$ReturnVarName" == pat) {
      val = OB->getReturnCName(OT->getName());
    } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 9 && "Type" == pat.substr(5)) {
      unsigned i = pat[4] - '0';
      val = OB->getArgumentCType(i, OT->getName());
    } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 12 && "VecType" == pat.substr(5)) {
      unsigned i = pat[4] - '0';
      val = OB->getArgumentCVecType(i, OT->getName(), OT->getVecLength());
    } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 13 && "NoASType" == pat.substr(5)) {
      unsigned i = pat[4] - '0';
      val = OB->getArgumentCNoASType(i, OT->getName());
    } else if ("$Arg" == pat.substr(0, 4) && pat.size() == 12 && "VarName" == pat.substr(5)) {
      unsigned i = pat[4] - '0';
      val = OB->getArgumentCName(i, OT->getName());
    } else if ("$NativeFunc" == pat) {
      val = OB->getNativeCFunc(OT->getName());
    } else if ("$NativeReturnType" == pat) {
      val = OB->getNativeReturnCType(OT->getName());
    } else if ("$NativeArg" == pat.substr(0, 10) && pat.size() == 15 && "Type" == pat.substr(11)) {
      unsigned i = pat[10] - '0';
      val = OB->getNativeArgumentCType(i, OT->getName());
    } else if ("$ExpandLoFunc" == pat) {
      val = OB->getExpandLoCFunc(OT->getName());
    } else if ("$ExpandHiFunc" == pat) {
      val = OB->getExpandHiCFunc(OT->getName());
    } else if ("$ExpandLoPattern" == pat) {
      val = OT->getExpandLoCPattern();
    } else if ("$ExpandHiPattern" == pat) {
      val = OT->getExpandHiCPattern();
    } else {
      GENOCL_WARNING("Invalid rewrite pattern: '" << pat << "'\n");
    }
    ret += val;

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
    if (T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType())
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
    if (T->getVecLength() == len && T->getBaseCType() == OT->getBaseCType())
      return T->getName();
  }

  return "";
}

const OclType*
OclBuiltinDB::getOclType(const std::string& name) const
{
  if (m_TypeMap.find(name) == m_TypeMap.end())
    return 0;
  return m_TypeMap.find(name)->second;
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

/// OclBuiltinEmitter
OclBuiltinEmitter::OclBuiltinEmitter(RecordKeeper& R)
: m_Records(R)
, m_DB(R)
{
}

void
OclBuiltinEmitter::run(raw_ostream& OS)
{
  EmitSourceFileHeader("OpenCL Builtins", OS);

  if (!GenOCLBuiltinPrototype)
    OS << RemoveCommonLeadingSpaces(m_DB.getProlog()) << "\n";

  for (OclBuiltinDB::const_proto_iterator I = m_DB.proto_begin(), E = m_DB.proto_end(); I != E; ++I) {
    const OclBuiltin* P = I->second;

    if (!GenOCLBuiltinPrototype && !P->needForwardDecl())
      continue;

    for (OclBuiltin::const_type_iterator J = P->type_begin(), E = P->type_end(); J != E; ++J) {
      OS << P->getCProto((*J)->getName(), true) << '\n';
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
      GENOCL_WARNING("'" << P->getName() << "' has no implementation.\n");
      continue;
    }

    for (OclBuiltin::const_type_iterator J = P->type_begin(), E = P->type_end(); J != E; ++J) {
      std::string impl = Impl->getCImpl((*J)->getName());
      if (impl.empty()) {
        GENOCL_WARNING("'" << P->getName() << "' has no implementation on type '" << (*J)->getName() << "'.\n");
        continue;
      }
      OS << Impl->getCImpl((*J)->getName()) << '\n';
    }
  }

  OS << RemoveCommonLeadingSpaces(m_DB.getEpilog());
}
