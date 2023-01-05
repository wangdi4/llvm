; Test that caller/callee simd function parameter matching is done correctly
; with respect to caller/callee alignment. This means, for each param:
; - If caller/callee are unaligned/unaligned, the param matches
; - If caller/callee are   aligned/unaligned, the param matches
; - If caller/callee are   aligned/  aligned, the param matches if
;                                             caller align >= callee align
; - If caller/callee are unaligned/  aligned, the param matches if
;                                             caller ABI align >= callee align

; RUN: opt -S < %s -passes="vpo-paropt,vplan-vec" | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

declare void @unalign(i32*) #0
declare void @align16(i32*) #1
declare void @both16(i32*) #2

define dso_local void @foo(i32* %p, i32* %p.a4, i32* %p.a16) local_unnamed_addr #3 {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.ALIGNED"(i32* %p.a4, i32 4), "QUAL.OMP.ALIGNED"(i32* %p.a16, i32 16) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add, %omp.inner.for.body ]

; Unaligned -> Unaligned (always matches)
  call void @unalign(i32* %p)
; CHECK: call void @_ZGVbN2v_unalign

; Unaligned -> Aligned (no match: alignment is insufficient)
  call void @align16(i32* %p)
; CHECK: call void @align16(i32* %p)
; CHECK: call void @align16(i32* %p)

; Aligned -> Unaligned (always matches)
  call void @unalign(i32* %p.a16)
; CHECK: call void @_ZGVbN2v_unalign

; Aligned -> Aligned (match: alignment is equal or greater)
  call void @align16(i32* %p.a16)
; CHECK: call void @_ZGVbN2va16_align16

; Aligned -> Aligned (no match: alignment is insufficient)
  call void @align16(i32* %p.a4)
; CHECK: call void @align16(i32* %p.a4)
; CHECK: call void @align16(i32* %p.a4)

; Check that an aligned variant will be chosen over an unaligned variant if
; both are present
  call void @both16(i32* %p.a16);
; CHECK: call void @_ZGVbN2va16_both16

; Check that an unaligned variant will still be chosen if both are present and
; the value is not aligned
  call void @both16(i32* %p)
; CHECK: call void @_ZGVbN2v_both16

; Check that an unaligned variant will still be chosen if both are present and
; the value is not sufficiently aligned
  call void @both16(i32* %p.a4)
; CHECK: call void @_ZGVbN2v_both16

  %add = add nuw nsw i32 %.iv, 1
  %exit = icmp eq i32 %add, 256
  br i1 %exit, label %DIR.OMP.END.SIMD, label %omp.inner.for.body

DIR.OMP.END.SIMD:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1)

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN2v_unalign" }
attributes #1 = { nounwind uwtable "vector-variants"="_ZGVbN2va16_align16" }
attributes #2 = { nounwind uwtable "vector-variants"="_ZGVbN2va16_both16,_ZGVbN2v_both16" }
attributes #3 = { nounwind uwtable }
