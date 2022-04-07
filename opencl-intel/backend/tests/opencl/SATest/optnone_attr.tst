; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups -cpuarch=skx -config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK: define void @test
; CHECK-SAME: #[[#ATTR_ID:]]

; CHECK: attributes #[[#ATTR_ID]] =
; CHECK-SAME: optnone
