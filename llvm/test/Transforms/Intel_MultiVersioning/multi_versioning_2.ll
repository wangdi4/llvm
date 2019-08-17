; Check that multi-versioning is not happening for unsafe loads.
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

%struct.S = type { i8 }

declare void @bar(%struct.S* %Arg) local_unnamed_addr

; Check that multi-versioning is still happening when unsafe loads are removed
; from the closure. The third load+cmp+select should be left unchanged.
;
;  CHECK: br i1 true
;  CHECK: select i1 true
;  CHECK: call void @bar
;  CHECK: load i8, i8* %0
;  CHECK: icmp ne i8 %
;  CHECK: select i1 %

;  CHECK: br i1 false
;  CHECK: select i1 false
;  CHECK: call void @bar
;  CHECK: load i8, i8* %0
;  CHECK: icmp ne i8 %
;  CHECK: select i1 %

define i32 @foo1(%struct.S* %Arg) local_unnamed_addr {
entry:
  %0 = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %1 = load i8, i8* %0, align 1
  %2 = icmp ne i8 %1, 0
  br i1 %2, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %3 = load i8, i8* %0, align 1
  %4 = icmp ne i8 %3, 0
  %5 = select i1 %4, i32 33, i32 22

  ; Moving the next load above this call is unsafe
  call void @bar(%struct.S* nonnull %Arg)

  %6 = load i8, i8* %0, align 1
  %7 = icmp ne i8 %6, 0
  %8 = select i1 %7, i32 33, i32 22

  %9 = add i32 %5, %8
  ret i32 %9
}

; Check that multi-versioning is not happening when moving load to entry block
; is not safe. The second load is unsafe and thus should be removed from the
; closure. After that there will be unsufficient number of conditional uses.
;
; CHECK-NOT: br i1 true
; CHECK-NOT: select i1 true
; CHECK-NOT: br i1 false
; CHECK-NOT: select i1 false

define i32 @foo2(%struct.S* %Arg) local_unnamed_addr {
entry:
  %0 = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %1 = load i8, i8* %0, align 1
  %2 = icmp ne i8 %1, 0
  br i1 %2, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  ; Moving the next load above this call is unsafe
  call void @bar(%struct.S* nonnull %Arg)

  %3 = load i8, i8* %0, align 1
  %4 = icmp ne i8 %3, 0
  %5 = select i1 %4, i32 33, i32 22

  ret i32 %5
}

