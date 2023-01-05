; RUN: opt < %s -passes='require<anders-aa>' -print-anders-constraints -disable-output  2>&1 | FileCheck %s
; Check that non-pointer assignments are collected right in escape
; analysis and appropriate constraints were created. Test is related
; to the bug CMPLRLLVM-1231.

; CHECK-DAG: *global_var2 = f1:call15 (Store)
; CHECK-DAG: foo:%0 = *global_var1 (Load)
; CHECK-DAG: f2:%0 = *global_var2 (Load)
; CHECK-DAG: bar:%0 = *global_var2 (Load)
; CHECK-DAG: *global_var1 = bar:%0 (Store)

%struct.a = type { i32, i32, i32, i32, i32, i32, [6 x [33 x i64]], [6 x [33 x i64]], [6 x [33 x i64]], [6 x [33 x i64]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16**, i16*, i16*, i16**, i16**, i16***, i8*, i16***, i64***, i64***, i16****, i8**, i8**, %struct.a*, %struct.a*, %struct.a*, i32, i32, i32, i32, i32, i32, i32 }

@global_var1 = internal unnamed_addr global %struct.a* null, align 8
@global_var2 = internal unnamed_addr global %struct.a* null, align 8

define void @f1() {
entry:
  %call15 = call fastcc %struct.a* @get_struct_a()
  store %struct.a* %call15, %struct.a** @global_var2, align 8
  ret void
}

define void @bar() {
entry:
  %0 = load i64, i64* bitcast (%struct.a** @global_var2 to i64*), align 8
  store i64 %0, i64* bitcast (%struct.a** @global_var1 to i64*), align 8
  ret void
}

declare %struct.a* @get_struct_a()

define void @foo() {
entry:
  %0 = load %struct.a*, %struct.a** @global_var1, align 8
  %mv = getelementptr inbounds %struct.a, %struct.a* %0, i64 0, i32 35
  %.pre931 = load i16****, i16***** %mv, align 8
  %arrayidx58.us = getelementptr inbounds i16***, i16**** %.pre931, i64 1
  %1 = load i16***, i16**** %arrayidx58.us, align 8
  %arrayidx62.us = getelementptr inbounds i16**, i16*** %1, i64 undef
  %2 = load i16**, i16*** %arrayidx62.us, align 8
  %arrayidx66.us = getelementptr inbounds i16*, i16** %2, i64 undef
  %3 = load i16*, i16** %arrayidx66.us, align 8
  store i16 0, i16* %3, align 2
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
  %0 = load %struct.a*, %struct.a** @global_var2, align 8
  %cmp194.i = icmp eq %struct.a* %0, null
  br i1 %cmp194.i, label %if.then195.i, label %if.else196.i

if.then195.i:                                     ; preds = %if.then193.i
  unreachable

if.else196.i:                                     ; preds = %if.then193.i
  unreachable
}
