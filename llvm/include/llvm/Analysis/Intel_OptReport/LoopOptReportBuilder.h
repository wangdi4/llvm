//===------ LoopOptReportBuilder.h ----------------------------*- C++ -*---===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines LoopOptReportBuilder class. That is the main interface of
// loop optimization reports for transformations.
//
// For more details on how to use loop transformation reports, please look at:
//     docs/Intel/OptReport.rst
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
#define LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DebugLoc.h"

#include <type_traits>
#include <utility>

namespace llvm {

template <typename T> class LoopOptReportThunk;
template <typename T> struct LoopOptReportTraits;

/// The verbosity level of loop optimization reports.
/// For detailed description, please look at:
///     docs/Intel/OptReport.rst
namespace OptReportVerbosity {
enum Level { None = 0, Low = 1, Medium = 2, High = 3 };
}

/// \brief Builder for OptReports.
///
/// Optimization passes are expected to have one object of this type. Such an
/// object incapsulates all the information that is necessary for OptReport
/// generation and is constant during a single run of the pass. A typical usage
/// of the class looks like this:
///
///   LoopOptReportBuilder LORBuilder(...);
///   LORBuilder(Lp).addRemark(MyFancyRemark);
///
/// The key part of the LoopOptReportBuilder API is operator(), which creates an
/// instance of LoopOptReportThunk for object Lp with convenient interface for
/// OptReport manipulations.
class LoopOptReportBuilder {
  LLVMContext *Context;
  OptReportVerbosity::Level OutputVerbosity;

  LoopOptReportBuilder(const LoopOptReportBuilder &) = delete;
  LoopOptReportBuilder &operator=(const LoopOptReportBuilder &) = delete;

public:
  LoopOptReportBuilder(LoopOptReportBuilder &&Arg)
      : Context(Arg.Context), OutputVerbosity(Arg.OutputVerbosity) {}

  LoopOptReportBuilder()
      : Context(nullptr), OutputVerbosity(OptReportVerbosity::None) {}

  void setup(LLVMContext &C, OptReportVerbosity::Level V) {
    Context = &C;
    OutputVerbosity = V;
  }

  LLVMContext &getContext() const {
    assert(Context && "Uninitialized LoopOptReportBuilder");
    return *Context;
  }

  OptReportVerbosity::Level getVerbosity() const {
    assert(Context && "Uninitialized LoopOptReportBuilder");
    return OutputVerbosity;
  }

  template <
    typename T, typename... Ts,
    typename X = typename std::enable_if<!std::is_const<T>::value>::type>
  LoopOptReportThunk<T> operator()(T &Arg, Ts &&... Args) const {
    assert(Context && "Uninitialized LoopOptReportBuilder");
    using ObjectHandleTy = typename LoopOptReportTraits<T>::ObjectHandleTy;
    return LoopOptReportThunk<T>(*this,
                                 ObjectHandleTy(Arg,
                                                std::forward<Ts>(Args)...));
  }

  bool isLoopOptReportOn() const { return getVerbosity() > 0; }
};

/// \brief Helper class specialized for particular object and verbosity level.
///
/// The class is not expected to be used by user directly. Instead, transient
/// objects of this class are created by LoopOptReportBuilder::operator().
///
/// It is a template class with most methods having a generic implementation.
/// However, a few methods cannot have generic implementation, and explicit
/// specializations should be provided for each supported type.
template <typename T> class LoopOptReportThunk {
  // Handle object must contain all the information required for
  // managing optimization reports for a particular "Loop" object,
  // i.e. it contains a reference of some sort to the "Loop" object.
  // Handle must also provide all auxiliary information needed
  // to configure the Builder for child "Loop" objects of the current
  // "Loop" object.
  using ObjectHandleTy = typename LoopOptReportTraits<T>::ObjectHandleTy;
  typename LoopOptReportTraits<T>::ObjectHandleTy Handle;
  const LoopOptReportBuilder &Builder;

public:
  template <typename... Ts>
  LoopOptReportThunk(const LoopOptReportBuilder &Builder, ObjectHandleTy Handle)
      : Handle(Handle), Builder(Builder) {}

  LoopOptReport getOptReport() const {
    return LoopOptReportTraits<T>::getOptReport(Handle);
  }

  void setOptReport(LoopOptReport OR) const {
    return LoopOptReportTraits<T>::setOptReport(Handle, OR);
  }

  void eraseOptReport() const {
    return LoopOptReportTraits<T>::eraseOptReport(Handle);
  }

  DebugLoc getDebugLoc() const {
    return LoopOptReportTraits<T>::getDebugLoc(Handle);
  }

  LoopOptReport getOrCreateOptReport() const {
    if (LoopOptReport OR = getOptReport())
      return OR;

    LoopOptReport NewOR =
        LoopOptReport::createEmptyOptReport(Builder.getContext());
    if (const DebugLoc &DL = getDebugLoc())
      NewOR.setDebugLoc(DL.get());
    setOptReport(NewOR);
    return NewOR;
  }

  // Return OptReport of previous sibling. Sibling's OptReport gets initialized
  // if necessary. If no sibling is found, nullptr is returned.
  LoopOptReport getOrCreatePrevOptReport() const {
    return LoopOptReportTraits<T>::getOrCreatePrevOptReport(Handle, Builder);
  }

  // Return OptReport for the parent. OptReport for the parent gets initialized
  // if necessary.
  LoopOptReport getOrCreateParentOptReport() const {
    return LoopOptReportTraits<T>::getOrCreateParentOptReport(Handle, Builder);
  }

  // Backward traversal of child loops (without recursion).
  template <typename F> void traverseChildLoopsBackward(F &&Func) const {
    LoopOptReportTraits<T>::traverseChildLoopsBackward(Handle,
                                                       std::forward<F>(Func));
  }

  template <typename... Args>
  LoopOptReportThunk<T> &addOrigin(Args &&... args) {
    if (!Builder.getVerbosity())
      return *this;

    LoopOptRemark Remark =
        LoopOptRemark::get(Builder.getContext(), std::forward<Args>(args)...);
    getOrCreateOptReport().addOrigin(Remark);
    return *this;
  }

  template <typename... Args>
  LoopOptReportThunk<T> &addRemark(OptReportVerbosity::Level MessageVerbosity,
                                   Args &&... args) {
    if (Builder.getVerbosity() < MessageVerbosity)
      return *this;

    LoopOptRemark Remark =
        LoopOptRemark::get(Builder.getContext(), std::forward<Args>(args)...);
    getOrCreateOptReport().addRemark(Remark);
    return *this;
  }

  LoopOptReportThunk<T> &addChild(LoopOptReport Child) {
    if (!Builder.getVerbosity())
      return *this;

    getOrCreateOptReport().addChild(Child);
    return *this;
  }

  LoopOptReportThunk<T> &addSibling(LoopOptReport Sibling) {
    if (!Builder.getVerbosity())
      return *this;

    getOrCreateOptReport().addSibling(Sibling);
    return *this;
  }

  // Preserve OptReport of a loop that is going to be removed.
  // The method returns void to disable chaining.
  void preserveLostLoopOptReport() const {
    if (!Builder.getVerbosity())
      return;

    // First, preserve OptReports of nested loops, if any. It is importand to
    // traverse child loops backwards, so that each loop is attached as a
    // sibling to the previous loop before the previous loop is "lost".
    // traverseChildLoopsBwd is not recursive by itself, but a lambda passed
    // into it makes indirect recursive call back to preserveLostLoopOptReport,
    // so that all child loops are processed before their parent loop.
    using ChildLoopTy = typename LoopOptReportTraits<T>::ChildLoopTy;
    using ChildHandleTy =
      typename LoopOptReportTraits<ChildLoopTy>::ObjectHandleTy;
    const LoopOptReportBuilder &B = Builder;
    traverseChildLoopsBackward(
        [&B](ChildHandleTy ChildHandle) {
          LoopOptReportThunk<ChildLoopTy>(B, ChildHandle)
              .preserveLostLoopOptReport();
        });

    // Even if there is no optreport yet, create one to report at least source
    // location of the lost loop.
    LoopOptReport OR = getOrCreateOptReport();

    // Attach optreport to the previous sibling (if there is one) or to the
    // parent.
    if (LoopOptReport Dest = getOrCreatePrevOptReport())
      Dest.addSibling(OR);
    else if (LoopOptReport Dest = getOrCreateParentOptReport())
      Dest.addChild(OR);
    else
      llvm_unreachable("Failed to find destination for a lost loop optreport");

    // Unlink OptReport from the loop.
    eraseOptReport();
  }

  // Moves OptReports from one loop to another.
  //
  // Some loops require auxiliary objects for the corresponding
  // LoopOptReportBuilder to be configured properly.  For example,
  // for LLVM Loop's there has to be LoopInfo available.  When
  // you move an OptReport to an LLVM Loop, you have to pass
  // LoopInfo as an additional parameter to moveOptReportTo().
  template <
      typename R, typename... AuxObjectTys,
      typename X = typename std::enable_if<!std::is_const<R>::value>::type>
  void moveOptReportTo(R &Other, AuxObjectTys &&... AuxObjects) const {
    if (!Builder.getVerbosity())
      return;

    const auto &ThunkObj =
        Builder(Other, std::forward<AuxObjectTys>(AuxObjects)...);
    assert(!ThunkObj.getOptReport() &&
           "Cannot override existing OptReport");
    ThunkObj.setOptReport(getOrCreateOptReport());
    eraseOptReport();
  }

  // Re-attaches sibling OptReports as siblings to another loop.
  // See moveOptReportTo() comment above for details on the auxiliary objects.
  template <
      typename R, typename... AuxObjectTys,
      typename X = typename std::enable_if<!std::is_const<R>::value>::type>
  void moveSiblingsTo(R &Other, AuxObjectTys &&... AuxObjects) const {
    if (!Builder.getVerbosity())
      return;

    LoopOptReport OR = getOptReport();
    if (!OR)
      return;

    LoopOptReport Sibling = OR.nextSibling();
    if (!Sibling)
      return;

    Builder(Other, std::forward<AuxObjectTys>(AuxObjects)...)
        .addSibling(Sibling);
    OR.eraseSiblings();
  }
};

// Traits of LLVM Function for LoopOptReportBuilder.
template <> struct LoopOptReportTraits<Function> {
  using ObjectHandleTy = Function &;

  static LoopOptReport getOptReport(const Function &F) {
    return cast_or_null<MDTuple>(F.getMetadata(LoopOptReportTag::Root));
  }

  static void setOptReport(Function &F, LoopOptReport OR) {
    assert(OR && "eraseOptReport method should be used to remove OptReport");
    F.setMetadata(LoopOptReportTag::Root, OR.get());
  }

  static void eraseOptReport(Function &F) {
    F.setMetadata(LoopOptReportTag::Root, nullptr);
  }

  static DebugLoc getDebugLoc(const Function &F) { return nullptr; }
};

// Traits of LLVM Loop for LoopOptReportBuilder.
template <> struct LoopOptReportTraits<Loop> {
  using ObjectHandleTy = std::pair<Loop &, LoopInfo &>;

  static LoopOptReport getOptReport(const ObjectHandleTy &Handle) {
    return LoopOptReport::findOptReportInLoopID(Handle.first.getLoopID());
  }

  static void setOptReport(const ObjectHandleTy &Handle, LoopOptReport OR) {
    auto &L = Handle.first;
    LLVMContext &C = L.getHeader()->getContext();
    L.setLoopID(LoopOptReport::addOptReportToLoopID(L.getLoopID(), OR, C));
  }

  static void eraseOptReport(const ObjectHandleTy &Handle) {
    auto &L = Handle.first;
    auto *OrigLoopID = L.getLoopID();
    auto *LoopID =
      LoopOptReport::eraseOptReportFromLoopID(OrigLoopID,
                                              L.getHeader()->getContext());

    if (LoopID)
      L.setLoopID(LoopID);
    else if (OrigLoopID) {
      // If OptReport was the only entry of the original LoopID,
      // then we have to erase the Loop's LoopID completely.
      // Unfortunately, there is no existing method for that,
      // so we create a dummy LoopID and assign it to the Loop.
      //
      // We do not want to set the dummy LoopID, if the original
      // LoopID was NULL, though.
      LoopID = MDTuple::get(L.getHeader()->getContext(), { nullptr });
      LoopID->replaceOperandWith(0, LoopID);
      L.setLoopID(LoopID);
    }
  }

  static DebugLoc getDebugLoc(const ObjectHandleTy &Handle) {
    return Handle.first.getLocRange().getStart();
  }

  static LoopOptReport getOrCreatePrevOptReport(
      const ObjectHandleTy &Handle, const LoopOptReportBuilder &Builder) {
    auto &L = Handle.first;
    Loop *PrevSiblingLoop = nullptr;

    if (L.getParentLoop())
      for (auto *ChildLoop : L.getParentLoop()->getSubLoops()) {
        if (ChildLoop == &L)
          break;

        PrevSiblingLoop = ChildLoop;
      }
    else {
      auto &LI = Handle.second;

      for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend();
           I != E; ++I) {
        if (*I == &L)
          break;

        PrevSiblingLoop = *I;
      }
    }

    if (!PrevSiblingLoop)
      return nullptr;

    return Builder(*PrevSiblingLoop, Handle.second).getOrCreateOptReport();
  }

  static LoopOptReport getOrCreateParentOptReport(
      const ObjectHandleTy &Handle, const LoopOptReportBuilder &Builder) {
    auto &L = Handle.first;
    // Attach to the parent Loop, if it exists.
    if (Loop *Dest = L.getParentLoop())
      return Builder(*Dest, Handle.second).getOrCreateOptReport();

    // Attach to the Function, otherwise.
    if (Function *Dest = L.getHeader()->getParent())
      return Builder(*Dest).getOrCreateOptReport();

    llvm_unreachable("Failed to find a parent.");
  }

  using ChildLoopTy = Loop;
  using ChildHandleTy = typename LoopOptReportTraits<ChildLoopTy>::ObjectHandleTy;
  using LoopVisitorTy = std::function<void(ChildHandleTy)>;
  static void traverseChildLoopsBackward(const ObjectHandleTy &Handle,
                                         LoopVisitorTy Func) {
    auto &L = Handle.first;
    std::for_each(L.rbegin(), L.rend(),
                  [&Handle, &Func](Loop *CL) {
                    Func(ChildHandleTy(*CL, Handle.second));
                  });
  }
};
} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
