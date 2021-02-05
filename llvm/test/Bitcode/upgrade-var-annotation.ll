; INTEL -- This test is landing in xmain first but will be replaced by
;          an upstream version when the same patch lands in llorg.
; Test upgrade of var.annotation intrinsics.
;
; RUN: llvm-dis < %s.bc | FileCheck %s


define void @f() {
  call void @llvm.var.annotation(i8* undef, i8* undef, i8* undef, i32 undef)
;CHECK:  call void @llvm.var.annotation(i8* undef, i8* undef, i8* undef, i32 undef, i8* null)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.var.annotation(i8*, i8*, i8*, i32)
; CHECK: declare void @llvm.var.annotation(i8*, i8*, i8*, i32, i8*)
