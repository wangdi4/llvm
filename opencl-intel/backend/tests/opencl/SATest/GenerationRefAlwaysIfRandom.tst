;RUN: SATest -VAL -config=%s.cfg
;RUN: SATest -VAL -config=%s.cfg >%t
;RUN: FileCheck %s <%t
;CHECK-NOT: Mismatched values index
