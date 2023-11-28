; RUN: opt -passes=vpo-paropt -S %s | FileCheck %s

; C++ code is something like this:
; try {
;   foo(); // invoke, may throw
;   #pragma omp parallel
;     #pragma omp single
;       ...
; }
; catch (...) {
;   ...
; }
;
; #single becomes an invoke to __kmpc_single and __kmpc_end_single. The
; lpads for all 3 invokes are merged together. This causes the region finder
; to add the entire subgraph starting at "lpad.i" (the catch block) to the
; region.
; The EH fixup algorithm must replace the "invoke __kmpc_single" with calls.
; Then all the catch blocks must be removed from the region.
; As a complication, we do not want to break the unwind edge from foo() to
; the catch, or the edges coming out of the catch. The blocks that have been
; mistakenly added to the region, should not be modified.

; CHECK-NOT: unreachable

; CHECK-LABEL: @foo
; CHECK: invoke{{.*}}_ZN1i2at2auC2Ec
; CHECK: invoke{{.*}}_ZN1i2at2btEv
; CHECK: invoke{{.*}}_ZN1i2at2auC2Ec
; CHECK: invoke{{.*}}_ZN1i2at2auC2Ec
; CHECK: invoke{{.*}}__cxa_end_catch
; this next one was already a call
; CHECK: call{{.*}}__cxa_end_catch

; CHECK: define{{.*}}DIR.OMP.PARALLEL
; CHECK: call{{.*}}__kmpc_single
; CHECK: call{{.*}}__kmpc_end_single
; CHECK: call{{.*}}__kmpc_barrier

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%"struct.i::at::ax" = type { i8 }
%"struct.i::at::aw" = type { i8 }

$_ZTSN1i2at2blE = comdat any

$_ZTIN1i2at2blE = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTSN1i2at2blE = linkonce_odr dso_local constant [11 x i8] c"N1i2at2blE\00", comdat, align 1
@_ZTIN1i2at2blE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTSN1i2at2blE }, comdat, align 8
@"@tid.addr" = external local_unnamed_addr global i32
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0 }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0.1 }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.4 = private unnamed_addr global %struct.ident_t { i32 0, i32 838861122, i32 0, i32 0, ptr @.source.0.0.3 }

; Function Attrs: uwtable
define dso_local void @foo() local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
entry:
  %agg.tmp.ensured = alloca %"struct.i::at::ax", align 1
  %end.dir.temp = alloca i1, align 1
  %agg.tmp14.i.i = alloca %"struct.i::at::ax", align 1
  %ref.tmp = alloca %"struct.i::at::aw", align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %ref.tmp) #5
  invoke void @_ZN1i2at2auC2Ec(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp, i8 noundef signext 7)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  invoke void @_ZN1i2at2btEv()
          to label %.noexc unwind label %lpad

.noexc:                                           ; preds = %invoke.cont
  call void @llvm.lifetime.start.p0(i64 1, ptr %agg.tmp14.i.i)
  invoke void @_ZN1i2at2auC2Ec(ptr noundef nonnull align 1 dereferenceable(1) %agg.tmp14.i.i, i8 noundef signext 4)
          to label %.noexc7 unwind label %lpad

.noexc7:                                          ; preds = %.noexc
  call void @llvm.lifetime.start.p0(i64 1, ptr %agg.tmp.ensured)
  call void @llvm.lifetime.start.p0(i64 1, ptr %end.dir.temp)
  invoke void @_ZN1i2at2auC2Ec(ptr noundef nonnull align 1 dereferenceable(1) %agg.tmp.ensured, i8 noundef signext 4)
          to label %.noexc.i unwind label %lpad.i

.noexc.i:                         ; preds = %.noexc7
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %.noexc.i
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
 "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %_ZN1i2at2ad2czclINS0_2axENS0_2boENS0_2bfIS5_NS0_2bdEEEEEbT_T1_NS_2agIT0_Li2EEESD_.exit.i, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:       ; preds = %DIR.OMP.PARALLEL.2
  %my.tid = load i32, ptr @"@tid.addr", align 4
  %1 = invoke i32 @__kmpc_single(ptr nonnull @.kmpc_loc.0.0, i32 %my.tid)
          to label %.noexc1.i unwind label %lpad.i

.noexc1.i:                        ; preds = %DIR.OMP.PARALLEL.3
  %2 = icmp eq i32 %1, 1
  br i1 %2, label %if.then.single.8, label %DIR.OMP.END.SINGLE.5.split

if.then.single.8:         ; preds = %.noexc1.i
  fence acquire
  fence release
  %my.tid6 = load i32, ptr @"@tid.addr", align 4
  invoke void @__kmpc_end_single(ptr nonnull @.kmpc_loc.0.0.2, i32 %my.tid6)
          to label %DIR.OMP.END.SINGLE.5.split unwind label %lpad.i

DIR.OMP.END.SINGLE.5.split: ; preds = %if.then.single.8, %.noexc1.i
  %my.tid7 = load i32, ptr @"@tid.addr", align 4
  invoke void @__kmpc_barrier(ptr nonnull @.kmpc_loc.0.0.4, i32 %my.tid7)
          to label %_ZN1i2at2ad2czclINS0_2axENS0_2boENS0_2bfIS5_NS0_2bdEEEEEbT_T1_NS_2agIT0_Li2EEESD_.exit.i unwind label %lpad.i

_ZN1i2at2ad2czclINS0_2axENS0_2boENS0_2bfIS5_NS0_2bdEEEEEbT_T1_NS_2agIT0_Li2EEESD_.exit.i: ; preds = %DIR.OMP.END.SINGLE.5.split, %DIR.OMP.PARALLEL.2
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %_ZN1i2at2ad2czclINS0_2axENS0_2boENS0_2bfIS5_NS0_2bdEEEEEbT_T1_NS_2agIT0_Li2EEESD_.exit.i
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  call void @llvm.lifetime.end.p0(i64 1, ptr %agg.tmp.ensured)
  call void @llvm.lifetime.end.p0(i64 1, ptr %end.dir.temp)
  br label %_ZN1i2at2djINS0_1oEEENS0_2bfINS_1nENS0_2bdEEET_NS0_2auE.exit

lpad.i:                           ; preds = %DIR.OMP.END.SINGLE.5.split, %if.then.single.8, %DIR.OMP.PARALLEL.3, %.noexc7
  %3 = landingpad { ptr, i32 }
          cleanup
          catch ptr null
          catch ptr @_ZTIN1i2at2blE
  %4 = extractvalue { ptr, i32 } %3, 0
  %5 = call ptr @__cxa_begin_catch(ptr %4) #5
  invoke void @__cxa_end_catch()
          to label %_ZN1i2at2djINS0_1oEEENS0_2bfINS_1nENS0_2bdEEET_NS0_2auE.exit unwind label %lpad

_ZN1i2at2djINS0_1oEEENS0_2bfINS_1nENS0_2bdEEET_NS0_2auE.exit: ; preds = %lpad.i, %DIR.OMP.END.PARALLEL.4
  call void @llvm.lifetime.end.p0(i64 1, ptr %agg.tmp14.i.i)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %ref.tmp) #5
  br label %try.cont

lpad:                                             ; preds = %lpad.i, %.noexc, %invoke.cont, %entry
  %6 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN1i2at2blE
  %7 = extractvalue { ptr, i32 } %6, 1
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %ref.tmp) #5
  %8 = call i32 @llvm.eh.typeid.for(ptr nonnull @_ZTIN1i2at2blE) #5
  %matches = icmp eq i32 %7, %8
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad
  %9 = extractvalue { ptr, i32 } %6, 0
  %10 = call ptr @__cxa_begin_catch(ptr %9) #5
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %_ZN1i2at2djINS0_1oEEENS0_2bfINS_1nENS0_2bdEEET_NS0_2auE.exit
  ret void

eh.resume:                                        ; preds = %lpad
  resume { ptr, i32 } %6
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #2

; Function Attrs: nofree
declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr #3

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #3

declare dso_local void @_ZN1i2at2btEv() local_unnamed_addr 

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #5

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #5

declare dso_local void @_ZN1i2at2auC2Ec(ptr noundef nonnull align 1 dereferenceable(1), i8 noundef signext) unnamed_addr 

declare i32 @__kmpc_single(ptr, i32) local_unnamed_addr

declare void @__kmpc_end_single(ptr, i32) local_unnamed_addr

declare void @__kmpc_barrier(ptr, i32) local_unnamed_addr

attributes #0 = { uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { nofree }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
