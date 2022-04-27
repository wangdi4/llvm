//===-- MetadataStatsAPI.h --------------------------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_METADATASTATSAPI_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_METADATASTATSAPI_H

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPIImpl.h"
#include <algorithm>

namespace llvm {
namespace DPCPPKernelMetadataAPI {

// Module level statistics
struct ModuleStatMetadataAPI {
  typedef NamedMDValue<std::string, MDValueModuleStrategy> StatTypeTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> RunTimeVersionTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> WorkloadNameTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> ModuleNameTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> ExecTimeTy;

  ModuleStatMetadataAPI(Module *M)
      : StatType(M, "dpcpp.stat.type"),
        RunTimeVersion(M, "dpcpp.stat.run_time_version"),
        WorkloadName(M, "dpcpp.stat.workload_name"),
        ModuleName(M, "dpcpp.stat.module_name"),
        ExecTime(M, "dpcpp.stat.exec_time") {}

  NamedMDValueAccessor<StatTypeTy> StatType;
  NamedMDValueAccessor<RunTimeVersionTy> RunTimeVersion;
  NamedMDValueAccessor<WorkloadNameTy> WorkloadName;
  NamedMDValueAccessor<ModuleNameTy> ModuleName;
  NamedMDValueAccessor<ExecTimeTy> ExecTime;
};

struct FunctionStatMetadataAPI {
  struct StatTy {
    std::string Name;
    int32_t Value;
    std::string Description;
  };

  struct DescriptionTy {
    std::string Name;
    std::string Description;
  };

  typedef SmallVector<StatTy, 8> StatListTy;
  typedef SmallVector<DescriptionTy, 8> DescriptionListTy;

  FunctionStatMetadataAPI(const GlobalObject &Object)
      : Obj(const_cast<GlobalObject &>(Object)), IsLoaded(false) {}

  StatListTy::iterator begin() {
    lazyLoad();
    return Stats.begin();
  }

  StatListTy::iterator end() {
    lazyLoad();
    return Stats.end();
  }

  StatListTy::const_iterator begin() const {
    lazyLoad();
    return Stats.begin();
  }

  StatListTy::const_iterator end() const {
    lazyLoad();
    return Stats.end();
  }

  bool empty() const {
    lazyLoad();
    return Stats.empty();
  }

private:
  void lazyLoad() const {
    if (IsLoaded)
      return;

    SmallVector<StringRef, 8> MDStatKinds;
    getMDStatKindNames(Obj, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      std::string MDStatKindStr = std::string(MDStatKind);
      FunctionStatTy MDStat(&Obj, MDStatKindStr.c_str());

      if (MDStat.hasValue()) {
        std::string Name = std::string(MDStatKind);
        auto Value = MDStat.get();

        StatDescriptionTy StatDescription(Obj.getParent(), Name.c_str());

        assert(StatDescription.hasValue() && "Description must have value!");
        auto Description = StatDescription.get();

        Stats.push_back({stripDPCPPStatKind(Name), Value, Description});
      }
    }

    IsLoaded = true;
  }

private:
  GlobalObject &Obj;
  mutable bool IsLoaded;
  mutable StatListTy Stats;

  // static members
public:
  static void readDescription(Module &M, DescriptionListTy &Descs) {
    const auto &MDList = M.getNamedMDList();

    for (auto &NamedMetadata : MDList) {
      auto Name = NamedMetadata.getName();
      if (isDPCPPStatKind(Name)) {
        StatDescriptionTy StatDescription(&M, Name.str().c_str());
        Descs.push_back({std::string(Name), StatDescription.get()});
      }
    }
  }

  static void set(GlobalObject &Global, const StatListTy &Stats) {
    for (const auto &Stat : Stats) {
      std::string ClStatKind = toDPCPPStatKind(Stat.Name);
      FunctionStatTy MDStat(&Global, ClStatKind.c_str());
      MDStat.set(Stat.Value);

      // unique stat description
      StatDescriptionTy StatDescription(Global.getParent(), ClStatKind.c_str());

      if (!StatDescription.hasValue())
        StatDescription.set(Stat.Description);
    }
  }

  static void copy(const GlobalObject &Source, GlobalObject &Dest) {
    // assuming both functions are in the same LLVMContext.
    assert((&Source.getContext() == &Dest.getContext()) &&
           "Objects belong to different llvm contexts!");

    SmallVector<StringRef, 8> MDStatKinds;
    getMDStatKindNames(Source, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      if (auto *N = Source.getMetadata(MDStatKind))
        Dest.setMetadata(MDStatKind, N);
    }
  }

  static void remove(GlobalObject &Global) {
    SmallVector<StringRef, 8> MDStatKinds;
    getMDStatKindNames(Global, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      if (Global.getMetadata(MDStatKind))
        Global.setMetadata(MDStatKind, nullptr);
    }
  }

  static void move(GlobalObject &Source, GlobalObject &Dest) {
    copy(Source, Dest);
    remove(Source);
  }

private:
  typedef NamedMDValueAccessor<NamedMDValue<std::string, MDValueModuleStrategy>>
      StatDescriptionTy;
  typedef NamedMDValueAccessor<
      NamedMDValue<int32_t, MDValueGlobalObjectStrategy>>
      FunctionStatTy;

  static bool isDPCPPStatKind(StringRef Kind) {
    return Kind.startswith("dpcpp.stats.");
  }

  static std::string toDPCPPStatKind(std::string Kind) {
    return Kind.insert(0, "dpcpp.stats.");
  }

  static std::string stripDPCPPStatKind(std::string Kind) {
    assert(isDPCPPStatKind(Kind) && "Expected stats prefix!");
    return Kind.erase(0, 12);
  }

  // populates statistic names
  static void getMDStatKindNames(const GlobalObject &Global,
                                 SmallVectorImpl<StringRef> &Result) {
    auto &Context = Global.getContext();
    SmallVector<StringRef, 8> MDKindNames;
    Context.getMDKindNames(MDKindNames);

    std::copy_if(MDKindNames.begin(), MDKindNames.end(),
                 std::back_inserter(Result),
                 [](StringRef Kind) { return isDPCPPStatKind(Kind); });
  }
};

} // namespace DPCPPKernelMetadataAPI
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_METADATASTATSAPI_H
