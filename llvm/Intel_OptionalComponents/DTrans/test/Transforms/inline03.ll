; RUN: opt -dtrans-inline-heuristics -inline < %s -S 2>&1 | FileCheck --check-prefix=CHECK-IR %s
; RUN: opt -passes='cgscc(inline)' -dtrans-inline-heuristics < %s -S 2>&1 | FileCheck --check-prefix=CHECK-IR %s
; RUN: opt -dtrans-inline-heuristics -inline -inline-report=0xe847 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-RPT %s
; RUN: opt -passes='cgscc(inline)' -dtrans-inline-heuristics -inline-report=0xe847 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-RPT %s

target triple = "x86_64-unknown-linux-gnu"

; ------------------------------------------------------------------------------------------------------------------
; Function Description                                                  | Inline Heuristic                         |
; ------------------------------------------------------------------------------------------------------------------
; @myavg:       short function with a loop (trip count is 101), leaf    | inlined (profitable)                     |
; @myweight:    short function with a loop (TC: 101), leaf              | inlined (profitable)                     |
; @myinit:      short function 3 repeats of GETP+STORE seq, leaf        | inlined (single callsite, local linkage) |
; @mynoloops:   single BB, short, leaf function                         | inlined (single BB)                      |
; @mythreecalls:short function with a loop (TC is 11), leaf             | inlined (profitable)                     |
; ------------------------------------------------------------------------------------------------------------------

; CHECK-IR-NOT: call i32 @myavg
; CHECK-IR-NOT: call i32 @myavg
; CHECK-RPT: -> INLINE: myavg ({{[-0-9\<\=]+}}) <<Inlining is profitable>>
; CHECK-RPT: -> INLINE: myavg ({{[-0-9\<\=]+}}) <<Inlining is profitable>>
; CHECK-IR-NOT: call i32 @myweight
; CHECK-IR-NOT: call i32 @myweight
; CHECK-RPT: -> INLINE: myweight ({{[-0-9\<\=]+}}) <<Inlining is profitable>>
; CHECK-RPT: -> INLINE: myweight ({{[-0-9\<\=]+}}) <<Inlining is profitable>>
; CHECK-NOT-RPT: -> myinit {{\[\[}}Callsite preferred for multiversioning{{\]\]}}
; CHECK-NOT-RPT: -> mynoloops {{\[\[}}Callsite preferred for multiversioning{{\]\]}}
; CHECK-NOT-RPT: -> mythreeloops {{\[\[}}Callsite preferred for multiversioning{{\]\]}}

%struct.MYSTRUCT = type { i32 ()*, i32 ()*, i32 ()* }

@myglobal = common dso_local global %struct.MYSTRUCT zeroinitializer, align 8

define dso_local i32 @myavg() {
entry:
  br label %for.body
for.body:
  %counter.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nuw nsw i32 %counter.07, %i.06
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp ult i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end
for.end:
  ret i32 %add
}

define dso_local i32 @myweight() {
entry:
  br label %for.body
for.body:
  %counter.07 = phi i32 [ 0, %entry ], [ %sub, %for.body ]
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %sub = sub nuw nsw i32 %counter.07, %i.06
  %inc = sub nuw nsw i32 %i.06, 1
  %cmp = icmp ult i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end
for.end:
  ret i32 %sub
}

define dso_local i32 @mythreecalls() {
entry:
  br label %for.body
for.body:
  %counter.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nuw nsw i32 %counter.07, %i.06
  %inc = add nuw nsw i32 %i.06, 5
  %cmp = icmp ult i32 %inc, 50
  br i1 %cmp, label %for.body, label %for.end
for.end:
  ret i32 %add
}

define dso_local i32 @mynoloops() {
  ret i32 5
}

define dso_local i32 @foo() {
  %rv = call i32 @mythreecalls()
  ret i32 %rv
}

define dso_local i32 @bar() {
  %call1 = call i32 @myavg()
  %call2 = call i32 @myweight()
  %add1 = add nsw i32 %call1, %call2
  %call3 = call i32 @myweight()
  %call4 = call i32 @mynoloops()
  %add2 = add nsw i32 %call1, %call2
  %add3 = add nsw i32 %add1, %add2
  %add4 = call i32 @mythreecalls()
  %add = add nsw i32 %add3, %add4
  ret i32 %add
}

define dso_local i32 @baz() {
  %call1 = call i32 @myavg()
  %call2 = call i32 @myweight()
  %sub1 = sub nsw i32 %call1, %call2
  %call3 = call i32 @myweight()
  %call4 = call i32 @mynoloops()
  %sub2 = sub nsw i32 %call3, %call4
  %sub3 = sub nsw i32 %sub1, %sub2
  %sub4 = call i32 @mythreecalls()
  %sub = sub nsw i32 %sub3, %sub4
  ret i32 %sub
}

define internal void @myinit(%struct.MYSTRUCT* %myglobalptr) {
  %field1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myglobalptr, i32 0, i32 0
  store i32 ()* @foo, i32 ()** %field1, align 8
  %field2 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myglobalptr, i32 0, i32 1
  store i32 ()* @bar, i32 ()** %field2, align 8
  %field3 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %myglobalptr, i32 0, i32 2
  store i32 ()* @baz, i32 ()** %field3, align 8
  ret void
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  call fastcc void @myinit(%struct.MYSTRUCT* @myglobal)
  %0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @myglobal, i64 0, i32 0), align 8
  %call = call i32 %0()
  %1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @myglobal, i64 0, i32 1), align 8
  %call1 = call i32 %1()
  %add = add nsw i32 %call, %call1
  %2 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @myglobal, i64 0, i32 2), align 8
  %call2 = call i32 %2()
  %add3 = add nsw i32 %add, %call2
  ret i32 %add3
}
