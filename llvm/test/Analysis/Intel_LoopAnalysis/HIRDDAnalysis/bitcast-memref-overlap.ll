; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-create-function-level-region -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that output dependence is calculated between (i32*)(%p)[i1] and (i32*)(%p)[i1 + 1].
; The type of %p is i16*. Distance is negative.

; <17>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <4>                |   (i32*)(%p)[i1] = i1;
; <10>               |   (i32*)(%p)[i1 + 1] = i1 + 1;
; <17>               + END LOOP

; CHECK: DD graph for function foo:
; CHECK: (i32*)(%p)[i1] --> (i32*)(%p)[i1 + 1] OUTPUT (*)

target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %i.012 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %ptridx = getelementptr inbounds i16, ptr %p, i64 %indvars.iv
  %bc = bitcast ptr %ptridx to ptr
  store i32 %i.012, ptr %bc, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add = add nuw nsw i32 %i.012, 1
  %ptridx4 = getelementptr inbounds i16, ptr %p, i64 %indvars.iv.next
  %bc1 = bitcast ptr %ptridx4 to ptr
  %0 = trunc i64 %indvars.iv.next to i32
  store i32 %0, ptr %bc1, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Test checks that output dependence is calculated between (i32*)(%q)[i1+1] and (i32*)(%q)[i1].
; The type of %q is i16*. Distance is positive.

; <0>          BEGIN REGION { }
; <18>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <8>                |   (i32*)(%s)[i1] = (i32*)(%q)[i1 + 1];
; <12>               |   (i32*)(%q)[i1] = i1 + 1;
; <18>               + END LOOP
; <0>          END REGION

; CHECK: DD graph for function bar:
; CHECK: (i32*)(%q)[i1 + 1] --> (i32*)(%q)[i1] ANTI (*)

define dso_local i32 @bar(ptr nocapture %q, ptr nocapture %s, i32 %m) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add.ptr = getelementptr inbounds i16, ptr %q, i64 %indvars.iv
  %add.ptr1 = getelementptr inbounds i16, ptr %add.ptr, i64 1
  %0 = bitcast ptr %add.ptr1 to ptr
  %1 = load i32, ptr %0, align 4
  %add.ptr3 = getelementptr inbounds i16, ptr %s, i64 %indvars.iv
  %2 = bitcast ptr %add.ptr3 to ptr
  store i32 %1, ptr %2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %3 = bitcast ptr %add.ptr to ptr
  %4 = trunc i64 %indvars.iv.next to i32
  store i32 %4, ptr %3, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %idxprom = sext i32 %m to i64
  %ptridx = getelementptr inbounds i16, ptr %q, i64 %idxprom
  %5 = load i16, ptr %ptridx, align 2
  %conv = sext i16 %5 to i32
  ret i32 %conv
}


; Test checks that output dependence is calculated between (i32*)(%r)[3] and (i32*)(%r)[i1 + 1].
; The type of %r is i16*. Distance is unknown.

; <0>          BEGIN REGION { }
; <14>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <3>                |   (i32*)(%r)[3] = i1;
; <8>                |   (i32*)(%r)[i1 + 1] = i1 + 1;
; <14>               + END LOOP
; <0>          END REGION

; CHECK: DD graph for function baz:
; CHECK: (i32*)(%r)[3] --> (i32*)(%r)[i1 + 1] OUTPUT (*)

define dso_local i32 @baz(ptr nocapture %r, i32 %n) {
entry:
  %add.ptr = getelementptr inbounds i16, ptr %r, i64 3
  %0 = bitcast ptr %add.ptr to ptr
  %add.ptr1 = getelementptr inbounds i16, ptr %r, i64 1
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %0, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add.ptr2 = getelementptr inbounds i16, ptr %add.ptr1, i64 %indvars.iv
  %2 = bitcast ptr %add.ptr2 to ptr
  %3 = trunc i64 %indvars.iv.next to i32
  store i32 %3, ptr %2, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %idxprom = sext i32 %n to i64
  %ptridx = getelementptr inbounds i16, ptr %r, i64 %idxprom
  %4 = load i16, ptr %ptridx, align 2
  %conv = sext i16 %4 to i32
  ret i32 %conv
}

; Test checks that output dependence is not calculated between (i16*)(%p)[i1] and (i16*)(%p)[i1 + 1].
; The type of %p is i32*.

; <17>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <4>                |   (i16*)(%p)[i1] = i1;
; <10>               |   (i16*)(%p)[i1 + 1] = i1 + 1;
; <17>               + END LOOP

; CHECK: DD graph for function quux:
; CHECK-NOT: (i16*)(%u)[i1] --> (i16*)(%u)[i1 + 1] OUTPUT (*)

define dso_local void @quux(ptr nocapture %u) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %i.012 = phi i16 [ 0, %entry ], [ %add, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %u, i64 %indvars.iv
  %bc = bitcast ptr %ptridx to ptr
  store i16 %i.012, ptr %bc, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add = add nuw nsw i16 %i.012, 1
  %ptridx4 = getelementptr inbounds i32, ptr %u, i64 %indvars.iv.next
  %bc1 = bitcast ptr %ptridx4 to ptr
  %trunc = trunc i64 %indvars.iv.next to i16
  store i16 %trunc, ptr %bc1, align 2
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

