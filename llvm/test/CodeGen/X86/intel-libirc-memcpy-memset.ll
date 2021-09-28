; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: llc < %s -O0                                                  | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O0                       -disable-simplify-libcalls | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O0 -intel-libirc-allowed -disable-simplify-libcalls | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O0 -intel-libirc-allowed                            | FileCheck %s -check-prefix=LIBIRCO0
; RUN: llc < %s -O1 -intel-libirc-allowed                            | FileCheck %s -check-prefix=LIBIRCO1
;
; RUN: llc < %s -O2                                                  | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O2                       -disable-simplify-libcalls | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O2 -intel-libirc-allowed -disable-simplify-libcalls | FileCheck %s -check-prefix=NOLIBIRC
; RUN: llc < %s -O2 -intel-libirc-allowed                            | FileCheck %s -check-prefix=LIBIRCO2


; This test checks that llc make calls to libirc mem* functions only if
; both of the following are true
; 1. Option -intel-libirc-allowed is present
; 2. Option -disable-simplify-libcalls is absent


; NOLIBIRC-LABEL: MOO:
; NOLIBIRC-NOT: _intel_fast_memcpy
; LIBIRCO0-LABEL: MOO:
; LIBIRCO0-NOT: _intel_fast_memcpy
; LIBIRCO1-LABEL: MOO:
; LIBIRCO1-NOT: _intel_fast_memcpy
; LIBIRCO2-LABEL: MOO:
; LIBIRCO2: _intel_fast_memcpy

; Function Attrs: nounwind uwtable
define void @MOO(i8* noalias nocapture readonly %S, i8* noalias nocapture %D, i32 %N) #0 {
entry:
  %conv = zext i32 %N to i64
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %D, i8* %S, i64 %conv, i32 1, i1 false)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1


; NOLIBIRC-LABEL: COO:
; NOLIBIRC-NOT: _intel_fast_memset
; LIBIRCO0-LABEL: COO:
; LIBIRCO0-NOT: _intel_fast_memset
; LIBIRCO1-LABEL: COO:
; LIBIRCO1-NOT: _intel_fast_memset
; LIBIRCO2-LABEL: COO:
; LIBIRCO2: _intel_fast_memset

; Function Attrs: nounwind uwtable
define void @COO(i8* noalias nocapture %D, i8 signext %c, i32 %N) #0 {
entry:
  %conv1 = zext i32 %N to i64
  tail call void @llvm.memset.p0i8.i64(i8* %D, i8 %c, i64 %conv1, i32 1, i1 false)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1
; end INTEL_FEATURE_SW_ADVANCED
