; RUN: opt -opaque-pointers=0 -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' %s | FileCheck %s
;
; In this case, CFE has merged two cleanup handlers inside an OMP region,
; with a cleanup handler outside the OMP region.
;
; {
;   object o1; // needs destruction
;   // if func3() has an error, we need to destroy o1 and exit
;   try { } catch(...) { func3(); } // cleanup edge to cleanup_for_o1
;
;   #pragma omp parallel for
;   for (...) {
;     object o2;
;     // if func() has an error, we need to destroy o2 and o1 and exit.
;     try { } catch (...) { func(); }  // cleanup edge to cleanup_for_o2
;     try { } catch (...) { func2(); } // cleanup edge to cleanup_for_o2
;   }
;   // This cleanup code is inside the region, because it is only reachable
;   // inside the region.
;   cleanup_for_o2:
;     ~object(&o2);
;     // We also need to clean up o1! But this is actually undefined behavior
;     // as it is outside the region.
;     goto cleanup_for_o1;
;
;   // This cleanup code is reachable from inside and outside the region.
;   // We need to break the illegal region edge.
;   cleanup_for_o1:
;     ~object(&o1)
;
; The bug in this case, was that the o2 cleanup block was not dominated by
; any single EH pad. It is reachable from 2 different pads.

; CHECK: define{{.*}}split
; CHECK-LABEL: lpad.body.i.i.i
; CHECK: unreachable

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

$_ZN42gtest_case_ReductionTupleLocTestTargetOMP_17ReduceMaxLocTupleISt5tupleIJN4RAJA6policy3omp28omp_target_parallel_for_execILm256EEENS2_12ReduceMinLocINS4_17omp_target_reduceEdN4camp5tupleIJiiEEEEEEEE8TestBodyEv = comdat any

@.mapname.278 = private unnamed_addr constant [48 x i8] c";actualdata;test-reduce-tuplemaxloc.cpp;155;8;;\00", align 1
@.mapname.257 = private unnamed_addr constant [46 x i8] c";indirect;test-reduce-tuplemaxloc.cpp;156;8;;\00", align 1
declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare dso_local void @_ZN4RAJA17TypedRangeSegmentIllEC2Ell() unnamed_addr #1 align 2

declare dso_local void @_ZSt5beginIN4RAJA17TypedRangeSegmentIllEEEDTcldtfp_5beginEERT_() local_unnamed_addr #1

declare dso_local void @_ZN4camp10make_tupleIJiiEEENS_5tupleIJDpNS_8internal17unwrap_refwrapperINSt5decayIT_E4typeEE4typeEEEEDpOS5_() local_unnamed_addr #1

declare dso_local void @_ZN4RAJA12ReduceMaxLocINS_6policy3omp17omp_target_reduceEdN4camp5tupleIJiiEEEECI2NS_15TargetReduceLocINS_3omp6maxlocIdS6_EEdS6_EEEdS6_() unnamed_addr #1 align 2

define dso_local void @_ZN42gtest_case_ReductionTupleLocTestTargetOMP_17ReduceMaxLocTupleISt5tupleIJN4RAJA6policy3omp28omp_target_parallel_for_execILm256EEENS2_12ReduceMinLocINS4_17omp_target_reduceEdN4camp5tupleIJiiEEEEEEEE8TestBodyEv() unnamed_addr #1 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %dptr = alloca double*, align 8
  %tptr = alloca double**, align 8
  %ptr1 = alloca double, align 8
  %ptr2 = alloca double, align 8
  invoke void @_ZN4RAJA17TypedRangeSegmentIllEC2Ell()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  invoke void @_ZN4camp10make_tupleIJiiEEENS_5tupleIJDpNS_8internal17unwrap_refwrapperINSt5decayIT_E4typeEE4typeEEEEDpOS5_()
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  invoke void @_ZN4RAJA12ReduceMaxLocINS_6policy3omp17omp_target_reduceEdN4camp5tupleIJiiEEEECI2NS_15TargetReduceLocINS_3omp6maxlocIdS6_EEdS6_EEEdS6_()
          to label %DIR.OMP.TARGET.DATA.212 unwind label %lpad3

DIR.OMP.TARGET.DATA.212:                          ; preds = %invoke.cont4
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %DIR.OMP.TARGET.DATA.212
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM"(double* %ptr1, double* %ptr2, i64 0, i64 64, [48 x i8]* @.mapname.278, i8* null) ]
  br label %DIR.OMP.TARGET.DATA.3.split.split

DIR.OMP.TARGET.DATA.3.split.split:                ; preds = %DIR.OMP.TARGET.DATA.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM"(double** %dptr, double** %dptr, i64 0, i64 64, [46 x i8]* @.mapname.257, i8* null) ]
  invoke fastcc void @_ZZN42gtest_case_ReductionTupleLocTestTargetOMP_17ReduceMaxLocTupleISt5tupleIJN4RAJA6policy3omp28omp_target_parallel_for_execILm256EEENS2_12ReduceMinLocINS4_17omp_target_reduceEdN4camp5tupleIJiiEEEEEEEE8TestBodyEvENUliE_C2ERKSF_()
          to label %.noexc.i.i.i unwind label %lpad.i.i.i

.noexc.i.i.i:                                     ; preds = %DIR.OMP.TARGET.DATA.3.split.split
  invoke void @_ZSt5beginIN4RAJA17TypedRangeSegmentIllEEEDTcldtfp_5beginEERT_()
          to label %invoke.cont.i.i.i.i unwind label %lpad.i.i.i.i

invoke.cont.i.i.i.i:                              ; preds = %.noexc.i.i.i
  br label %invoke.cont2.i.i.i.i

invoke.cont2.i.i.i.i:                             ; preds = %invoke.cont.i.i.i.i
  br label %DIR.OMP.END.TARGET.DATA.18

lpad.i.i.i.i:                                     ; preds = %.noexc.i.i.i
  %2 = landingpad { i8*, i32 }
          cleanup
  br label %lpad.body.i.i.i

lpad.i.i.i:                                       ; preds = %DIR.OMP.TARGET.DATA.3.split.split
  %3 = landingpad { i8*, i32 }
          cleanup
  br label %lpad.body.i.i.i

; This is the merge point outside the region. (There's no "o1" destructor,
; it's been removed). lpad.body.i.i.i is the edge from inside the region that
; we need to delete.
common.resume:                                    ; preds = %lpad, %lpad.body.i.i.i
  resume { i8*, i32 } undef

; This is the merge point inside the region for both of the landingpads.
; It flows into the merge point outside the region.
lpad.body.i.i.i:                                  ; preds = %lpad.i.i.i, %lpad.i.i.i.i
  br label %common.resume

DIR.OMP.END.TARGET.DATA.18:                       ; preds = %invoke.cont2.i.i.i.i
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.19

DIR.OMP.END.TARGET.DATA.19:                       ; preds = %DIR.OMP.END.TARGET.DATA.18
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  unreachable

lpad:                                             ; preds = %entry
  %4 = landingpad { i8*, i32 }
          cleanup
  br label %common.resume

lpad3:                                            ; preds = %invoke.cont4, %invoke.cont
  %5 = landingpad { i8*, i32 }
          cleanup
  unreachable
}

declare dso_local fastcc void @_ZZN42gtest_case_ReductionTupleLocTestTargetOMP_17ReduceMaxLocTupleISt5tupleIJN4RAJA6policy3omp28omp_target_parallel_for_execILm256EEENS2_12ReduceMinLocINS4_17omp_target_reduceEdN4camp5tupleIJiiEEEEEEEE8TestBodyEvENUliE_C2ERKSF_() unnamed_addr #1 align 2

attributes #0 = { nounwind }
attributes #1 = { "unsafe-fp-math"="true" }
