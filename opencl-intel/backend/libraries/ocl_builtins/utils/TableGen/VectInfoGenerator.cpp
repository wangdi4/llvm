// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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

#include "VectInfoGenerator.h"

#include "ClangUtils.h"
#include "NameMangleAPI.h"
#include "OclBuiltinEmitter.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace llvm {

const std::string VectEntry::isaClass = "VectorVariant::ISAClass::XMM";
const std::string VectEntry::baseName = "\"\"";
std::string VectEntry::isMasked = "false";
std::string VectEntry::vectorKindEncode = "";

OclBuiltinDB *VectInfo::builtinDB = nullptr;

template <class T>
static VecVec<T> transpose(const std::vector<std::vector<T>> &matrix) {
  size_t rowSize = matrix.size();
  if (rowSize == 0)
    return {};
  size_t colSize = matrix[0].size();
  if (colSize == 0)
    return {};
  bool hasSameSize = true;
  for (auto &row : matrix) {
    hasSameSize &= (colSize == row.size());
  }
  assert(hasSameSize && "all rows must have the same size");
  VecVec<T> transposed;
  for (size_t i = 0; i < colSize; ++i) {
    std::vector<T> newRow;
    for (size_t j = 0; j < rowSize; ++j) {
      newRow.push_back(matrix[j][i]);
    }
    transposed.push_back(newRow);
  }
  return transposed;
}

VectInfo::VectInfo(Record *record) : m_builtins(4) {
  m_handleAlias = record->getValueAsBit("HandleAlias");

  const Record *pV1Func = record->getValueAsDef("v1Func");
  const Record *pV4Func = record->getValueAsDef("v4Func");
  const Record *pV8Func = record->getValueAsDef("v8Func");
  const Record *pV16Func = record->getValueAsDef("v16Func");

  std::vector<const Record *> funcs = {pV1Func, pV4Func, pV8Func, pV16Func};
  std::vector<std::vector<Record *>> types(4);
  std::transform(
      funcs.begin(), funcs.end(), types.begin(),
      [](const Record *r) { return r->getValueAsListOfDefs("types"); });
  m_numTypes = types[0].size();
  assert(m_numTypes > 0 && "No type binds to VectInfo!");
  std::vector<const Record *> builtins(4);
  std::transform(funcs.begin(), funcs.end(), builtins.begin(),
                 [](const Record *r) { return r->getValueAsDef("builtin"); });
  std::transform(builtins.begin(), builtins.end(), m_builtins.begin(),
                 [=](const Record *r) {
                   return builtinDB->getOclBuiltin(r->getNameInitAsString());
                 });
  auto getOclType = [=](const Record *r) {
    return builtinDB->getOclType(r->getNameInitAsString());
  };
  std::vector<std::vector<const OclType *>> oclTypes(4);
  std::transform(types.begin(), types.end(), oclTypes.begin(),
                 [=](const std::vector<Record *> &rs) {
                   std::vector<const OclType *> ts(rs.size());
                   std::transform(rs.begin(), rs.end(), ts.begin(), getOclType);
                   return std::move(ts);
                 });

  m_types = transpose(oclTypes);
}

std::ostream &operator<<(std::ostream &output, const VectEntry &Ent) {
  output << "{\"" << Ent.v1FuncName << "\", {" << VectEntry::isaClass << ", "
         << VectEntry::isMasked << ", " << 4 << ", "
         << VectEntry::vectorKindEncode << ", " << VectEntry::baseName << ", \""
         << Ent.v4FuncName << "\"}},\n";
  output << "{\"" << Ent.v1FuncName << "\", {" << VectEntry::isaClass << ", "
         << VectEntry::isMasked << ", " << 8 << ", "
         << VectEntry::vectorKindEncode << ", " << VectEntry::baseName << ", \""
         << Ent.v8FuncName << "\"}},\n";
  output << "{\"" << Ent.v1FuncName << "\", {" << VectEntry::isaClass << ", "
         << VectEntry::isMasked << ", " << 16 << ", "
         << VectEntry::vectorKindEncode << ", " << VectEntry::baseName << ", \""
         << Ent.v16FuncName << "\"}},\n";
  return output;
}

VectInfoGenerator::VectInfoGenerator(RecordKeeper &keeper)
    : m_RecordKeeper(keeper), m_DB(keeper) {}

void VectInfoGenerator::decodeParamKind(const std::string &scalarName,
                                        const std::string &v4FuncName) {

  reflection::FunctionDescriptor v1Func = demangle(scalarName.c_str(), false);
  reflection::FunctionDescriptor v4Func = demangle(v4FuncName.c_str(), false);
  const auto &v1Params = v1Func.parameters;
  const auto &v4Params = v4Func.parameters;
  std::stringstream ss;
  ss << '{';
  size_t v1ParamsNum = v1Params.size();
  VectEntry::isMasked = v1ParamsNum == v4Params.size() ? "false" : "true";
  for (size_t i = 0; i < v1ParamsNum; ++i) {
    const auto *v1ParamType = (const reflection::ParamType *)v1Params[i];
    const auto *v4ParamType = (const reflection::ParamType *)v4Params[i];
    reflection::TypeEnum v1TypeId = v1ParamType->getTypeId();
    reflection::TypeEnum v4TypeId = v4ParamType->getTypeId();
    if (v1TypeId == v4TypeId) {
      if (v1TypeId == reflection::TYPE_ID_VECTOR) {
        const auto *v1VectorType =
            static_cast<const reflection::VectorType *>(v1ParamType);
        if (v1VectorType->equals(v4ParamType)) {
          ss << "VectorKind::uniform()";
        } else {
          // Should be vector kind, do some check here.
          assert(v1VectorType->getLength() * 4 ==
                     static_cast<const reflection::VectorType *>(v4ParamType)
                         ->getLength() &&
                 "Unresolved vector kind");
          ss << "VectorKind::vector()";
        }
      } else {
        ss << "VectorKind::uniform()";
      }

    } else {
      // Should be vector kind, do some check here.
      assert(v4TypeId == reflection::TYPE_ID_VECTOR &&
             static_cast<const reflection::VectorType *>(v4ParamType)
                     ->getScalarType()
                     ->getTypeId() == v1TypeId &&
             "Unresolved vector kind");
      ss << "VectorKind::vector()";
    }
    ss << ", ";
  }
  ss << '}';
  VectEntry::vectorKindEncode = ss.str();
}

void VectInfoGenerator::generateFunctions(
    const std::vector<const OclBuiltin *> &builtins,
    const std::vector<const OclType *> &types, const SVecVec &funcNames) {
  // Generate dummy builtins.
  for (auto &aliasName : funcNames) {
    for (size_t t = 0; t < 4; ++t) {
      const OclBuiltin *pBuiltin = builtins[t];
      const OclType *pType = types[t];
      const std::string &strTy = pType->getName();

      std::string proto = pBuiltin->getCProto(strTy);
      // Get Alias proto.
      const std::string &funcName = pBuiltin->getCFunc(strTy);
      size_t pos = proto.find(funcName);
      size_t length = funcName.size();
      const std::string &rewritedAlias =
          m_DB.rewritePattern(pBuiltin, pType, aliasName[t], {});
      proto.replace(pos, length, rewritedAlias);

      // Process duplicate function
      FuncProto key = {{pBuiltin, rewritedAlias}, strTy};
      if (m_funcProtoToIndex.count(key)) {
        size_t pos = proto.find(rewritedAlias);
        size_t length = rewritedAlias.size();
        proto.replace(pos, length,
                      rewritedAlias + std::to_string(m_funcCounter));
        m_dupFuncToOrigFunc[m_funcCounter++] = m_funcProtoToIndex[key];
      } else {
        m_funcProtoToIndex[key] = m_funcCounter++;
      }
      m_funcStream << proto;
      std::string body =
          generateDummyBody(pBuiltin->getReturnBaseCType(strTy),
                            pBuiltin->getReturnVectorLength(strTy));
      assert(!body.empty() && "empty body?");
      m_funcStream << body;
      m_funcStream << "\n";
    }
  }
}

void VectInfoGenerator::run(raw_ostream &os) {

  m_funcCounter = 0;
  m_funcProtoToIndex.clear();
  m_dupFuncToOrigFunc.clear();
  m_funcStream.clear();
  m_funcStream.str("");

  VectInfo::builtinDB = &m_DB;
  std::vector<Record *> vectInfos =
      m_RecordKeeper.getAllDerivedDefinitions("VectInfo");

  // Record the num of types within the same Builtins. Since they
  // share the same vector kind info.
  std::vector<size_t> numEntries;

  for (Record *record : vectInfos) {
    VectInfo vectInfo{record};
    const auto &builtins = vectInfo.getBuiltins();

    auto *pV1Builtin = builtins[0];
    bool handleAlias = vectInfo.handleAlias() && m_DB.hasAlias(pV1Builtin);
    // Hold the raw func name and alias names.
    SVecVec funcNames;
    std::vector<std::string> rawName;
    for (auto *p : builtins)
      rawName.push_back(p->getCFunc());
    if (handleAlias) {
      numEntries.push_back(vectInfo.getNumOfTypes() *
                           (1 + m_DB.getAlias(pV1Builtin).size()));
      // Get the alias names.
      SVecVec aliasArray;
      std::for_each(builtins.begin(), builtins.end(), [&](const OclBuiltin *p) {
        assert(m_DB.hasAlias(p) &&
               "Some alias are missing for vectorized funcs");
        aliasArray.push_back(m_DB.getAlias(p));
      });
      funcNames = transpose(aliasArray);
    } else {
      numEntries.push_back(vectInfo.getNumOfTypes());
    }
    funcNames.push_back(rawName);
    // Generate dummy functions to get the mangled function name.
    for (VectInfo::type_iterator typeIt = vectInfo.type_begin(),
                                 typeEnd = vectInfo.type_end();
         typeIt != typeEnd; typeIt++)
      generateFunctions(builtins, *typeIt, funcNames);
  }
  build(m_funcStream.str(), "protos.ll");
  m_funcStream.clear();
  m_funcStream.str("");
  LLVMContext context;
  llvm::SMDiagnostic errDiagnostic;
  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("protos.ll", errDiagnostic, context);
  remove("protos.ll");

  // Restore the duplicate functions.
  std::vector<const Function *> funcs;
  std::for_each(pModule->begin(), pModule->end(),
                [&](const Function &fn) { funcs.push_back(&fn); });
  std::for_each(m_dupFuncToOrigFunc.begin(), m_dupFuncToOrigFunc.end(),
                [&](const std::pair<size_t, size_t> &item) {
                  funcs[item.first] = funcs[item.second];
                });

  // Every four functions forms a VectEntry.
  std::stringstream ss;
  auto funcIt = funcs.cbegin(), funcEnd = funcs.cend();
  for (size_t numEntry : numEntries) {
    size_t i = 0;
    decodeParamKind((*funcIt)->getName().str(),
                    (*(funcIt + 1))->getName().str());
    while (i++ < numEntry) {
      const std::string &v1FuncName = (*funcIt)->getName().str();
      funcIt++;
      const std::string &v4FuncName = (*funcIt)->getName().str();
      funcIt++;
      const std::string &v8FuncName = (*funcIt)->getName().str();
      funcIt++;
      const std::string &v16FuncName = (*funcIt)->getName().str();
      funcIt++;
      ss << VectEntry{v1FuncName, v4FuncName, v8FuncName, v16FuncName};
    }
  }
  assert(funcIt == funcEnd &&
         "the number of tblgen function and llvm functions should be the same");
  os << ss.str();
  ss.clear();
  ss.str("");
}

} // namespace llvm
