; Test multi-versioning with various combinations of def-use chains in
; bool closure.
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

%struct.S = type { i8 }

;                           / Branch
;  Arg - GEP - Load - Cmp NE
;                           \ Select
;
; CHECK: br i1 true
; CHECK: select i1 true
; CHECK: br i1 false
; CHECK: select i1 false

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

  %3 = select i1 %2, i32 33, i32 22
  ret i32 %3
}

;                   / Cmp NE - Branch
;  Arg - GEP - Load
;                   \ Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

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

  %3 = icmp eq i8 %1, 0
  %4 = select i1 %3, i32 33, i32 22
  ret i32 %4
}

;           / Load - Cmp NE - Branch
;  Arg - GEP
;           \ Load - Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

define i32 @foo3(%struct.S* %Arg) local_unnamed_addr {
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
  %4 = icmp eq i8 %3, 0
  %5 = select i1 %4, i32 33, i32 22
  ret i32 %5
}

;     / GEP - Load - Cmp NE - Branch
;  Arg
;     \ GEP - Load - Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

define i32 @foo4(%struct.S* %Arg) local_unnamed_addr {
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

  %3 = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %4 = load i8, i8* %3, align 1
  %5 = icmp eq i8 %4, 0
  %6 = select i1 %5, i32 33, i32 22
  ret i32 %6
}

