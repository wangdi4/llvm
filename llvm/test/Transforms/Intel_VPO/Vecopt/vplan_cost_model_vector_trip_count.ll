; RUN: opt < %s -disable-output -passes=vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s --check-prefix=CHECK-IR

; RUN: opt < %s -disable-output -passes=vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 -vplan-force-uf=3 | FileCheck %s --check-prefix=CHECK-IR-UF3

; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s --check-prefix=CHECK-HIR

; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 -vplan-force-uf=3 | FileCheck %s --check-prefix=CHECK-HIR-UF3

; The test verifies the cost of vector-trip-count instruction in various
; scenarios.

; CHECK-IR: Cost 0 for i32 %vp{{[0-9]+}} = vector-trip-count i32 128, UF = 1
; CHECK-IR: Cost 1 for i32 %vp{{[0-9]+}} = vector-trip-count i32 %tc, UF = 1
; CHECK-IR-UF3: Cost 0 for i32 %vp{{[0-9]+}} = vector-trip-count i32 128, UF = 1
; CHECK-IR-UF3: Cost 4 for i32 %vp{{[0-9]+}} = vector-trip-count i32 %tc, UF = 1
; CHECK-HIR: Cost 0 for i32 %vp{{[0-9]+}} = vector-trip-count i32 128, UF = 1
; CHECK-HIR: Cost 1 for i32 %vp{{[0-9]+}} = vector-trip-count i32 %vp{{[0-9]+}}, UF = 1
; CHECK-HIR-UF3: Cost 0 for i32 %vp{{[0-9]+}} = vector-trip-count i32 128, UF = 1
; CHECK-HIR-UF3: Cost 4 for i32 %vp{{[0-9]+}} = vector-trip-count i32 %vp{{[0-9]+}}, UF = 1

@arr = external local_unnamed_addr global [128 x i32], align 16

define void @foo(i32 %tc) {
entry1:
  %tok1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for1.body

for1.body:
  %i1.merge = phi i32 [ 0, %entry1 ], [ %i1.next, %for1.body ]
  %idx1 = getelementptr inbounds [128 x i32], ptr @arr, i32 0, i32 %i1.merge
  %ld1 = load i32, ptr %idx1
  %r1 = add nuw nsw i32 %ld1, 1
  store i32 %ld1, ptr %idx1

  %i1.next = add nuw nsw i32 %i1.merge, 1
  %exitcond1 = icmp eq i32 %i1.next, 128
  br i1 %exitcond1, label %for1.exit, label %for1.body

for1.exit:
  call void @llvm.directive.region.exit(token %tok1) [ "DIR.OMP.END.SIMD"()]
  br label %entry2
  
entry2:
  %tok2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for2.body

for2.body:
  %i2.merge = phi i32 [ 0, %entry2 ], [ %i2.next, %for2.body ]
  %idx2 = getelementptr inbounds [128 x i32], ptr @arr, i32 0, i32 %i2.merge
  %ld2 = load i32, ptr %idx2
  %r2 = add nuw nsw i32 %ld2, 1
  store i32 %ld2, ptr %idx2

  %i2.next = add nuw nsw i32 %i2.merge, 1
  %exitcond2 = icmp eq i32 %i2.next, %tc
  br i1 %exitcond2, label %for2.exit, label %for2.body

for2.exit:
  call void @llvm.directive.region.exit(token %tok2) [ "DIR.OMP.END.SIMD"()]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
