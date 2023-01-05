; CMPLRLLVM-8837: Verifies that points-to info for bug:%4 is not incorrectly
; computed as empty.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: bug:%4        --> (0): <universal>
; CHECK-NOT: [0] bug:%4        -->

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ant_s = type { %struct.info_s* }
%struct.info_s = type { %struct.info_s*, i32 }

define dso_local i32 @bug(i8* %p_beetle, %struct.ant_s** %pp_ant, i32 (...)* nocapture %wasp, i8* %p_fly) {
entry:
  %0 = load %struct.ant_s*, %struct.ant_s** %pp_ant, align 8
  %1 = bitcast %struct.ant_s** %pp_ant to i32**
  %call = tail call i32 @get_count(i32** %1)
  %info_array = getelementptr inbounds %struct.ant_s, %struct.ant_s* %0, i64 1, i32 0
  %2 = bitcast %struct.info_s** %info_array to i32**
  %call1 = tail call i32 @get_count(i32** nonnull %2)
  %3 = load %struct.info_s*, %struct.info_s** %info_array, align 8
  %arrayidx4 = getelementptr inbounds %struct.info_s, %struct.info_s* %3, i64 4
  %callee.knr.cast = bitcast i32 (...)* %wasp to i32 (i8*, i32, %struct.info_s*, i8*, ...)*
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %p_info.016 = phi %struct.info_s* [ %arrayidx4, %entry ], [ %4, %for.body ]
  %call5 = tail call i32 (i8*, i32, %struct.info_s*, i8*, ...) %callee.knr.cast(i8* %p_beetle, i32 0, %struct.info_s* nonnull %p_info.016, i8* %p_fly)
  %next = getelementptr inbounds %struct.info_s, %struct.info_s* %p_info.016, i64 0, i32 0
  %4 = load %struct.info_s*, %struct.info_s** %next, align 8
  %cmp = icmp eq %struct.info_s* %4, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}
declare dso_local i32 @get_count(i32**)
