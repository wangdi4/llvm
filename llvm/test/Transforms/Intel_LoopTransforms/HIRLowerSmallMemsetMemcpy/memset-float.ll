; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memset intrinsic got recognized by HIR Lower Small Memset/Memcpy pass.
; The access type is floating point in this test.

; HIR before optimization:
;            BEGIN REGION { }
;                  @llvm.memset.p0.i64(&((i8*)(%dst)[0]),  0,  20,  0);
;                  ret ;
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   (%dst)[0].0.0[i1] = 0.000000e+00;
; CHECK:           + END LOOP
; CHECK:     END REGION


%struct1 = type { %struct2 }
%struct2 = type { [5 x float] }

define void @foo(){
entry:
  %dst = alloca %struct1, align 4
  %bc = bitcast ptr %dst to ptr
  br label %bb

bb:
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %bc, i8 0, i64 20, i1 false)
  ret void
}

declare void @llvm.memset.p0.i64(ptr noalias nocapture writeonly, i8, i64, i1 immarg) #7


attributes #7 = { argmemonly nocallback nofree nounwind willreturn writeonly }
