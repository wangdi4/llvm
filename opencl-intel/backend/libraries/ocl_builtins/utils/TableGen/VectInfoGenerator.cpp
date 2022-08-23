// Copyright (C) 2012-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "VectInfoGenerator.h"
#include "ClangUtils.h"
#include "OclBuiltinEmitter.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"

#define DEBUG_TYPE "vect-info-gen"

namespace llvm {

const VectorVariant::ISAClass VectEntry::isaClass =
    VectorVariant::ISAClass::XMM;
const std::string VectEntry::baseName = "";
bool VectEntry::isMasked = false;
bool VectEntry::kernelCallOnce = true;
unsigned VectEntry::stride = 0;
std::vector<VectorKind> VectEntry::vectorKindEncode;

OclBuiltinDB *VectInfo::builtinDB = nullptr;

VectorVariant getVectorVariant(unsigned int V, const std::string &BaseName,
                               const std::string &Alias) {
  return VectorVariant{VectEntry::isaClass,
                       VectEntry::isMasked,
                       V,
                       VectEntry::vectorKindEncode,
                       BaseName,
                       Alias};
}

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
  m_stride = record->getValueAsInt("Stride");
  const Record *pV1Func = record->getValueAsDef("v1Func");
  const Record *pV4Func = record->getValueAsDef("v4Func");
  const Record *pV8Func = record->getValueAsDef("v8Func");
  const Record *pV16Func = record->getValueAsDef("v16Func");
  const Record *pV32Func = record->getValueAsDef("v32Func");
  const Record *pV64Func = record->getValueAsDef("v64Func");

  const std::string &v32BuiltinName =
      pV32Func->getValueAsDef("builtin")->getNameInitAsString();
  const std::string &v64BuiltinName =
      pV64Func->getValueAsDef("builtin")->getNameInitAsString();

  std::vector<const Record *> funcs = {pV1Func, pV4Func, pV8Func, pV16Func};
  size_t numOfVariants = 4;
  if (v32BuiltinName != "unimplemented") {
    funcs.push_back(pV32Func);
    numOfVariants++;
    if (v64BuiltinName != "unimplemented") {
      funcs.push_back(pV64Func);
      numOfVariants++;
    }
    m_builtins.resize(numOfVariants);
  }

  // Get OclType.
  std::vector<std::vector<Record *>> types(numOfVariants);
  std::transform(
      funcs.begin(), funcs.end(), types.begin(),
      [](const Record *r) { return r->getValueAsListOfDefs("types"); });
  assert(types[0].size() > 0 && "No type binds to VectInfo!");

  auto getOclType = [=](const Record *r) {
    return builtinDB->getOclType(r->getNameInitAsString());
  };
  std::vector<std::vector<const OclType *>> oclTypes(numOfVariants);
  std::transform(types.begin(), types.end(), oclTypes.begin(),
                 [=](const std::vector<Record *> &rs) {
                   std::vector<const OclType *> ts(rs.size());
                   std::transform(rs.begin(), rs.end(), ts.begin(), getOclType);
                   return ts;
                 });

  m_types = transpose(oclTypes);

  // Get OclBuiltin.
  std::transform(funcs.begin(), funcs.end(), m_builtins.begin(),
                 [](const Record *r) {
                   return builtinDB->getOclBuiltin(
                       r->getValueAsDef("builtin")->getNameInitAsString());
                 });
}

static void generateVectInfos(const VectEntry &Ent,
                              std::set<std::string> &AllVectInfos) {
  unsigned NumVariants = Ent.funcNames.size();
  for (unsigned I = 1; I < NumVariants; ++I) {
    std::stringstream S;
    S << '"' << Ent.funcNames[0] << "\",\"";
    if (Ent.kernelCallOnce) {
      S << "kernel-call-once\",\"";
    } else {
      S << "\",\"";
    }

    S << getVectorVariant((unsigned)2 << I, Ent.funcNames[0], Ent.funcNames[I])
             .toString()
      << '"';
    AllVectInfos.insert(S.str());
  }
}

VectInfoGenerator::VectInfoGenerator(RecordKeeper &keeper)
    : m_RecordKeeper(keeper), m_DB(keeper, /*CollectImplDefs*/ false) {}

void VectInfoGenerator::decodeParamKind(StringRef scalarName,
                                        StringRef v4FuncName) {
  reflection::FunctionDescriptor v1Func =
      NameMangleAPI::demangle(scalarName, false);
  reflection::FunctionDescriptor v4Func =
      NameMangleAPI::demangle(v4FuncName, false);
  const auto &v1Params = v1Func.Parameters;
  const auto &v4Params = v4Func.Parameters;
  size_t v1ParamsNum = v1Params.size();
  VectEntry::isMasked = v1ParamsNum == (v4Params.size() - 1);
  VectEntry::vectorKindEncode.clear();
  for (size_t i = 0; i < v1ParamsNum; ++i) {
    const auto *v1ParamType = (const reflection::ParamType *)v1Params[i].get();
    const auto *v4ParamType = (const reflection::ParamType *)v4Params[i].get();
    reflection::TypeEnum v1TypeId = v1ParamType->getTypeId();
    reflection::TypeEnum v4TypeId = v4ParamType->getTypeId();
    if (v1TypeId == v4TypeId) {
      if (v1TypeId == reflection::TYPE_ID_VECTOR) {
        const auto *v1VectorType =
            static_cast<const reflection::VectorType *>(v1ParamType);
        if (v1VectorType->equals(v4ParamType)) {
          VectEntry::vectorKindEncode.push_back(VectorKind::uniform());
        } else {
          // Should be vector kind, do some check here.
          assert(v1VectorType->getLength() * 4 ==
                     static_cast<const reflection::VectorType *>(v4ParamType)
                         ->getLength() &&
                 "Unresolved vector kind");
          VectEntry::vectorKindEncode.push_back(VectorKind::vector());
        }
      } else {
        if (v1TypeId == reflection::TYPE_ID_POINTER && VectEntry::stride != 0)
          VectEntry::vectorKindEncode.push_back(
              VectorKind::linear(VectEntry::stride));
        else
          VectEntry::vectorKindEncode.push_back(VectorKind::uniform());
      }

    } else {
      // Should be vector kind, do some check here.
      assert(v4TypeId == reflection::TYPE_ID_VECTOR &&
             static_cast<const reflection::VectorType *>(v4ParamType)
                     ->getScalarType()
                     ->getTypeId() == v1TypeId &&
             "Unresolved vector kind");
      VectEntry::vectorKindEncode.push_back(VectorKind::vector());
    }
  }
}

void VectInfoGenerator::generateFunctions(
    const std::vector<const OclBuiltin *> &builtins,
    const std::vector<const OclType *> &types, const SVecVec &funcNames) {
  // Generate dummy builtins.
  size_t numOfVariants = builtins.size();
  for (auto &aliasName : funcNames) {
    for (size_t t = 0; t < numOfVariants; ++t) {
      const OclBuiltin *pBuiltin = builtins[t];
      const OclType *pType = types[t];
      const std::string &strTy = pType->getName();

      std::string proto = pBuiltin->getCProto(strTy);
      // Get Alias proto.
      const std::string &funcName = pBuiltin->getCFunc(strTy);
      LLVM_DEBUG(dbgs() << "Generating dummy function body for " << funcName
                        << "\n  #Variant = " << t << "  Proto: " << proto
                        << "\n  Type: " << strTy << '\n');
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

bool isVPlanMaskedFunctionVectorVariant(Function &F, VectorVariant &Variant,
                                        Type *CharacteristicType) {
  if (!Variant.isMasked())
    return false;

  assert(!CharacteristicType->isVoidTy() &&
         "Characteristic type should not be void");

  FunctionType *FnType = F.getFunctionType();
  // Mask argument is always the last argument
  assert(1 <= F.arg_size() && "Unexpected mask argument for vector variant!");
  unsigned MaskArgIdx = F.arg_size() - 1;
  auto *MaskType = cast<VectorType>(FnType->getParamType(MaskArgIdx));
  auto *MaskElementType = MaskType->getElementType();

  return CharacteristicType == MaskElementType;
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

  // Record the num of types and num of variants for the same Builtin.
  // Since they share the same vector kind info.
  std::vector<std::pair<size_t, size_t>> numEntries;
  std::vector<const OclBuiltin *> ScalarBuiltins;
  std::vector<unsigned> strides;

  for (Record *record : vectInfos) {
    LLVM_DEBUG(dbgs() << "Processing VectInfo record: " << *record << '\n');
    VectInfo vectInfo{record};

    strides.push_back(vectInfo.stride());
    const auto &builtins = vectInfo.getBuiltins();

    auto *pV1Builtin = builtins[0];
    ScalarBuiltins.push_back(pV1Builtin);
    bool handleAlias = vectInfo.handleAlias() && m_DB.hasAlias(pV1Builtin);

    // Hold the alias names defined in AliasMap.
    SVecVec aliasNames;
    // Hold the raw func name defined in OclBuiltin.
    std::vector<std::string> rawName;
    // Hold the OclTypes specified in AliasMap for the scalar builtin.
    std::set<const OclType *> v1AliasTypes;

    for (auto *p : builtins)
      rawName.push_back(p->getCFunc());
    if (handleAlias) {
      // Get the alias names and specified types.
      SVecVec aliasArray;
      std::for_each(builtins.begin(), builtins.end(), [&](const OclBuiltin *p) {
        assert(m_DB.hasAlias(p) &&
               "Some alias are missing for vectorized funcs");
        aliasArray.push_back(m_DB.getAliasNames(p));
      });
      aliasNames = transpose(aliasArray);
      v1AliasTypes = m_DB.getAliasTypes(pV1Builtin);
    }

    size_t numAliasNames = m_DB.getAliasNames(pV1Builtin).size();
    size_t numEntry = vectInfo.getNumOfTypes();
    // Generate dummy functions to get the mangled function name.
    for (VectInfo::type_iterator typeIt = vectInfo.type_begin(),
                                 typeEnd = vectInfo.type_end();
         typeIt != typeEnd; typeIt++) {
      generateFunctions(builtins, *typeIt, {rawName});
      // Note: We only check the specified types for scalar builtin here.
      if (handleAlias && v1AliasTypes.count((*typeIt)[0])) {
        generateFunctions(builtins, *typeIt, aliasNames);
        numEntry += numAliasNames;
      }
    }
    numEntries.emplace_back(numEntry, builtins.size());
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
  std::vector<Function *> funcs;
  std::for_each(pModule->begin(), pModule->end(),
                [&](Function &fn) { funcs.push_back(&fn); });
  std::for_each(m_dupFuncToOrigFunc.begin(), m_dupFuncToOrigFunc.end(),
                [&](const std::pair<size_t, size_t> &item) {
                  funcs[item.first] = funcs[item.second];
                });

  static std::set<std::string> AllVectInfos;
  // Generating list of variants who use VPlan-fashioned masks.
  static std::set<std::string> VPlanMaskedFuncs;

  auto funcIt = funcs.cbegin();
  size_t k = 0;
  for (const auto &numEntry : numEntries) {
    VectEntry::kernelCallOnce = ScalarBuiltins[k]->hasKernelCallOnce();
    VectEntry::stride = strides[k++];
    size_t i = 0;
    assert(
        funcIt != funcs.cend() && funcIt != funcs.cend() - 1 &&
        "the number of tblgen functions and llvm functions should be the same");
    decodeParamKind((*funcIt)->getName(), (*(funcIt + 1))->getName());
    while (i++ < numEntry.first) {
      size_t j = 0;
      std::vector<std::string> funcNames;
      auto origIt = funcIt; // original LLVM function

      while (j++ < numEntry.second) {
        assert(funcIt != funcs.cend() && "the number of tblgen functions and "
                                         "llvm functions should be the same");

        std::string fname((*funcIt)->getName());
        size_t m = funcNames.size();

        std::string BaseName((*origIt)->getName());

        // for vector variant
        if (m > 0) {
          auto variant = getVectorVariant((unsigned)2 << m, BaseName, fname);
          auto *characteristicType = calcCharacteristicType(**origIt, variant);

          if (isVPlanMaskedFunctionVectorVariant(**funcIt, variant,
                                                 characteristicType)) {
            VPlanMaskedFuncs.insert(fname);
          }
        }

        funcNames.push_back(fname);
        funcIt++;
      }
      generateVectInfos(VectEntry{funcNames}, AllVectInfos);
    }
  }

  assert(k == ScalarBuiltins.size() && k == strides.size() &&
         "the number of entries and scalar builtins should be the same");
  assert(
      funcIt == funcs.cend() &&
      "the number of tblgen functions and llvm functions should be the same");
  os << "#ifndef IMPORT_VPLAN_MASKED_VARIANTS\n";
  for (auto &S : AllVectInfos)
    os << '{' << S << "},\n";

  os << "#else\n";
  os << "{\n";
  for (auto &S : VPlanMaskedFuncs)
    os << '"' << S << '"' << ",\n";
  os << "}\n";
  os << "#endif // IMPORT_VPLAN_MASKED_VARIANTS\n";
}

} // namespace llvm
