; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD -native-subgroups --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: %elt.min{{.*}} = call <8 x i32> @llvm.smin.v8i32(<8 x i32> {{.*}}
; CHECK-DAG-COUNT-2: %elt.min{{.*}} = call <8 x i32> @llvm.umin.v8i32(<8 x i32> {{.*}}
; CHECK-DAG-COUNT-6: %elt.min{{.*}} = call <8 x i32> @llvm.umin.v4i32(<4 x i32>{{.*}}
