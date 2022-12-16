; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memcpy intrinsic got recognized by HIR Lower Small Memset/Memcpy pass.

; HIR before optimization:
;    BEGIN REGION { }
;       @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%dst)[0]),  &((i8*)(%src)[0]),  20,  0);
;       ret ;
;    END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   (%dst)[0].0.0[i1] = (%src)[0].0.0[i1];
; CHECK:           + END LOOP
; CHECK:     END REGION


%struct1 = type { %struct2 }
%struct2 = type { [5 x i32] }

define void @foo(){
entry:
  %dst = alloca %struct1, align 4
  %src = alloca %struct1, align 4
  %bc1 = bitcast %struct1* %dst to i8*
  %bc2 = bitcast %struct1* %src to i8*
  br label %bb

bb:
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 4 dereferenceable(20) %bc1, i8* noundef nonnull align 4 dereferenceable(20) %bc2, i64 20, i1 false)
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #7

attributes #7 = { argmemonly nocallback nofree nounwind willreturn }
