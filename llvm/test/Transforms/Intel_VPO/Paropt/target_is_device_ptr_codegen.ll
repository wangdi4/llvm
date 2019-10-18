; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s
;
; It tests the OMP code generation for is_device_ptr clause.
; extern double hh,gg;
; int *a;
; void foo(int n) {
;   #pragma omp target firstprivate(n)  map(from: gg) is_device_ptr(a)
;   for (int i = 0; i < n; ++i) {
;     gg = i*i+hh;
;     a++;
;   }
; }

; There must be 4 mapped entries: hh, n, gg, a
; Verify that their MAP TYPEs are correct
; FPRIV(hh):    33= 0x21= TGT_MAP_TO | TGT_MAP_TARGET_PARAM
; FPRIV(n):     33= 0x21= TGT_MAP_TO | TGT_MAP_TARGET_PARAM
; MAPFROM(gg):  34= 0x22= TGT_MAP_FROM | TGT_MAP_TARGET_PARAM
; IsDevPtr(a):  33= 0x21= TGT_MAP_TO | TGT_MAP_TARGET_PARAM
; CHECK: @.offload_maptypes = private unnamed_addr constant [4 x i64] [i64 34, i64 33, i64 33, i64 33]
;
; Verify that the offload entry has the 4 matching arguments
; CHECK: call void @__omp_offloading{{.*}}(double* %gg, i32* %n.addr, double* %hh, i32** %a)

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-mic"

@gg = external dso_local global double, align 8
@a = common dso_local global i32* null, align 8
@hh = external dso_local global double, align 8
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4, !tbaa !2
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr), "QUAL.OMP.MAP.FROM"(double* @gg), "QUAL.OMP.IS_DEVICE_PTR"(i32** @a), "QUAL.OMP.FIRSTPRIVATE"(double* @hh), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  store i32 0, i32* %i, align 4, !tbaa !2
  br label %for.cond

for.cond:                                         ; preds = %for.body, %DIR.OMP.TARGET.1
  %2 = load i32, i32* %i, align 4, !tbaa !2
  %3 = load i32, i32* %n.addr, align 4, !tbaa !2
  %cmp = icmp slt i32 %2, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  %mul = mul nsw i32 %2, %2
  %conv = sitofp i32 %mul to double
  %4 = load double, double* @hh, align 8, !tbaa !6
  %add = fadd double %conv, %4
  store double %add, double* @gg, align 8, !tbaa !6
  %5 = load i32*, i32** @a, align 8, !tbaa !8
  %incdec.ptr = getelementptr inbounds i32, i32* %5, i32 1
  store i32* %incdec.ptr, i32** @a, align 8, !tbaa !8
  %6 = load i32, i32* %i, align 4, !tbaa !2
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %i, align 4, !tbaa !2
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!omp_offload.info = !{!10}
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !4, i64 0}
!10 = !{i32 0, i32 54, i32 -698850821, !"foo", i32 31, i32 0, i32 0}
