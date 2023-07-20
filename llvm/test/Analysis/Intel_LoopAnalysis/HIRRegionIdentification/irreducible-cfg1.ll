; REQUIRES: asserts
; RUN: opt < %s -passes='print<hir-region-identification>' -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Verify that we detect and skip irreducible cfg. We tried to look for the IV before checking irreducibility which resulted in infinite looping.
; CHECK: Irreducible CFG not supported


@.str.49 = private unnamed_addr constant [2 x i8] c" \00", align 1

declare i64 @strspn(ptr nocapture, ptr nocapture)

declare i64 @strlen(ptr nocapture)

define void @foo(i8 %in, ptr %p.0.ph452) {
entry:
  br label %for.body220

for.body220:                                      ; preds = %entry, %for.inc243
  %0 = phi i8 [ %2, %for.inc243 ], [ %in, %entry ]
  %xml.addr.7394 = phi ptr [ %incdec.ptr244, %for.inc243 ], [ %p.0.ph452, %entry ]
  %call221 = tail call i64 @strspn(ptr %xml.addr.7394, ptr @.str.49) #13
  %cmp222 = icmp eq i64 %call221, 0
  br i1 %cmp222, label %while.cond231, label %if.then224

if.then224:                                       ; preds = %for.body220
  %add.ptr225 = getelementptr inbounds i8, ptr %xml.addr.7394, i64 %call221
  %call227 = tail call i64 @strlen(ptr %add.ptr225) #13
  %add228 = add i64 %call227, 1
  br label %while.cond231thread-pre-split

while.cond231thread-pre-split:                    ; preds = %while.body240, %if.then224
  %xml.addr.8.ph = phi ptr [ %incdec.ptr241, %while.body240 ], [ %xml.addr.7394, %if.then224 ]
  %.pr376 = load i8, ptr %xml.addr.8.ph, align 1
  br label %while.cond231

while.cond231:                                    ; preds = %while.cond231thread-pre-split, %for.body220
  %1 = phi i8 [ %.pr376, %while.cond231thread-pre-split ], [ %0, %for.body220 ]
  %xml.addr.8 = phi ptr [ %xml.addr.8.ph, %while.cond231thread-pre-split ], [ %xml.addr.7394, %for.body220 ]
  switch i8 %1, label %while.body240 [
    i8 0, label %for.inc243
    i8 32, label %for.inc243
  ]

while.body240:                                    ; preds = %while.cond231
  %incdec.ptr241 = getelementptr inbounds i8, ptr %xml.addr.8, i64 1
  br label %while.cond231thread-pre-split

for.inc243:                                       ; preds = %while.cond231, %while.cond231
  %incdec.ptr244 = getelementptr inbounds i8, ptr %xml.addr.8, i64 1
  %2 = load i8, ptr %incdec.ptr244, align 1
  %cmp218 = icmp eq i8 %2, 0
  br i1 %cmp218, label %for.end245.loopexit, label %for.body220

for.end245.loopexit:
  ret void
}


