//==--- FPGAAnalyzeChannelsUsage.cpp -                         -*- C++ -*---==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file contains the implementation of several diagnostic messages
/// related to the OpenCL FPGA channels feature.
///
/// Documentation of these features can be found at:
///   https://www.intel.com/content/altera-www/global/en_us/index/documentation/mwh1391807965224.html
///
// ===--------------------------------------------------------------------=== //

#include "clang/Sema/intel/FPGAAnalyzeChannelsUsage.h"

#include "ConstCallExprVisitor.h"

#include "clang/AST/Decl.h"
#include "clang/AST/Redeclarable.h"
#include "clang/Sema/SemaInternal.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include <algorithm> // std::find
#include <string>

///
/// \detail
///
/// According to spec, kernel can read from the same channel multiple times, but
/// multiple kernels cannot read from the same channel. The same rule applies to
/// write operations with channel.
///
/// Examples:
///
/// \anchor simplest-example
///
/// channel int a;
/// __kernel void test1() {
///   write_channel_intel(a, 10);
/// }
/// __kernel void test2() {
///   write_channel_intel(a, 20); // error!
/// }
///
/// ----
///
/// \anchor helper-functions
///
/// channel int a;
/// void helper(int v) {
///   write_channel_intel(a, v * 2);
/// }
/// __kernel void test1() {
///   write_channel_intel(a, 10);
/// }
/// __kernel void test2() {
///   helper(15); // error!
/// }
///
/// ----
///
/// \anchor channel-by-value-helper-functions
///
/// channel int a;
/// void write_helper(channel int ch) {
///   write_channel_intel(ch, 10);
/// }
/// __kernel void test1() {
///   write_helper(a);
/// }
/// __kernel void test2() {
///   write_helper(a); // error!
/// }
///
/// ----
///
/// \anchor call-kernel
///
/// channel int a;
/// __kernel void test1() {
///   write_channel_intel(a, 10);
/// }
/// __kernel void test2() {
///   test1(); // error!
/// }
///
/// Basic idea of the analysis: for each channel find all kernels which directly
/// or indirectly use the channel and emit diagnostics if more than one kernel
/// uses the same channel.
///
/// So, for the \ref simplest-example an implementation could look like:
/// for each FunctionDecl:
///   1.1 for each channel BI call:
///     - build channel descriptor
///   1.2 if FunctionDecl is an OpenCL kernel
///     - for each built channel descriptor:
///       - check if this descriptor is already used in any other kernel and
///         emit diagnostic if necessary
///
/// But, the code user written can be more complex: as you can see in examples,
/// kernels can access channels via user-defined helper functions. Also, it is
/// legal to call one kernel from another kernel and pass channels as arguments
/// to user-defined functions.
///
/// Let's increase complexity of the example and look how algorithm evolves step
/// by step.
///
/// To support helper functions (\ref helper-functions) we need to take in
/// account the whole call graph:
/// for each FunctionDecl:
///   1.1 for each channel BI call:
///     - build channel descriptor
///   1.2 for each user function call:
///     - repeat steps 1.1 - 1.2 recursively
///   1.3 if FunctionDecl is an OpenCL kernel
///     - for each built channel descriptor:
///       - check if this descriptor is already used in any other kernel and
///         emit diagnostic if necessary
///
/// There is another "obstacle" with clang: functions are parsed in order of
/// occurrence in the source file. Using forward declarations user can shuffle
/// order of the function definitions in a source file. So, we cannot guarantee
/// that all functions called directly or indirectly from the current one are
/// parsed and can be analyzed.
///
/// There are two general cases which algorithm should be able to handle:
/// 1. Callee was analyzed before the caller
/// 2. Caller was analyzed before the callee
///
/// Algorithm now looks like:
/// for each FunctionDecl:
///   1.1 for each channel BI call:
///     - build channel descriptor
///   1.2 for each user function call:
///     - repeat steps 1.1 - 1.2 recursively
///   1.3 for each function in (current FunctionDecl and callers of current FD):
///     - check if it is an OpenCL kernel
///       - for each built channel descriptor:
///         - check if this descriptor is already used in any other kernel and
///           emit diagnostic if necessary
///
/// Here is an example:
///
/// channel int a;
///
/// void helper();
/// void helper2();
/// __kernel void kernel1() {
///   helper();
/// }
/// __kernel void kernel2() {
///   helper2();
/// }
/// void helper2() {
///   write_channel_intel(a, 10);
/// }
/// void helper() {
///   helper2();
/// }
///
/// How it is handled by the compiler:
/// 1. kernel1: we cannot build any channel descriptors because there is no
///    direct usages of channels in the kernel and 'helper' function is not
///    parsed yet
/// 2. kernel2: the same as 'kernel1'
/// 3. helper2: we build channel descriptor: (a, write). No calls to user
///    functions, but 'helper2' was called from 'kernel2': now algorithm should
///    know that 'kernel2' uses channel 'a' for writing
/// 4. helper: No direct usage of channels. 'helper2' is called by 'helper' and
///    already analyzed. Now algorithm should know that 'helper' uses channel
///    'a' for writing. Also, 'helper' is called by 'kernel1': now algorithm
///    should know that channel 'a' is used for writing there. But channel 'a'
///    is already used for writing in 'kernel2' - algorithm emits error.
///
/// To avoid re-analyzing each FunctionDecl, the following map is created:
/// FuncitonDecl -> set of channel descriptors which directly or indirectly used
/// from the certain FunctionDecl. This allows us avoid recursion and storing
/// information about the call graph is not needed anymore, because we always
/// need to check only direct callees to get all channels which indirectly used
/// from the current FunctionDecl being analyzed.
///
/// Algorithm now looks like:
/// for each FunctionDecl:
///   1.1 for each channel BI call:
///     - build channel descriptor
///   1.2 for each user function call:
///     - if the function is already analyzed
///       - add all channel descriptors used in callee to list of channel
///         descriptors used in current FunctionDecl
///   1.3 for each function in (current FunctionDecl and callers of current FD):
///     - check if it is an OpenCL kernel
///       - for each built channel descriptor:
///         - check if this descriptor is already used in any other kernel and
///           emit diagnostic if necessary
///
/// To simplify walking up from callee to caller Sema is extended with one new
/// field: reverse call graph. It contains the following mapping:
/// Callee -> Callers.
///
/// Call graph is not needed for analysis, but it is exists anyway, because it
/// is used to display call stack in diagnostic messages (see tests for
/// examples).
///
/// Both of call graphs are expanded here, in this file. It could be done in
/// Sema (like it is done for CUDA), but I decided to reduce number of changes
/// in files created by the community.
///
/// Channel-by-value support
///
/// Algorithm is extended with new type of channel descriptors: local
/// descriptors which represents channel arguments in helper functions. Such
/// descriptors are used to store information about how each channel argument
/// is used: for reading or writing.
///
/// Here is some examples:
///
/// \anchor channel-by-value-callee-before-caller
///
/// channel int a;
/// void helper(channel int ch, int data) {
///   write_channel_intel(ch, data);
/// }
/// void helper2(int data, channel int ch) {
///   helper(ch, data);
/// }
/// __kernel void test_a(int data) {
///   helper2(data, a);
/// }
///
/// How it is handled by the compiler:
/// 1. helper: channel 'ch' is used for writing, build descriptor
///    ('helper-0', write)
/// 2. helper2: channel 'ch' is used as argument #0 of 'helper'. Let
///    \c mode = how argument #0 is used inside the 'helper' function, i.e.
///    write in this example. Algorithm builds new descriptor:
///    ('helper2-1', write)
/// 3. test_a: channel 'a' is used as argument #1 of 'helper2'. Let
///    \c mode = how argument #1 is used inside the 'helper2' function, i.e.
///    write in this example. Algorithm builds new descriptor:
///    ('a', write)
///
/// And another one example:
///
/// \anchor channel-by-value-caller-before-callee
///
/// channel int a;
/// void helper(channel int ch, int data);
/// void helper2(int data, channel int ch);
/// __kernel void test_a(int data) {
///   helper2(data, a);
/// }
/// void helper2(int data, channel int ch) {
///   helper(ch, data);
/// }
/// void helper(channel int ch, int data) {
///   write_channel_intel(ch, data);
/// }
///
/// How it is handled by the compiler:
/// 1. test_a: channel 'a' is used as argument #1 of 'helper2'. But 'helper2' is
///    not analyzed yet and we cannot build a descriptor because mode is unknown
/// 2. helper2: channel 'ch' is used as argument #0 of 'helper'. But 'helper' is
///    not analyzed yet and we cannot build a descriptor because mode is unknown
/// 1. helper: channel 'ch' is used for writing, build descriptor
///    ('helper-0', write).
///    For each caller of helper and its callers:
///     - helper is called from 'helper2'. Let \c name = name of channel
///       descriptor which is passed to 'helper' from 'helper2', i.e.
///       'helper2-1' in this example. Let \c mode = write (value from recently
///       built descriptor). Add descriptor ('helper2-1', write) to list of
///       channels used in helper2
///     - helper2 is called from 'test_a'. Let \c name = name of channel
///       descriptor which is passed to 'helper2' from 'test_a', i.e. 'a' in
///       this example. Let \c mode = how argument #1 is used in 'helper2', i.e.
///       write in this example. add descriptor ('a', write) to list of
///       channels used in test_a
///

namespace {

using namespace clang;

/// Extracts indices which were used to access an element of an array by
/// unwrapping ArraySubscriptExpr AST nodes if such exists.
///
/// \param [in, out] CE If there is no array access at all it is not changed.
///                  Otherwise CE->getSubExpr() points to a DeclRefExpr
/// \param [in, out] Indices Contains indices which were used to access an
///                  element of an array in reverse order (due to AST
///                  representation details)
/// \param [in] Context ASTContext used to evaluate constant expressions
///
/// \returns false if is any of indices is not compile-time known.
///          \arg \c Indices will be incomplete in that case
/// \returns true if there is no array access at all or all indices were
///          extracted successfully
bool ExtractArraySubscriptIndices(const CastExpr *&CE,
                                  llvm::SmallVectorImpl<int64_t> &Indices,
                                  ASTContext &Context) {
  while (const auto *SubscriptExpr =
             dyn_cast<ArraySubscriptExpr>(CE->getSubExpr())) {
    Expr::EvalResult ERes;
    if (!SubscriptExpr->getIdx()->EvaluateAsInt(ERes, Context)) {
      return false;
    }
    if (!ERes.Val.isInt()) {
      return false;
    }

    Indices.push_back(ERes.Val.getInt().getExtValue());
    CE = cast<CastExpr>(SubscriptExpr->getBase());
  }

  return true;
}

/// Tries to construct channel descriptor by extracting the exact ValueDecl
/// which represents a channel.
///
/// \param [in] CE CallExpr from which to extract channel descriptor
/// \param [in] ArgNo Argument number which is expected to be a channel
/// \param [in] Kind Describes Kind which is set to built channel descriptor
/// \param [in, out] Desc Contains constructed channel descriptor
/// \param [in] CurFD FunctionDecl where \arg \c CE is located
///
/// \returns true if channel descriptor were constructed successfully, false
///          otherwise
bool BuildChannelDescriptor(const CallExpr *CE, unsigned ArgNo,
                            FPGAChannelAccessKind Kind,
                            FPGAChannelDescriptor &Desc,
                            const FunctionDecl *CurFD) {
  // I expect ImplicitCastExpr(LValueToRValue) here
  const auto *Cast = cast<CastExpr>(CE->getArg(ArgNo));
  llvm::SmallVector<int64_t, 4> Indices;
  if (!ExtractArraySubscriptIndices(Cast, Indices, CurFD->getASTContext()))
    return false;

  // TODO: this if should be an assert: support access to channels via pointer
  // arithmetic must be diagnosed earlier as error
  // channel uint c[2][3];
  // read_channel_nb_intel( *(*c), &valid);
  const auto *DeclRef = dyn_cast<DeclRefExpr>(Cast->getSubExpr());
  if (!DeclRef)
    return false;
  const ValueDecl *ChannelDecl = DeclRef->getDecl();

  Desc.Kind = Kind;

  if (const auto *PVD = dyn_cast<ParmVarDecl>(ChannelDecl)) {
    Desc.ArgNo = PVD->getFunctionScopeIndex();
    Desc.IsLocalChannel = true;
    Desc.Name = CurFD->getName().str() + "-" + std::to_string(Desc.ArgNo);

    return true;
  }

  Desc.ChannelDecl = ChannelDecl;
  Desc.Name = Desc.ChannelDecl->getName().str();
  for (const auto &I : llvm::reverse(Indices))
    Desc.Name += "[" + std::to_string(I) + "]";

  return true;
}

FPGAChannelAccessKind GetChannelAccessKind(StringRef Name) {
  if ("read_channel_intel" == Name || "read_channel_nb_intel" == Name) {
    return FPGAChannelAccessKind::Read;
  }
  if ("write_channel_intel" == Name || "write_channel_nb_intel" == Name) {
    return FPGAChannelAccessKind::Write;
  }

  return FPGAChannelAccessKind::Undefined;
}

bool HasChannelArgs(const FunctionDecl *FD) {
  for (const ParmVarDecl *PVD : FD->parameters()) {
    if (PVD->getType()->isChannelType())
      return true;
  }

  return false;
}

/// Helper structure to maintain state of BFS while collecting call stack
/// of channel usage
struct State {
  const FunctionDecl *FD = nullptr;
  // Distance (number of edges) between the root node (passed to
  // CollectCallStack, should be an OpenCL Kernel function) and the current node
  // (FD)
  unsigned Depth = 0;
  SourceLocation SLoc;

  State() = default;

  State(const FunctionDecl *FD, unsigned Depth) : FD(FD), Depth(Depth) {}

  State(const FunctionDecl *FD, unsigned Depth, SourceLocation SLoc)
      : FD(FD), Depth(Depth), SLoc(SLoc) {}
};

void CollectCallStack(const FPGAChannelDescriptor &Desc,
                      const FunctionDecl *Kernel, Sema &S,
                      llvm::SmallVectorImpl<SourceLocation> &CallStack) {
  FPGAChannelDescriptor TargetDesc = Desc;
  State CurrentState;
  llvm::SmallVector<State, 8> WorkList;
  llvm::SmallPtrSet<const FunctionDecl *, 16> Visited;
  CallStack.clear();

  WorkList.emplace_back(State(Kernel, /* Depth = */ 0));
  Visited.insert(Kernel);

  bool GlobalChannelFound = false;
  while (!WorkList.empty()) {
    CurrentState = WorkList.back();
    WorkList.pop_back();

    // Maintain current call stack
    if (CurrentState.Depth > CallStack.size()) {
      // We need to push another one item of call stack
      CallStack.push_back(CurrentState.SLoc);
      assert(CallStack.size() == CurrentState.Depth && "Unexpected depth");
    } else {
      // We failed to find a usage of the requested channel in some sub-tree
      // and we are going to walk-up in call graph and start looking into
      // another sub-tree.
      //
      // For example:
      //  Call graph:
      //    k1 calls f2 and f3
      //    f3 calls f5
      //    f2 calls f4 and f5
      //  Current state:
      //    Started from k1, algorithm pushed into WorkList f2 and f3. Then f3
      //    are handled and then f5. Usage of requested channel not found yet.
      //    WorkList contains: [f2]
      //    Call Stack contains: [call to f3, call to f5]
      //    The current step is to analyze f2. At this point we need to remove
      //    some elements from call stack (all in this example) to get it in a
      //    right state.
      CallStack.resize(CurrentState.Depth);
    }

    // Analyze current FunctionDecl to search for requested channel in directly
    // used channels
    const FunctionDecl *FD = CurrentState.FD;
    Visited.insert(FD);

    GlobalChannelFound = false;
    for (const Sema::FunctionDeclAndCallSitesPair &It :
         S.OCLFPGACallGraph[FD]) {
      bool LocalChannelFound = false;
      const FunctionDecl *Callee = It.first;
      FPGAChannelAccessKind Kind = GetChannelAccessKind(Callee->getName());
      if (FPGAChannelAccessKind::Undefined != Kind) {
        // channel BI call
        for (const CallExpr *CE : It.second) {
          FPGAChannelDescriptor TempDesc;
          if (!BuildChannelDescriptor(CE, 0, Kind, TempDesc, FD))
            continue;
          if (TargetDesc != TempDesc)
            continue;

          GlobalChannelFound = true;
          CallStack.push_back(CE->getBeginLoc());
          break;
        } // for CallExpr
      } else {
        // everything else
        if (!HasChannelArgs(Callee)) {
          bool NotVisitedYet = Visited.insert(Callee).second;
          if (NotVisitedYet)
            WorkList.push_back(State(Callee, CurrentState.Depth + 1,
                                     It.second.front()->getBeginLoc()));
          continue;
        }

        for (const FPGAChannelDescriptor &TempDesc :
             S.FunctionToChannelMap[Callee]) {
          if (!TempDesc.IsLocalChannel)
            continue;

          for (const CallExpr *CE : It.second) {
            FPGAChannelDescriptor NewDesc;
            if (!BuildChannelDescriptor(CE, TempDesc.ArgNo, TempDesc.Kind,
                                        NewDesc, FD))
              continue;
            if (TargetDesc != NewDesc)
              continue;

            LocalChannelFound = true;
            TargetDesc = TempDesc;
            WorkList.clear();
            WorkList.emplace_back(
                State(Callee, CurrentState.Depth + 1, CE->getBeginLoc()));
            break;
          } // for CallExpr

          if (LocalChannelFound)
            break;
        } // for Desc
      }

      if (GlobalChannelFound || LocalChannelFound)
        break;
    } // for Callee

    if (GlobalChannelFound)
      break; // stop traverse over call graph
  }

  // CallStack is expected to be correct when do { ... } while (...) loop
  // finished
  assert(GlobalChannelFound && "Failed to collect call stack");
}

enum ChannelUsageDiagHelper : int { Previous = 0, Current = 1 };

void emitDiags(const FunctionDecl *CurKernel,
               Sema::ChannelDescSet &ChannelsUsedInDecl, Sema &S) {
  for (const auto &CurDesc : ChannelsUsedInDecl) {
    unsigned Kind = static_cast<unsigned>(CurDesc.Kind);

    // Check for I/O channels usage
    if (CurKernel->hasAttr<AutorunAttr>() &&
        CurDesc.ChannelDecl->hasAttr<OpenCLIOAttr>()) {
      S.Diag(CurDesc.ChannelDecl->getBeginLoc(),
             diag::warn_io_channel_is_used_from_autorun_kernel)
          << CurDesc.Name << CurKernel->getName();
      SmallVector<SourceLocation, 8> CurCallStack;
      CollectCallStack(CurDesc, CurKernel, S, CurCallStack);
      S.Diag(CurKernel->getBeginLoc(), diag::note_channel_is_used_through)
          << CurDesc.Name << Kind;
      for (const auto &SLoc : CurCallStack)
        S.Diag(SLoc, diag::note_channel_is_used_through)
            << CurDesc.Name << Kind;
    }

    const FunctionDecl *&PrevKernel = S.ChannelToKernelMap[CurDesc];
    if (!PrevKernel) {
      PrevKernel = CurKernel;
      continue;
    }

    // More than one use of channel
    S.Diag(CurDesc.ChannelDecl->getBeginLoc(),
           diag::warn_channel_is_used_from_more_than_one_kernel) << Kind;
    // Show current usage of a channel to user
    S.Diag(CurKernel->getBeginLoc(),
           diag::note_channel_usage_already_found_in_kernel)
        << Current << CurDesc.Name << Kind << CurKernel->getName();
    SmallVector<SourceLocation, 8> CallStack;
    CollectCallStack(CurDesc, CurKernel, S, CallStack);
    for (const auto &SLoc : CallStack)
      S.Diag(SLoc, diag::note_channel_is_used_through) << CurDesc.Name << Kind;

    // Show previous usage of a channel to user
    S.Diag(PrevKernel->getBeginLoc(),
           diag::note_channel_usage_already_found_in_kernel)
        << Previous << CurDesc.Name << Kind << PrevKernel->getName();
    CollectCallStack(CurDesc, PrevKernel, S, CallStack);
    for (const auto &SLoc : CallStack)
      S.Diag(SLoc, diag::note_channel_is_used_through) << CurDesc.Name << Kind;
  } // for Desc
}

} // namespace

namespace clang {

// 1. For each FunctionDecl
void launchOCLFPGAFeaturesAnalysis(const Decl *D, Sema &S) {
  if (!isa<FunctionDecl>(D) || !D->hasBody())
    return;

  const auto *FD = cast<FunctionDecl>(D);
  // Some preparations: collect all CallExpr's for further analysis
  ConstCallExprVisitor Visitor;
  Visitor.TraverseFunctionDecl(FD);
  Sema::ChannelDescSet ChannelsUsedInDecl;

  Sema::FunctionDeclToCallSitesMap &CalledByFD = S.OCLFPGACallGraph[FD];
  for (const CallExpr *CE : Visitor.GetCalls()) {
    const FunctionDecl *Callee = CE->getDirectCallee();
    CalledByFD[Callee].push_back(CE);
    S.OCLFPGAReverseCallGraph[Callee][FD].push_back(CE);

    FPGAChannelAccessKind Kind = GetChannelAccessKind(Callee->getName());
    if (FPGAChannelAccessKind::Undefined != Kind) {
      // Step 1.1 of the algorithm: collect all channels used directly from the
      // current FunctionDecl
      FPGAChannelDescriptor Desc;
      // CE is a call to channel BI function. For all such functions channel
      // argument is at first position, i.e. 0
      if (BuildChannelDescriptor(CE, 0, Kind, Desc, FD))
        ChannelsUsedInDecl.insert(Desc);
      continue;
    }

    // Step 1.2 of the algorithm: extend list of directly used channels with
    // lists of indirectly used channels (via calls to helper functions and
    // other kernels)
    const Sema::ChannelDescSet &ChannelsUsedInCallee =
        S.FunctionToChannelMap[Callee];
    for (const FPGAChannelDescriptor &Desc : ChannelsUsedInCallee) {
      if (!Desc.IsLocalChannel) {
        ChannelsUsedInDecl.insert(Desc);
      } else {
        FPGAChannelDescriptor NewDesc;
        if (BuildChannelDescriptor(CE, Desc.ArgNo, Desc.Kind, NewDesc, FD))
          ChannelsUsedInDecl.insert(NewDesc);
      }
    }
  }

  if (ChannelsUsedInDecl.empty())
    return;

  S.FunctionToChannelMap[FD].insert(ChannelsUsedInDecl.begin(),
                                    ChannelsUsedInDecl.end());

  // Step 1.3 of the algorithm: walk up in call graph and emit diagnostics if
  // necessary
  llvm::SmallVector<State, 16> WorkList;
  WorkList.push_back(State(FD, /* Depth = */ 0));
  // Used to detect recursion
  llvm::SmallVector<const FunctionDecl *, 8> Stack;

  Sema::FunctionToChannelMapType NewChannels;
  NewChannels[FD].insert(ChannelsUsedInDecl.begin(), ChannelsUsedInDecl.end());

  while (!WorkList.empty()) {
    State CurrentState = WorkList.back();
    WorkList.pop_back();
    const FunctionDecl *CurFD = CurrentState.FD;

    if (CurrentState.Depth > Stack.size()) {
      if (llvm::find(Stack, FD) != Stack.end())
        break; // recursion detected
      Stack.push_back(CurFD);
      assert(Stack.size() == CurrentState.Depth && "Unexpected depth");
    } else {
      Stack.resize(CurrentState.Depth);
    }

    Sema::ChannelDescSet &ChannelsToPromote = NewChannels[CurFD];
    if (ChannelsToPromote.empty())
      continue;

    for (const Sema::FunctionDeclAndCallSitesPair &Node :
         S.OCLFPGAReverseCallGraph[CurFD]) {
      const FunctionDecl *Caller = Node.first;
      Sema::ChannelDescSet &NewChannelsForCaller = NewChannels[Caller];
      WorkList.emplace_back(State(Caller, CurrentState.Depth + 1));

      for (const FPGAChannelDescriptor &Desc : ChannelsToPromote) {
        if (!Desc.IsLocalChannel) {
          NewChannelsForCaller.insert(Desc);
        } else {
          for (const CallExpr *Call : Node.second) {
            FPGAChannelDescriptor NewDesc;
            if (BuildChannelDescriptor(Call, Desc.ArgNo, Desc.Kind, NewDesc,
                                       Caller)) {
              NewChannelsForCaller.insert(NewDesc);
            }
          }
        }
      }
      S.FunctionToChannelMap[Caller].insert(NewChannelsForCaller.begin(),
                                            NewChannelsForCaller.end());
    } // for Node in call graph

    if (!CurFD->hasAttr<OpenCLKernelAttr>())
      continue;

    emitDiags(CurFD, ChannelsToPromote, S);
    ChannelsToPromote.clear();
  }
}

} // namespace clang
