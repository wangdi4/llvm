; RUN: opt < %s -passes='require<anders-aa>' -print-anders-constraints -disable-output  2>&1 | FileCheck %s
; Check that non-pointer assignments are collected right in escape
; analysis and appropriate constraints were created. Test is related
; to the bug CMPLRLLVM-1231.
; CHECK-DAG: *global_var2 = f1:call15 (Store)
; CHECK-DAG: foo:i = *global_var1 (Load)
; CHECK-DAG: f2:i = *global_var2 (Load)
; CHECK-DAG: bar:i = *global_var2 (Load)
; CHECK-DAG: *global_var1 = bar:i (Store)

%struct.a = type { i32, i32, i32, i32, i32, i32, [6 x [33 x i64]], [6 x [33 x i64]], [6 x [33 x i64]], [6 x [33 x i64]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i32, i32, i32, i32, i32 }

@global_var1 = internal unnamed_addr global ptr null, align 8
@global_var2 = internal unnamed_addr global ptr null, align 8

define void @f1() {
entry:
  %call15 = call fastcc ptr @get_struct_a()
  store ptr %call15, ptr @global_var2, align 8
  ret void
}

define void @bar() {
entry:
  %i = load i64, ptr @global_var2, align 8
  store i64 %i, ptr @global_var1, align 8
  ret void
}

declare ptr @get_struct_a()

define void @foo() {
entry:
  %i = load ptr, ptr @global_var1, align 8
  %mv = getelementptr inbounds %struct.a, ptr %i, i64 0, i32 35
  %.pre931 = load ptr, ptr %mv, align 8
  %arrayidx58.us = getelementptr inbounds ptr, ptr %.pre931, i64 1
  %i1 = load ptr, ptr %arrayidx58.us, align 8
  %arrayidx62.us = getelementptr inbounds ptr, ptr %i1, i64 undef
  %i2 = load ptr, ptr %arrayidx62.us, align 8
  %arrayidx66.us = getelementptr inbounds ptr, ptr %i2, i64 undef
  %i3 = load ptr, ptr %arrayidx66.us, align 8
  store i16 0, ptr %i3, align 2
  unreachable
}

define void @f2() {
entry:
  switch i32 undef, label %if.end82.i [
    i32 0, label %land.lhs.true.i
    i32 3, label %land.lhs.true.i
    i32 1, label %for.end.if.then41_crit_edge.i
  ]

for.end.if.then41_crit_edge.i:                    ; preds = %entry
  unreachable

land.lhs.true.i:                                  ; preds = %entry, %entry
  unreachable

if.end82.i:                                       ; preds = %entry
  br label %if.then193.i

if.then193.i:                                     ; preds = %if.end82.i
  %i = load ptr, ptr @global_var2, align 8
  %cmp194.i = icmp eq ptr %i, null
  br i1 %cmp194.i, label %if.then195.i, label %if.else196.i

if.then195.i:                                     ; preds = %if.then193.i
  unreachable

if.else196.i:                                     ; preds = %if.then193.i
  unreachable
}
