; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -intel-ind-call-force-dtrans -dtransanalysis -indirectcallconv -intel-ind-call-conv-max-target=3 < %s -S 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume -intel-ind-call-force-dtrans -passes=indirectcallconv -intel-ind-call-conv-max-target=3 < %s  -S 2>&1 | FileCheck %s

; Check that the first indirect call in @main is specialized to @foo and
; then @baz, and then @baf.

; CHECK: define dso_local i32 @main
; CHECK: .indconv.cmp.foo:
; CHECK: [[T1:%.indconv.c.*]] = icmp eq i32 ()* [[T10:%t.*]], @foo
; CHECK: br i1 [[T1]], label %[[L0:.indconv.call.foo.*]], label %[[L1:.indconv.cmp.baz.*]]
; CHECK: [[L0]]:
; CHECK: call i32 @foo()
; CHECK: br label %[[L2:.indconv.sink.*]]
; CHECK: [[L1]]:
; CHECK: [[T2:%.indconv.c.*]] = icmp eq i32 ()* [[T10]], @baz
; CHECK: br i1 [[T2]], label %[[L3:.indconv.call.baz.*]], label %[[L4:.indconv.call.baf.*]]
; CHECK: [[L3]]:
; CHECK: call i32 @baz()
; CHECK: br label %[[L2]]
; CHECK: [[L4]]:
; CHECK: call i32 @baf()
; CHECK: br label %[[L2]]
; CHECK: [[L2]]:

; Check that the second indirect call in @main is specialized to @bar and
; then @baf, and then @baz.

; CHECK: .indconv.cmp.bar:
; CHECK:  [[T3:%.indconv.c.*]] = icmp eq i32 ()* [[T11:%t.*]], @bar
; CHECK:  br i1 [[T3]], label %[[L5:.indconv.call.bar.*]], label %[[L6:.indconv.cmp.baf.*]]
; CHECK: [[L5]]:
; CHECK:  call i32 @bar()
; CHECK:  br label %[[L9:.indconv.sink.*]]
; CHECK: [[L6]]:
; CHECK:  [[T4:%.indconv.c.*]] = icmp eq i32 ()* [[T11]], @baf
; CHECK:  br i1 [[T4]], label %[[L7:.indconv.call.baf.*]], label %[[L8:.indconv.call.baz.*]]
; CHECK: [[L7]]:
; CHECK:  call i32 @baf()
; CHECK:  br label %[[L9]]
; CHECK: [[L8]]:
; CHECK:  call i32 @baz()
; CHECK: br label %[[L9]]
; CHECK: [[L9]]:


declare dso_local i32 @boo() local_unnamed_addr

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

define dso_local i32 @baz() {
  ret i32 7
}

define dso_local i32 @baf() {
  ret i32 7
}

%struct.MYSTRUCT = type { i32 ()*, i32 ()* }

@globstruct = internal global %struct.MYSTRUCT { i32 ()* @foo, i32 ()* @bar }, align 8

define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 @boo()
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 ()* @baz, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  store i32 ()* @baf, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store i32 ()* @baf, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  store i32 ()* @baz, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %t0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  %call1 = call i32 %t0()
  %t1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %call2 = call i32 %t1()
  %add = add nsw i32 %call1, %call2
  ret i32 %add
}