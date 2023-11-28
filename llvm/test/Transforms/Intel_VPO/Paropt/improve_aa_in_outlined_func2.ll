; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -aa-pipeline=tbaa %s | opt -disable-output -aa-pipeline="scoped-noalias-aa" -passes="aa-eval" -evaluate-aa-metadata -print-no-aliases 2>&1 | FileCheck %s

; TODO: TBAA only appears to be available during vpo-paropt with the new pass
; manager. As a result, the old pass manager RUN line is disabled.

; Check that AA information attached to instructions, such as TBAA, is
; preserved via ScopedNoAlias metadata after outlining.

; // The IR below corresponds the following C function.
; // Notably, in C int/double pointers such as x/y cannot alias, and this rule
; // effected by the TBAA metadata.

; Test src:
;
; int foo(int *x, double *y) {
;   int r = 0;
;
; #pragma omp target map(tofrom : r)
;   {
;     *x = 1;
;     *y = 2.0;
;     r = *x; // there should be no load here
;   }
;
;   return r;
; }

; Ensure that the aliasing information was preserved by evaluating all pairwise
; queries and looking for the "NoAlias" result between the x/y stores.
; Note that TBAA metadata is still present at this point, but not used; only
; the ScopedNoAlias metadata is used for the "-aa-eval" run.

; CHECK-LABEL: Function: __omp_offloading
; CHECK: NoAlias:   store double 2.000000e+00, ptr %y
; CHECK-SAME:       store i32 1, ptr %x

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local i32 @foo(ptr %x, ptr %y) local_unnamed_addr {
entry:
  %r = alloca i32, align 4
  %x.map.ptr.tmp = alloca ptr, align 8
  %y.map.ptr.tmp = alloca ptr, align 8
  %0 = bitcast ptr %r to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %0)
  store i32 0, ptr %r, align 4, !tbaa !2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr %y, ptr %y, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr %r, ptr %r, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y.map.ptr.tmp, ptr null, i32 1) ]
  store i32 1, ptr %x, align 4, !tbaa !2
  store double 2.000000e+00, ptr %y, align 8, !tbaa !6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %0)
  ret i32 1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 64, i32 -1781690476, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
