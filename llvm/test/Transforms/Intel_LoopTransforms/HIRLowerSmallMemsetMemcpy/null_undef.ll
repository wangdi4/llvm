; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; The test checks that memcpy intrinsic with null or undef
; is skiped by HIR Lower Small Memset/Memcpy pass.

; HIR before optimization:
;            BEGIN REGION { }
;                  @llvm.memcpy.p0.p0.i64(&((undef)[0]),  &((ptr)(%src)[0]),  20,  0);
;                  @llvm.memcpy.p0.p0.i64(&((ptr)(%dst)[0]),  null,  20,  0);
;                  ret ;
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { }
; CHECK:           @llvm.memcpy.p0.p0.i64(&((undef)[0]),  &((%src)[0]),  20,  0);
; CHECK:           @llvm.memcpy.p0.p0.i64(&((%dst)[0]),  null,  20,  0);
; CHECK:           ret ;
; CHECK:     END REGION


%struct1 = type { %struct2 }
%struct2 = type { [5 x i32] }

define void @foo(){
entry:
  %dst = alloca %struct1, align 4
  %src = alloca %struct1, align 4
  %bc1 = bitcast ptr %dst to ptr
  %bc2 = bitcast ptr %src to ptr
  br label %bb

bb:
  call void @llvm.memcpy.p0.p0.i64(ptr undef, ptr noundef nonnull align 4 dereferenceable(20) %bc2, i64 20, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(20) %bc1, ptr null, i64 20, i1 false)
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

attributes #7 = { argmemonly nocallback nofree nounwind willreturn }
