; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD -native-subgroups --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: %elt.min{{.*}} = call <8 x i64> @llvm.smin.v8i64(<8 x i64> %{{.*}}
; CHECK-COUNT-26: %elt.min{{.*}} = call <8 x i64> @llvm.umin.v8i64(<8 x i64> %{{.*}}
