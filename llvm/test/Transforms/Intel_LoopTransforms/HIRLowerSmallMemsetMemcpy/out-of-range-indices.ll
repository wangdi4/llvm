; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy" -hir-create-function-level-region -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; The test checks that memset intrinsic is not transformed by HIR Lower Small Memset/Memcpy pass
; because it would create out-of-range array access (%a[0][0][i1] with i1 going from 0 to 8).

; HIR before optimization:
;            BEGIN REGION { }
;                  @llvm.memset.p0i8.i64(&((i8*)(%a)[0][0][0]),  0,  36,  0);
;                  ret ;
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { }
; CHECK:           @llvm.memset.p0i8.i64(&((i8*)(%a)[0][0][0]),  0,  36,  0);
; CHECK:           ret ;
; CHECK:     END REGION

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLowerSmallMemsetMemcpy

define void @foo() {
entry:
  %a = alloca [3 x [3 x i32]], align 16
  %gep = getelementptr inbounds [3 x [3 x i32]], [3 x [3 x i32]]* %a, i64 0, i64 0, i64 0
  %bc = bitcast i32* %gep to i8*
  br label %bb

bb:
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 2 dereferenceable(36) %bc, i8 0, i64 36, i1 false)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3
attributes #3 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }
