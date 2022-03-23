; RUN: opt -O3 -paropt=31 -disable-output -pass-remarks-analysis=openmp %s 2>&1 | FileCheck %s
; RUN: opt -passes='default<O3>' -paropt=31 -disable-output -pass-remarks-analysis=openmp %s 2>&1 | FileCheck %s
;
; Test src:
;
; struct S {
;   int h, i, j;
;   int ag, ah;
; };
;
; extern void foo(S bbox);
;
; void bar(const S &bbox) {
;   foo(bbox);
; }
;
; void test() {
;   S bbox;
; #pragma omp target
;   bar(bbox);
; }

; Check that shared privatization pass can determine that 'bbox' can be passed
; to the target region as firstprivate.
;
; CHECK: remark: {{.+}} MAP:TOFROM clause for variable 'bbox' on 'target' construct can be changed to FIRSTPRIVATE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32, i32, i32, i32, i32 }

; Function Attrs: mustprogress uwtable
define dso_local void @_Z3barRK1S(%struct.S* nonnull align 4 dereferenceable(20) %bbox) #0 {
entry:
  %bbox.addr = alloca %struct.S*, align 8
  %agg.tmp = alloca %struct.S, align 8
  store %struct.S* %bbox, %struct.S** %bbox.addr, align 8
  %0 = load %struct.S*, %struct.S** %bbox.addr, align 8
  %1 = bitcast %struct.S* %agg.tmp to i8*
  %2 = bitcast %struct.S* %0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 %2, i64 20, i1 false)
  call void @_Z3foo1S(%struct.S* byval(%struct.S) align 8 %agg.tmp)
  ret void
}

declare dso_local void @_Z3foo1S(%struct.S* byval(%struct.S) align 8) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z4testv() #3 {
entry:
  %bbox = alloca %struct.S, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%struct.S* %bbox, %struct.S* %bbox, i64 20, i64 547, i8* null, i8* null) ]
  call void @_Z3barRK1S(%struct.S* nonnull align 4 dereferenceable(20) %bbox) #4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { mustprogress nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -676547204, !"_Z4testv", i32 14, i32 0, i32 0}
