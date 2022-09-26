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

; Function Attrs: nounwind uwtable
define internal void @read_input(i8* nocapture noundef readonly %0) #0 {
  %i9 = alloca [4 x i8], align 1
  %i10 = alloca <{ i8* }>, align 8
  %i106 = getelementptr inbounds [4 x i8], [4 x i8]* %i9, i64 0, i64 0
   store i8 9, i8* %i106, align 1
  %i110 = getelementptr inbounds <{ i8* }>, <{ i8* }>* %i10, i64 0, i32 0
  store i8* bitcast (i32* @appludata_mp_ipr_ to i8*), i8** %i110, align 8
  %i111 = bitcast <{ i8* }>* %i10 to i8*
  %i112 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %0, i32 3, i64 1239157112576, i8* nonnull %i106, i8* nonnull %i111)
  ret void
}

declare dso_local i32 @for_read_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #0

attributes #0 = { "intel-lang"="fortran" }
