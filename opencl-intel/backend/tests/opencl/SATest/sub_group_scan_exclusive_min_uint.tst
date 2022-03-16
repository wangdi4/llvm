; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD -native-subgroups --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: %elt.min{{.*}} = call <16 x i32> @llvm.smin.v16i32(<16 x i32> %{{.*}}
; CHECK-COUNT-28: %elt.min{{.*}} = call <16 x i32> @llvm.umin.v16i32(<16 x i32> %{{.*}}
