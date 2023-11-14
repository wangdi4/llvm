//===------ Intel_OptReportBuilder.h --------------------------*- C++ -*---===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines OptReportBuilder class. That is the main interface of
// optimization reports for transformations.
//
// For more details on how to use transformation reports, please look at:
//     docs/Intel/OptReport.rst
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
#define LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_OptReport/Diag.h"
#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DebugLoc.h"

#include <type_traits>
#include <utility>

namespace llvm {

template <typename T> class OptReportThunk;
template <typename T> struct OptReportTraits;

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
///   OptReportBuilder ORBuilder(...);
///   ORBuilder(Lp).addRemark(MyFancyRemark);
///
/// The key part of the OptReportBuilder API is operator(), which creates an
/// instance of OptReportThunk for object Lp with convenient interface for
/// OptReport manipulations.
class OptReportBuilder {
  LLVMContext *Context;
  OptReportVerbosity::Level OutputVerbosity;

  OptReportBuilder(const OptReportBuilder &) = delete;
  OptReportBuilder &operator=(const OptReportBuilder &) = delete;

public:
  OptReportBuilder(OptReportBuilder &&Arg)
      : Context(Arg.Context), OutputVerbosity(Arg.OutputVerbosity) {}

  OptReportBuilder()
      : Context(nullptr), OutputVerbosity(OptReportVerbosity::None) {}

  void setup(LLVMContext &C, OptReportVerbosity::Level V) {
    Context = &C;
    OutputVerbosity = V;
  }

  LLVMContext &getContext() const {
    assert(Context && "Uninitialized OptReportBuilder");
    return *Context;
  }

  OptReportVerbosity::Level getVerbosity() const {
    assert(Context && "Uninitialized OptReportBuilder");
    return OutputVerbosity;
  }

  template <
      typename T, typename... Ts,
      typename X = typename std::enable_if<!std::is_const<T>::value>::type>
  OptReportThunk<T> operator()(T &Arg, Ts &&...Args) const {
    assert(Context && "Uninitialized OptReportBuilder");
    using ObjectHandleTy = typename OptReportTraits<T>::ObjectHandleTy;
    return OptReportThunk<T>(*this,
                             ObjectHandleTy(Arg, std::forward<Ts>(Args)...));
  }

  bool isOptReportOn() const { return getVerbosity() > 0; }
};

/// \brief Helper class specialized for particular object and verbosity level.
///
/// The class is not expected to be used by user directly. Instead, transient
/// objects of this class are created by OptReportBuilder::operator().
///
/// It is a template class with most methods having a generic implementation.
/// However, a few methods cannot have generic implementation, and explicit
/// specializations should be provided for each supported type.
template <typename T> class OptReportThunk {
  // Handle object must contain all the information required for
  // managing optimization reports for a particular "Loop" object,
  // i.e. it contains a reference of some sort to the "Loop" object.
  // Handle must also provide all auxiliary information needed
  // to configure the Builder for child "Loop" objects of the current
  // "Loop" object.
  using ObjectHandleTy = typename OptReportTraits<T>::ObjectHandleTy;
  typename OptReportTraits<T>::ObjectHandleTy Handle;
  const OptReportBuilder &Builder;

public:
  template <typename... Ts>
  OptReportThunk(const OptReportBuilder &Builder, ObjectHandleTy Handle)
      : Handle(Handle), Builder(Builder) {}

  OptReport getOptReport() const {
    return OptReportTraits<T>::getOptReport(Handle);
  }

  void setOptReport(OptReport OR) const {
    return OptReportTraits<T>::setOptReport(Handle, OR);
  }

  void eraseOptReport() const {
    return OptReportTraits<T>::eraseOptReport(Handle);
  }

  DebugLoc getDebugLoc() const {
    return OptReportTraits<T>::getDebugLoc(Handle);
  }

  OptReport getOrCreateOptReport() const {
    if (OptReport OR = getOptReport())
      return OR;

    OptReport NewOR = OptReport::createEmptyOptReport(Builder.getContext());
    if (const DebugLoc &DL = getDebugLoc())
      NewOR.setDebugLoc(DL.get());
    if (std::optional<std::string> Title =
            OptReportTraits<T>::getOptReportTitle(Handle))
      NewOR.setTitle(*Title);
    setOptReport(NewOR);
    return NewOR;
  }

  // Return OptReport of previous sibling. Sibling's OptReport gets initialized
  // if necessary. If no sibling is found, nullptr is returned.
  OptReport getOrCreatePrevOptReport() const {
    return OptReportTraits<T>::getOrCreatePrevOptReport(Handle, Builder);
  }

  // Return OptReport for the parent. OptReport for the parent gets initialized
  // if necessary.
  OptReport getOrCreateParentOptReport() const {
    return OptReportTraits<T>::getOrCreateParentOptReport(Handle, Builder);
  }

  // Backward traversal of child loops (without recursion).
  template <typename F> void traverseChildNodesBackward(F &&Func) const {
    OptReportTraits<T>::traverseChildNodesBackward(Handle,
                                                   std::forward<F>(Func));
  }

  // Interface to add origin remarks using given remark ID.
  template <typename... Args>
  OptReportThunk<T> &addOrigin(OptRemarkID RemarkID, Args &&...args) {
    if (!Builder.getVerbosity())
      return *this;

    assert(RemarkID != OptRemarkID::InvalidRemarkID && "Remark ID is invalid!");
    OptRemark Remark = OptRemark::get(Builder.getContext(), RemarkID,
                                      std::forward<Args>(args)...);
    getOrCreateOptReport().addOrigin(Remark);
    return *this;
  }

  // Interface to add opt-report remarks using given remark ID.
  template <typename... Args>
  OptReportThunk<T> &addRemark(OptReportVerbosity::Level MessageVerbosity,
                               OptRemarkID RemarkID, Args &&...args) {
    if (Builder.getVerbosity() < MessageVerbosity)
      return *this;

    assert(RemarkID != OptRemarkID::InvalidRemarkID && "Remark ID is invalid!");
    OptRemark Remark = OptRemark::get(Builder.getContext(), RemarkID,
                                      std::forward<Args>(args)...);
    getOrCreateOptReport().addRemark(Remark);
    return *this;
  }

  // Interface to add pre-built opt-report remark.
  OptReportThunk<T> &addRemark(OptReportVerbosity::Level MessageVerbosity,
                               OptRemark Remark) {
    if (Builder.getVerbosity() < MessageVerbosity)
      return *this;

    getOrCreateOptReport().addRemark(Remark);
    return *this;
  }

  OptReportThunk<T> &addChild(OptReport Child) {
    if (!Builder.getVerbosity())
      return *this;

    getOrCreateOptReport().addChild(Child);
    return *this;
  }

  OptReportThunk<T> &addSibling(OptReport Sibling) {
    if (!Builder.getVerbosity())
      return *this;

    getOrCreateOptReport().addSibling(Sibling);
    return *this;
  }

  // Preserve OptReport of a loop that is going to be removed.
  // The method returns void to disable chaining.
  void preserveLostOptReport() const {
    if (!Builder.getVerbosity())
      return;

    // First, preserve OptReports of nested loops, if any. It is important to
    // traverse child loops backwards, so that each loop is attached as a
    // sibling to the previous loop before the previous loop is "lost".
    // traverseChildLoopsBwd is not recursive by itself, but a lambda passed
    // into it makes indirect recursive call back to preserveLostOptReport,
    // so that all child loops are processed before their parent loop.
    using ChildNodeTy = typename OptReportTraits<T>::ChildNodeTy;
    using ChildHandleTy = typename OptReportTraits<ChildNodeTy>::ObjectHandleTy;
    const OptReportBuilder &B = Builder;
    traverseChildNodesBackward([&B](ChildHandleTy ChildHandle) {
      OptReportThunk<ChildNodeTy>(B, ChildHandle).preserveLostOptReport();
    });

    // Even if there is no optreport yet, create one to report at least source
    // location of the lost loop.
    OptReport OR = getOrCreateOptReport();

    // Attach optreport to the previous sibling (if there is one) or to the
    // parent.
    if (OptReport Dest = getOrCreatePrevOptReport())
      Dest.addSibling(OR);
    else if (OptReport Dest = getOrCreateParentOptReport())
      Dest.addChild(OR);
    else
      llvm_unreachable("Failed to find destination for a lost loop optreport");

    // Unlink OptReport from the loop.
    eraseOptReport();
  }

  // Moves OptReports from one loop to another.
  //
  // Some loops require auxiliary objects for the corresponding
  // OptReportBuilder to be configured properly.  For example,
  // for LLVM Loop's there has to be LoopInfo available.  When
  // you move an OptReport to an LLVM Loop, you have to pass
  // LoopInfo as an additional parameter to moveOptReportTo().
  template <
      typename R, typename... AuxObjectTys,
      typename X = typename std::enable_if<!std::is_const<R>::value>::type>
  void moveOptReportTo(R &Other, AuxObjectTys &&...AuxObjects) const {
    if (!Builder.getVerbosity())
      return;

    const auto &ThunkObj =
        Builder(Other, std::forward<AuxObjectTys>(AuxObjects)...);
    assert(!ThunkObj.getOptReport() && "Cannot override existing OptReport");
    ThunkObj.setOptReport(getOrCreateOptReport());
    eraseOptReport();
  }

  // Re-attaches sibling OptReports as siblings to another loop.
  // See moveOptReportTo() comment above for details on the auxiliary objects.
  template <
      typename R, typename... AuxObjectTys,
      typename X = typename std::enable_if<!std::is_const<R>::value>::type>
  void moveSiblingsTo(R &Other, AuxObjectTys &&...AuxObjects) const {
    if (!Builder.getVerbosity())
      return;

    OptReport OR = getOptReport();
    if (!OR)
      return;

    OptReport Sibling = OR.nextSibling();
    if (!Sibling)
      return;

    Builder(Other, std::forward<AuxObjectTys>(AuxObjects)...)
        .addSibling(Sibling);
    OR.eraseSiblings();
  }

  // Copy OptReport's children to another OptReport.
  template <
      typename R, typename... AuxObjectTys,
      typename X = typename std::enable_if<!std::is_const<R>::value>::type>
  void copyChildrenTo(R &Other, AuxObjectTys &&...AuxObjects) const {
    if (!Builder.getVerbosity())
      return;

    OptReport OR = getOptReport();
    if (!OR)
      return;

    for (OptReport Child = OR.firstChild(); Child; Child = Child.nextSibling())
      Builder(Other, std::forward<AuxObjectTys>(AuxObjects)...)
          .addChild(Child.copy());
  }
};

// Traits of LLVM Function for OptReportBuilder.
template <> struct OptReportTraits<Function> {
  using ObjectHandleTy = Function &;

  static OptReport getOptReport(const Function &F) {
    return cast_or_null<MDTuple>(F.getMetadata(OptReportTag::Report));
  }

  static void setOptReport(Function &F, OptReport OR) {
    assert(OR && "eraseOptReport method should be used to remove OptReport");
    F.setMetadata(OptReportTag::Report, OR.get());
  }

  static void eraseOptReport(Function &F) {
    F.setMetadata(OptReportTag::Report, nullptr);
  }

  static DebugLoc getDebugLoc(const Function &F) { return nullptr; }

  static std::optional<std::string> getOptReportTitle(const Function &F) {
    return std::string("FUNCTION REPORT");
  }
};

// Traits of LLVM Loop for OptReportBuilder.
template <> struct OptReportTraits<Loop> {
  using ObjectHandleTy = std::pair<Loop &, LoopInfo &>;

  static OptReport getOptReport(const ObjectHandleTy &Handle) {
    return OptReport::findOptReportInLoopID(Handle.first.getLoopID());
  }

  static void setOptReport(const ObjectHandleTy &Handle, OptReport OR) {
    auto &L = Handle.first;
    LLVMContext &C = L.getHeader()->getContext();
    L.setLoopID(OptReport::addOptReportToLoopID(L.getLoopID(), OR, C));
  }

  static void eraseOptReport(const ObjectHandleTy &Handle) {
    auto &L = Handle.first;
    auto *OrigLoopID = L.getLoopID();
    auto *LoopID = OptReport::eraseOptReportFromLoopID(
        OrigLoopID, L.getHeader()->getContext());

    if (LoopID)
      L.setLoopID(LoopID);
    else if (OrigLoopID)
      // If OptReport was the only entry of the original LoopID,
      // then we have to erase the Loop's LoopID completely.
      L.eraseLoopID();
  }

  static DebugLoc getDebugLoc(const ObjectHandleTy &Handle) {
    return Handle.first.getLocRange().getStart();
  }

  static std::optional<std::string> getOptReportTitle(const ObjectHandleTy &Handle) {
    return std::nullopt;
  }

  static OptReport getOrCreatePrevOptReport(const ObjectHandleTy &Handle,
                                            const OptReportBuilder &Builder) {
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

      for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend(); I != E;
           ++I) {
        if (*I == &L)
          break;

        PrevSiblingLoop = *I;
      }
    }

    if (!PrevSiblingLoop)
      return nullptr;

    return Builder(*PrevSiblingLoop, Handle.second).getOrCreateOptReport();
  }

  static OptReport getOrCreateParentOptReport(const ObjectHandleTy &Handle,
                                              const OptReportBuilder &Builder) {
    auto &L = Handle.first;
    // Attach to the parent Loop, if it exists.
    if (Loop *Dest = L.getParentLoop())
      return Builder(*Dest, Handle.second).getOrCreateOptReport();

    // Attach to the Function, otherwise.
    if (Function *Dest = L.getHeader()->getParent())
      return Builder(*Dest).getOrCreateOptReport();

    llvm_unreachable("Failed to find a parent.");
  }

  using ChildNodeTy = Loop;
  using ChildHandleTy = typename OptReportTraits<ChildNodeTy>::ObjectHandleTy;
  using NodeVisitorTy = std::function<void(ChildHandleTy)>;
  static void traverseChildNodesBackward(const ObjectHandleTy &Handle,
                                         NodeVisitorTy Func) {
    auto &L = Handle.first;
    std::for_each(L.rbegin(), L.rend(), [&Handle, &Func](Loop *CL) {
      Func(ChildHandleTy(*CL, Handle.second));
    });
  }
};

// Copy opt-report metadata from one function to another. Opt-report from
// the 'From' function is appended to the 'To' function.
inline void copyOptReport(Function &From, Function &To) {
  OptReportBuilder ORBuilder;
  // Opt-report verbosity level is not important here, we can use any
  // value >= Low just to copy opt-report metadata from one function to
  // another.
  ORBuilder.setup(To.getContext(), OptReportVerbosity::Low);
  if (ORBuilder(From).getOptReport())
    ORBuilder(From).copyChildrenTo(To);
}

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
