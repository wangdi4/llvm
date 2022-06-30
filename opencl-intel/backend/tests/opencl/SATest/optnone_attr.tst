; RUN: SATest -BUILD --vectorizer-type=vpo -cpuarch=skx -config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK: define dso_local void @test
; CHECK-SAME: #[[#ATTR_ID:]]

; CHECK: attributes #[[#ATTR_ID]] =
; CHECK-SAME: optnone
