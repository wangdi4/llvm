;RUN: SATest -BUILD -config=%s.cfg >%t
;RUN: SATest -BUILD -config=%s.cfg 2>>%t
;RUN: FileCheck %s <%t
;CHECK-NOT: WARNING: Linking two modules of different data layouts!
;CHECK-NOT: WARNING: Linking two modules of different target triples:{{.*}}
