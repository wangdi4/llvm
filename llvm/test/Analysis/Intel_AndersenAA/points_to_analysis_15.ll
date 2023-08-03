; CMPLRLLVM-8837: Verifies that points-to info for bug:%4 is not incorrectly
; computed as empty.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: bug:i4        --> (0): <universal>
; CHECK-NOT: [0] bug:i4        -->

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ant_s = type { ptr }
%struct.info_s = type { ptr, i32 }

define dso_local i32 @bug(ptr %p_beetle, ptr %pp_ant, ptr nocapture %wasp, ptr %p_fly) {
entry:
  %i = load ptr, ptr %pp_ant, align 8
  %i1 = bitcast ptr %pp_ant to ptr
  %call = tail call i32 @get_count(ptr %i1)
  %info_array = getelementptr inbounds %struct.ant_s, ptr %i, i64 1, i32 0
  %i2 = bitcast ptr %info_array to ptr
  %call1 = tail call i32 @get_count(ptr nonnull %i2)
  %i3 = load ptr, ptr %info_array, align 8
  %arrayidx4 = getelementptr inbounds %struct.info_s, ptr %i3, i64 4
  %callee.knr.cast = bitcast ptr %wasp to ptr
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %p_info.016 = phi ptr [ %arrayidx4, %entry ], [ %i4, %for.body ]
  %call5 = tail call i32 (ptr, i32, ptr, ptr, ...) %callee.knr.cast(ptr %p_beetle, i32 0, ptr nonnull %p_info.016, ptr %p_fly)
  %next = getelementptr inbounds %struct.info_s, ptr %p_info.016, i64 0, i32 0
  %i4 = load ptr, ptr %next, align 8
  %cmp = icmp eq ptr %i4, null
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

declare dso_local i32 @get_count(ptr)
