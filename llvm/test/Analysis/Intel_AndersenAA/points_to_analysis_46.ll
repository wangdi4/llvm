; This test verifies that for_read_seq_lis fortran runtime I/O library
; is considered as no-side effect call by checking that %i10 is not
; pointing to <universal>.

; RUN: opt < %s -passes='require<anders-aa>' -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] read_input:i10<mem>    --> ({{[0-9]+}}): appludata_mp_ipr_<mem>
; CHECK-NOT: [2] read_input:i10<mem>    --> ({{[0-9]+}}): <universal>, ({{[0-9]+}}): appludata_mp_ipr_<mem>

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@appludata_mp_ipr_ = internal global i32 0

define internal void @read_input(ptr nocapture noundef readonly %arg) #0 {
bb:
  %i9 = alloca [4 x i8], align 1
  %i10 = alloca <{ ptr }>, align 8
  %i106 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 0
  store i8 9, ptr %i106, align 1
  %i110 = getelementptr inbounds <{ ptr }>, ptr %i10, i64 0, i32 0
  store ptr @appludata_mp_ipr_, ptr %i110, align 8
  %i111 = bitcast ptr %i10 to ptr
  %i112 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %arg, i32 3, i64 1239157112576, ptr nonnull %i106, ptr nonnull %i111)
  ret void
}

declare dso_local i32 @for_read_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #0

attributes #0 = { "intel-lang"="fortran" }
