; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that we are able to handle header phi update values coming from outside the loop without failing.

; CHECK: + DO i1 = 0, 74, 1   <DO_LOOP>
; CHECK: |   (%w.sroa.0)[0] = 0;
; CHECK: |   (%l)[0] = 0;
; CHECK: |   %0 = (%pz.035)[0];
; CHECK: |   (%pz.035)[0] = %0 + -1;
; CHECK: |   %pz.035 = &((%w.sroa.0)[0]);
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@str = external hidden unnamed_addr constant [7 x i8]
@str.2 = external hidden unnamed_addr constant [7 x i8]

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  %l = alloca i32, align 4
  %w.sroa.0 = alloca i32, align 16
  %l.0.l.0..sroa_cast = bitcast ptr %l to ptr
  store i32 0, ptr %l, align 4
  %w.sroa.0.0..sroa_cast = bitcast ptr %w.sroa.0 to ptr
  store i32 0, ptr %w.sroa.0, align 16
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %pz.035 = phi ptr [ %l, %entry ], [ %w.sroa.0, %for.body ]
  %id7.034 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  store i32 0, ptr %w.sroa.0, align 16
  store i32 0, ptr %l, align 4
  %0 = load i32, ptr %pz.035, align 4
  %sub = add i32 %0, -1
  store i32 %sub, ptr %pz.035, align 4
  %inc = add nuw nsw i32 %id7.034, 1
  %exitcond = icmp eq i32 %inc, 76
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %l.0.l.0.23 = load i32, ptr %l, align 4
  %cmp4 = icmp eq i32 %l.0.l.0.23, 0
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.end
  %puts25 = tail call i32 @puts(ptr @str.2)
  br label %if.end

if.else:                                          ; preds = %for.end
  %puts = tail call i32 @puts(ptr @str)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly) local_unnamed_addr

