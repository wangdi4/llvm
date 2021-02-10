; INTEL -- This test is landing in xmain first but will be replaced by
;          an upstream version when the same patch lands in llorg.
; Test upgrade of ptr.annotation intrinsics.
;
; RUN: llvm-dis < %s.bc | FileCheck %s

; Unused return values
define void @f1() {
  %t0 = call i8* @llvm.ptr.annotation.p0i8(i8* undef, i8* undef, i8* undef, i32 undef)
;CHECK:  call i8* @llvm.ptr.annotation.p0i8(i8* undef, i8* undef, i8* undef, i32 undef, i8* null)

  %t1 = call i16* @llvm.ptr.annotation.p0i16(i16* undef, i8* undef, i8* undef, i32 undef)
;CHECK:  call i16* @llvm.ptr.annotation.p0i16(i16* undef, i8* undef, i8* undef, i32 undef, i8* null)

  %t2 = call i256* @llvm.ptr.annotation.p0i256(i256* undef, i8* undef, i8* undef, i32 undef)
;CHECK:  call i256* @llvm.ptr.annotation.p0i256(i256* undef, i8* undef, i8* undef, i32 undef, i8* null)
  ret void
}

; Used return values
define i16* @f2(i16* %x, i16* %y) {
  %t0 = call i16* @llvm.ptr.annotation.p0i16(i16* %x, i8* undef, i8* undef, i32 undef)
  %t1 = call i16* @llvm.ptr.annotation.p0i16(i16* %y, i8* undef, i8* undef, i32 undef)
  %cmp = icmp ugt i16* %t0, %t1
  %sel = select i1 %cmp, i16* %t0, i16* %t1
  ret i16* %sel
; CHECK:  [[T0:%.*]] = call i16* @llvm.ptr.annotation.p0i16(i16* %x, i8* undef, i8* undef, i32 undef, i8* null)
; CHECK:  [[T1:%.*]] = call i16* @llvm.ptr.annotation.p0i16(i16* %y, i8* undef, i8* undef, i32 undef, i8* null)
; CHECK:  %cmp = icmp ugt i16* [[T0]], [[T1]]
; CHECK:  %sel = select i1 %cmp, i16* [[T0]], i16* [[T1]]
; CHECK:  ret i16* %sel
}

declare i8*   @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32)
; CHECK: declare i8*   @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
declare i16*  @llvm.ptr.annotation.p0i16(i16*, i8*, i8*, i32)
; CHECK: declare i16*   @llvm.ptr.annotation.p0i16(i16*, i8*, i8*, i32, i8*)
declare i256* @llvm.ptr.annotation.p0i256(i256*, i8*, i8*, i32)
; CHECK: declare i256*   @llvm.ptr.annotation.p0i256(i256*, i8*, i8*, i32, i8*)
