; RUN: opt -passes='default<O3>' -inline-report=0xe807 -inline-threshold=-100 < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='default<O3>' -inline-report=0xe886 -inline-threshold=-100 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; Check that by default at -O3, the quality non-inlining messages are not
; overwritten in standard and metadata inlining reports by the always inliner.

; CHECK: foo1{{.*}}Callee has noinline attribute
; CHECK: foo2{{.*}}Inlining is not profitable

@glob1 = dso_local global i32 0, align 4
@glob2 = dso_local global i32 0, align 4

define dso_local i32 @foo1(i32 %arg) local_unnamed_addr #0 {
entry:
  %cmp = icmp slt i32 %arg, 0
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %mul = mul nsw i32 %arg, %arg
  br label %return

return:                                           ; preds = %entry, %if.end
  %retval.0 = phi i32 [ %mul, %if.end ], [ 0, %entry ]
  ret i32 %retval.0
}

define dso_local i32 @foo2(i32 %arg) local_unnamed_addr {
entry:
  %cmp = icmp slt i32 %arg, 0
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %mul = mul nsw i32 %arg, %arg
  br label %return

return:                                           ; preds = %entry, %if.end
  %retval.0 = phi i32 [ %mul, %if.end ], [ 0, %entry ]
  ret i32 %retval.0
}

define dso_local i32 @main() #0 {
entry:
  %0 = load i32, i32* @glob1, align 4
  %call = call i32 @foo1(i32 %0)
  %1 = load i32, i32* @glob2, align 4
  %call1 = call i32 @foo2(i32 %1)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

attributes #0 = { noinline }
