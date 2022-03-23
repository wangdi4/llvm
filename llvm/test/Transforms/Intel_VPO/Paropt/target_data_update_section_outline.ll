; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; Original code:
; extern void modify_array_on_target();
;
; void update(int *a) {
;   modify_array_on_target();
; #pragma omp target update to(a[13:17])
; #pragma omp target map(to: a[13:17])
;   {
;     a[15]++;
;   }
; }

; CHECK-DAG: @[[SIZE1:.+]] = {{.*}} constant {{.*}} [i64 68]
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 1]
; CHECK: define dso_local void @_Z6updatePi(i32* %[[A:[^)]+]])
; CHECK-DAG: %[[A_SEC_PTR:.+]] = getelementptr inbounds i32, i32* %[[A]], i64 13
; CHECK-DAG: %[[A_BPTR_CAST:.+]] = bitcast i32* %[[A]] to i8*
; CHECK-DAG: store i8* %[[A_BPTR_CAST]]
; CHECK-DAG: %[[A_SPTR_CAST:.+]] = bitcast i32* %[[A_SEC_PTR]] to i8*
; CHECK-DAG: store i8* %[[A_SPTR_CAST]]
; Verify that the descriptor for 'a' is passed to __tgt_target_data_update
; CHECK-DAG: call void @__tgt_target_data_update(i64 %{{.*}}, i32 1, {{.*}}@[[SIZE1]]{{.*}}@[[MAPTYPE1]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: uwtable mustprogress
define dso_local void @_Z6updatePi(i32* %a) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %a.map.ptr.tmp = alloca i32*, align 8
  store i32* %a, i32** %a.addr, align 8, !tbaa !4
  call void @_Z22modify_array_on_targetv()
  %0 = load i32*, i32** %a.addr, align 8
  %1 = load i32*, i32** %a.addr, align 8, !tbaa !4
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 13
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(), "QUAL.OMP.MAP.TO"(i32* %0, i32* %arrayidx, i64 68, i64 1, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  %3 = load i32*, i32** %a.addr, align 8, !tbaa !4
  %4 = load i32*, i32** %a.addr, align 8
  %5 = load i32*, i32** %a.addr, align 8, !tbaa !4
  %arrayidx1 = getelementptr inbounds i32, i32* %5, i64 13
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(i32* %4, i32* %arrayidx1, i64 68, i64 33, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32** %a.map.ptr.tmp) ]
  store i32* %4, i32** %a.map.ptr.tmp, align 8
  %7 = load i32*, i32** %a.map.ptr.tmp, align 8, !tbaa !4
  %arrayidx2 = getelementptr inbounds i32, i32* %7, i64 15
  %8 = load i32, i32* %arrayidx2, align 4, !tbaa !8
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %arrayidx2, align 4, !tbaa !8
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @_Z22modify_array_on_targetv() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #2

attributes #0 = { uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 66313, i32 12590453, !"_Z6updatePi", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 12.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSPi", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
