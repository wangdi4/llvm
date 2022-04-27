; RUN: opt -vpo-cfg-restructuring -vpo-paropt -tbaa %s | opt -disable-output -scoped-noalias-aa -aa-eval -evaluate-aa-metadata -print-no-aliases 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -aa-pipeline=tbaa %s | opt -disable-output -scoped-noalias-aa -aa-eval -evaluate-aa-metadata -print-no-aliases 2>&1 | FileCheck %s

; TODO: TBAA only appears to be available during vpo-paropt with the new pass
; manager. As a result, the old pass manager RUN line is disabled.

; Check that AA information attached to instructions, such as TBAA, is
; preserved via ScopedNoAlias metadata after outlining.

; // The IR below corresponds the following C function.
; // Notably, in C int/double pointers such as x/y cannot alias, and this rule
; // effected by the TBAA metadata.
; int foo(int *x, double *y) {
; 	int r = 0;
;
; #pragma omp target map(tofrom:r)
; 	{
; 		*x = 1;
; 		*y = 2.0;
; 		r = *x; // there should be no load here
; 	}
;
; 	return r;
; }

; Ensure that the aliasing information was preserved by evaluating all pairwise
; queries and looking for the "NoAlias" result between the x/y stores.
; Note that TBAA metadata is still present at this point, but not used; only
; the ScopedNoAlias metadata is used for the "-aa-eval" run.
;
; CHECK-LABEL: Function: __omp_offloading
; CHECK: NoAlias:   store double 2.000000e+00, double* %y
; CHECK-SAME:       store i32 1, i32* %x

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32* %x, double* %y) local_unnamed_addr #0 {
entry:
  %r = alloca i32, align 4
  %x.map.ptr.tmp = alloca i32*, align 8
  %y.map.ptr.tmp = alloca double*, align 8
  %0 = bitcast i32* %r to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %r, align 4, !tbaa !3

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %x, i32* %x, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(double* %y, double* %y, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32* %r, i32* %r, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32** %x.map.ptr.tmp), "QUAL.OMP.PRIVATE"(double** %y.map.ptr.tmp) ]
  store i32 1, i32* %x, align 4, !tbaa !3
  store double 2.000000e+00, double* %y, align 8, !tbaa !7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 1
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 64, i32 -1781690476, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
