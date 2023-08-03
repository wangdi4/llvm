; Test that caller/callee simd function parameter matching is done correctly
; with respect to caller/callee alignment. This means, for each param:
; - If caller/callee are unaligned/unaligned, the param matches
; - If caller/callee are   aligned/unaligned, the param matches
; - If caller/callee are   aligned/  aligned, the param matches if
;                                             caller align >= callee align
; - If caller/callee are unaligned/  aligned, the param matches if
;                                             caller ABI align >= callee align

; Test uniform and linear parameters, for the latter testing that alignment is
; computed taking the active VF into account.

; RUN: opt -S < %s -vplan-force-vf=2 -passes="vpo-paropt,vplan-vec" | FileCheck %s --check-prefix=VF2
; RUN: opt -S < %s -vplan-force-vf=4 -passes="vpo-paropt,vplan-vec" | FileCheck %s --check-prefix=VF4
; RUN: opt -S < %s -vplan-force-vf=8 -passes="vpo-paropt,vplan-vec" | FileCheck %s --check-prefix=VF8
; RUN: opt -S < %s -vplan-force-vf=2 -passes="vpo-paropt,hir-ssa-deconstruction,hir-vplan-vec,hir-cg" | FileCheck %s --check-prefix=VF2
; RUN: opt -S < %s -vplan-force-vf=4 -passes="vpo-paropt,hir-ssa-deconstruction,hir-vplan-vec,hir-cg" | FileCheck %s --check-prefix=VF4
; RUN: opt -S < %s -vplan-force-vf=8 -passes="vpo-paropt,hir-ssa-deconstruction,hir-vplan-vec,hir-cg" | FileCheck %s --check-prefix=VF8

target triple = "x86_64-unknown-linux-gnu"

declare void @unalign(ptr) #0
declare void @align16(ptr) #1
declare void @both16(ptr) #2

define dso_local void @foo(ptr %p, ptr %p.a4, ptr %p.a16) local_unnamed_addr #3 {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.ALIGNED"(ptr %p.a4, i32 4), "QUAL.OMP.ALIGNED"(ptr %p.a16, i32 16) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add, %omp.inner.for.body ]
  %gep.iv = getelementptr i32, ptr %p, i64 %.iv
  %gep4.iv = getelementptr i32, ptr %p.a4, i64 %.iv
  %gep16.iv = getelementptr i32, ptr %p.a16, i64 %.iv

; Unaligned -> Unaligned (always matches)
  call void @unalign(ptr %p)
; VF2: call void @_ZGVbN2u_unalign
; VF4: call void @_ZGVbN4u_unalign
; VF8: call void @_ZGVbN8u_unalign
  call void @unalign(ptr %gep.iv)
; VF2: call void @_ZGVbN2l4_unalign
; VF4: call void @_ZGVbN4l4_unalign
; VF8: call void @_ZGVbN8l4_unalign

; Unaligned -> Aligned (no match: alignment is insufficient)
  call void @align16(ptr %p)
; VF2-COUNT-2: call void @align16
; VF4-COUNT-4: call void @align16
; VF8-COUNT-8: call void @align16
  call void @align16(ptr %gep.iv)
; VF2-COUNT-2: call void @align16
; VF4-COUNT-4: call void @align16
; VF8-COUNT-8: call void @align16

; Aligned -> Unaligned (always matches)
  call void @unalign(ptr %p.a16)
; VF2: call void @_ZGVbN2u_unalign
; VF4: call void @_ZGVbN4u_unalign
; VF8: call void @_ZGVbN8u_unalign
  call void @unalign(ptr %gep16.iv)
; VF2: call void @_ZGVbN2l4_unalign
; VF4: call void @_ZGVbN4l4_unalign
; VF8: call void @_ZGVbN8l4_unalign

; Aligned -> Aligned (match if alignment is equal or greater)
  call void @align16(ptr %p.a16)
; VF2: call void @_ZGVbN2ua16_align16
; VF4: call void @_ZGVbN4ua16_align16
; VF8: call void @_ZGVbN8ua16_align16
  call void @align16(ptr %gep16.iv)
; VF2-COUNT-2: call void @align16
; VF4: call void @_ZGVbN4l4a16_align16
; VF8: call void @_ZGVbN8l4a16_align16

; Aligned -> Aligned (no match: alignment is insufficient)
  call void @align16(ptr %p.a4)
; VF2-COUNT-2: call void @align16
; VF4-COUNT-4: call void @align16
; VF8-COUNT-8: call void @align16
  call void @align16(ptr %gep4.iv)
; VF2-COUNT-2: call void @align16
; VF4-COUNT-4: call void @align16
; VF8-COUNT-8: call void @align16

; Check that an aligned variant will be chosen over an unaligned variant if
; both are present
  call void @both16(ptr %p.a16);
; VF2: call void @_ZGVbN2ua16_both16
; VF4: call void @_ZGVbN4ua16_both16
; VF8: call void @_ZGVbN8ua16_both16
  call void @both16(ptr %gep16.iv);
; VF2: call void @_ZGVbN2l4_both16
; VF4: call void @_ZGVbN4l4a16_both16
; VF8: call void @_ZGVbN8l4a16_both16

; Check that an unaligned variant will still be chosen if both are present and
; the value is not aligned
  call void @both16(ptr %p)
; VF2: call void @_ZGVbN2u_both16
; VF4: call void @_ZGVbN4u_both16
; VF8: call void @_ZGVbN8u_both16
  call void @both16(ptr %gep.iv)
; VF2: call void @_ZGVbN2l4_both16
; VF4: call void @_ZGVbN4l4_both16
; VF8: call void @_ZGVbN8l4_both16

; Check that an unaligned variant will still be chosen if both are present and
; the value is not sufficiently aligned
  call void @both16(ptr %p.a4)
; VF2: call void @_ZGVbN2u_both16
; VF4: call void @_ZGVbN4u_both16
; VF8: call void @_ZGVbN8u_both16
  call void @both16(ptr %gep4.iv)
; VF2: call void @_ZGVbN2l4_both16
; VF4: call void @_ZGVbN4l4_both16
; VF8: call void @_ZGVbN8l4_both16

  %add = add nuw nsw i64 %.iv, 1
  %exit = icmp eq i64 %add, 256
  br i1 %exit, label %DIR.OMP.END.SIMD, label %omp.inner.for.body

DIR.OMP.END.SIMD:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1)

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN2u_unalign,_ZGVbN2l4_unalign,_ZGVbN4u_unalign,_ZGVbN4l4_unalign,_ZGVbN8u_unalign,_ZGVbN8l4_unalign" }
attributes #1 = { nounwind uwtable "vector-variants"="_ZGVbN2ua16_align16,_ZGVbN2l4a16_align16,_ZGVbN4ua16_align16,_ZGVbN4l4a16_align16,_ZGVbN8ua16_align16,_ZGVbN8l4a16_align16" }
attributes #2 = { nounwind uwtable "vector-variants"="_ZGVbN2ua16_both16,_ZGVbN2u_both16,_ZGVbN2l4a16_both16,_ZGVbN2l4_both16,_ZGVbN4ua16_both16,_ZGVbN4u_both16,_ZGVbN4l4a16_both16,_ZGVbN4l4_both16,_ZGVbN8ua16_both16,_ZGVbN8u_both16,_ZGVbN8l4a16_both16,_ZGVbN8l4_both16" }
attributes #3 = { nounwind uwtable }
