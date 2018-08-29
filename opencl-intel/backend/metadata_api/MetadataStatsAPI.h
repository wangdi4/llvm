// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#ifndef METADATASTATSAPI_H
#define METADATASTATSAPI_H

#include "MetadataAPI.h"

#include <algorithm>

namespace Intel {
namespace MetadataAPI {
// Module level statistics
struct ModuleStatMetadataAPI {
  typedef NamedMDValue<std::string, MDValueModuleStrategy> StatTypeTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> RunTimeVersionTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> WorkloadNameTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> ModuleNameTy;
  typedef NamedMDValue<std::string, MDValueModuleStrategy> ExecTimeTy;

  ModuleStatMetadataAPI(llvm::Module *pModule)
      : StatType(pModule, "opencl.stat.type"),
        RunTimeVersion(pModule, "opencl.stat.run_time_version"),
        WorkloadName(pModule, "opencl.stat.workload_name"),
        ModuleName(pModule, "opencl.stat.module_name"),
        ExecTime(pModule, "opencl.stat.exec_time") {}

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

  typedef llvm::SmallVector<StatTy, 8> StatListTy;
  typedef llvm::SmallVector<DescriptionTy, 8> DescriptionListTy;

  FunctionStatMetadataAPI(const llvm::GlobalObject &Object)
      : m_Object(const_cast<llvm::GlobalObject &>(Object)), m_isLoaded(false) {}

  StatListTy::iterator begin() {
    lazyLoad();
    return m_Stats.begin();
  }

  StatListTy::iterator end() {
    lazyLoad();
    return m_Stats.end();
  }

  StatListTy::const_iterator begin() const {
    lazyLoad();
    return m_Stats.begin();
  }

  StatListTy::const_iterator end() const {
    lazyLoad();
    return m_Stats.end();
  }

  bool empty() const
  {
    lazyLoad();
    return m_Stats.empty();
  }

private:
  void lazyLoad() const {
    if (m_isLoaded)
      return;

    llvm::SmallVector<llvm::StringRef, 8> MDStatKinds;
    getMDStatKindNames(m_Object, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      std::string MDStatKindStr = MDStatKind;
      FunctionStatTy MDStat(&m_Object, MDStatKindStr.c_str());

      if (MDStat.hasValue()) {
        std::string name = MDStatKind;
        auto value = MDStat.get();

        StatDescriptionTy StatDescription(m_Object.getParent(), name.c_str());

        assert(StatDescription.hasValue() && "Description must have value!");
        auto description = StatDescription.get();

        m_Stats.push_back({stripOpenCLStatKind(name), value, description});
      }
    }

    m_isLoaded = true;
  }

private:
  llvm::GlobalObject &m_Object;
  mutable bool m_isLoaded;
  mutable StatListTy m_Stats;

// static members
public:
  static void readDescription(llvm::Module &M, DescriptionListTy &Descs) {
    const auto &MDList = M.getNamedMDList();

    for (auto &namedMetadata : MDList) {
      auto name = namedMetadata.getName();
      if (isOpenCLStatKind(name)) {
        StatDescriptionTy StatDescription(&M, name.str().c_str());
        Descs.push_back({name, StatDescription.get()});
      }
    }
  }

  static void set(llvm::GlobalObject &Global, const StatListTy &Stats) {
    for (const auto &Stat : Stats) {
      std::string ClStatKind = toOpenCLStatKind(Stat.Name);
      FunctionStatTy MDStat(&Global, ClStatKind.c_str());
      MDStat.set(Stat.Value);

      // unique stat description
      StatDescriptionTy StatDescription(Global.getParent(), ClStatKind.c_str());

      if (!StatDescription.hasValue())
        StatDescription.set(Stat.Description);
    }
  }

  static void copy(const llvm::GlobalObject &Source, llvm::GlobalObject &Dest) {
    // assuming both functions are in the same LLVMContext.
    assert((&Source.getContext() == &Dest.getContext()) &&
           "Objects belong to different llvm contexts!");

    llvm::SmallVector<llvm::StringRef, 8> MDStatKinds;
    getMDStatKindNames(Source, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      if (auto *pMDNode = Source.getMetadata(MDStatKind))
        Dest.setMetadata(MDStatKind, pMDNode);
    }
  }

  static void remove(llvm::GlobalObject &Global) {
    llvm::SmallVector<llvm::StringRef, 8> MDStatKinds;
    getMDStatKindNames(Global, MDStatKinds);

    for (auto &MDStatKind : MDStatKinds) {
      if (Global.getMetadata(MDStatKind))
        Global.setMetadata(MDStatKind, nullptr);
    }
  }

  static void move(llvm::GlobalObject &Source, llvm::GlobalObject &Dest) {
    copy(Source, Dest);
    remove(Source);
  }

private:
  typedef NamedMDValueAccessor<NamedMDValue<std::string, MDValueModuleStrategy>>
      StatDescriptionTy;
  typedef NamedMDValueAccessor<
      NamedMDValue<int32_t, MDValueGlobalObjectStrategy>>
      FunctionStatTy;

  static bool isOpenCLStatKind(llvm::StringRef Kind) {
    return Kind.startswith("opencl.stats.");
  }

  static std::string toOpenCLStatKind(std::string Kind) {
    return Kind.insert(0, "opencl.stats.");
  }

  static std::string stripOpenCLStatKind(std::string Kind) {
    assert(Kind.substr(0, 13) == "opencl.stats." &&
      "Expected stats prefix!");
    return Kind.erase(0, 13);
  }

  // populates statistic names
  static void
  getMDStatKindNames(const llvm::GlobalObject &Global,
                     llvm::SmallVectorImpl<llvm::StringRef> &Result) {
    auto &Context = Global.getContext();
    llvm::SmallVector<llvm::StringRef, 8> MDKindNames;
    Context.getMDKindNames(MDKindNames);

    std::copy_if(MDKindNames.begin(), MDKindNames.end(),
                 std::back_inserter(Result),
                 [](llvm::StringRef Kind) { return isOpenCLStatKind(Kind); });
  }
};
} // end namespace MetadataAPI
} // end namespace Intel

#endif
