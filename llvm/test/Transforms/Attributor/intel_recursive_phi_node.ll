; RUN: opt -passes=attributor-cgscc -S < %s | FileCheck %s

; This test case checks that the attributor isn't stuck in an infinite recursion
; because one of incoming values for the PHI node %e.i.i.0 (%e.i.i.1) is another
; PHI node where %e.i.i.0 is an incoming value. Also, it makes sure that
; instruction %e.i.i.0.vec.insert isn't simplified because it breaks the SSA
; form. This is the same test case as intel_recursive_phi_node.ll but it checks
; for opaque pointers.

; CHECK: define internal void @__omp_offloading_10309_c81353__Z1jv_l17() #0

; CHECK: %e.i.i.0.vec.insert = insertelement <2 x float> %e.i.i.0, float %conv.i.i, i32 0

; CHECK: declare ptr @llvm.stacksave.p0() #1

; CHECK: declare void @llvm.stackrestore.p0(ptr) #1

; CHECK: attributes #0 = { nofree norecurse noreturn nosync nounwind }
; CHECK: attributes #1 = { nocallback nofree nosync nounwind willreturn }

; ModuleID = 'intel_recursive_phi_node.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.anon = type { i32 }
%struct.__tgt_offload_entry.0 = type { ptr, ptr, i64, i32, i32 }

@l = external local_unnamed_addr global i32, align 4
@h = external local_unnamed_addr global %struct.anon, align 4
@.omp_offloading.entry_name = internal target_declare unnamed_addr constant [40 x i8] c"__omp_offloading_10309_c81353__Z1jv_l17\00"
@.omp_offloading.entry.__omp_offloading_10309_c81353__Z1jv_l17 = weak target_declare constant %struct.__tgt_offload_entry.0 { ptr @__omp_offloading_10309_c81353__Z1jv_l17, ptr @.omp_offloading.entry_name, i64 0, i32 0, i32 0 }, section "omp_offloading_entries"

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #0

define internal void @__omp_offloading_10309_c81353__Z1jv_l17() {
newFuncRoot:
  br label %for.cond

for.cond:                                         ; preds = %_Z1iv.exit, %newFuncRoot
  %e.i.i.0 = phi <2 x float> [ undef, %newFuncRoot ], [ %e.i.i.1, %_Z1iv.exit ]
  %savedstack = call ptr @llvm.stacksave()
  %0 = load i32, ptr @l, align 4
  %tobool.not.i = icmp eq i32 %0, 0
  br i1 %tobool.not.i, label %_Z1iv.exit, label %if.then.i

if.then.i:                                        ; preds = %for.cond
  %1 = load i32, ptr @h, align 4
  %conv.i.i = sitofp i32 %1 to float
  %e.i.i.0.vec.insert = insertelement <2 x float> %e.i.i.0, float %conv.i.i, i32 0
  %e.sroa.0.0.vec.extract.i.i.i.i = extractelement <2 x float> %e.i.i.0.vec.insert, i64 0
  %conv.i.i.i.i = fptosi float %e.sroa.0.0.vec.extract.i.i.i.i to i32
  %tobool.not.i.i = icmp eq i32 %conv.i.i.i.i, 0
  br i1 %tobool.not.i.i, label %if.end.i.i, label %_Z1mv.exit.i

if.end.i.i:                                       ; preds = %if.then.i
  br label %_Z1mv.exit.i

_Z1mv.exit.i:                                     ; preds = %if.end.i.i, %if.then.i
  br label %_Z1iv.exit

_Z1iv.exit:                                       ; preds = %_Z1mv.exit.i, %for.cond
  %e.i.i.1 = phi <2 x float> [ %e.i.i.0, %for.cond ], [ %e.i.i.0.vec.insert, %_Z1mv.exit.i ]
  call void @llvm.stackrestore(ptr %savedstack)
  br label %for.cond, !llvm.loop !0

DIR.OMP.END.TARGET.4:                             ; No predecessors!
  unreachable
}

attributes #0 = { nocallback nofree nosync nounwind willreturn }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.mustprogress"}
