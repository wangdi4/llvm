; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-lower-small-memset-memcpy" -hir-create-function-level-region -debug-only=hir-lower-small-memset-memcpy -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memset intrinsic got recognized by HIR Lower Small Memset/Memcpy pass.


; CHECK: BEGIN REGION { }
; CHECK:    @llvm.memset.p0i8.i64(&((i8*)(%dst)[0]),  0,  20,  0);
; CHECK:    ret ;
; CHECK: END REGION

; CHECK: HIR Lower Small Memset/Memcpy Pass for Function : foo
; CHECK: memset() is found.


%struct1 = type { %struct2 }
%struct2 = type { [5 x float] }

define void @foo(){
entry:
  %dst = alloca %struct1, align 4
  %bc = bitcast %struct1* %dst to i8*
  br label %bb

bb:
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 4 dereferenceable(20) %bc, i8 0, i64 20, i1 false)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* noalias nocapture writeonly, i8, i64, i1 immarg) #7


attributes #7 = { argmemonly nocallback nofree nounwind willreturn writeonly }
