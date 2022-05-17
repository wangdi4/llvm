; Positive checks:
; RUN: SATest -BUILD -cpuarch=skx -dump-llvm-file=- -config=%s.cfg | FileCheck %s

; CHECK: define dso_local void @kernel1({{.*}}!vectorized_width ![[MD1:[0-9]+]]
; CHECK: define dso_local void @kernel4({{.*}}!vectorized_width ![[MD4:[0-9]+]]
; CHECK: define dso_local void @kernel8({{.*}}!vectorized_width ![[MD8:[0-9]+]]
; CHECK: define dso_local void @kernel16({{.*}}!vectorized_width ![[MD16:[0-9]+]]
; CHECK: define dso_local void @kernel8_vec_hint_float8({{.*}}!vectorized_width ![[MD8]]
; CHECK: define dso_local void @kernel4_vec_hint_int({{.*}}!vectorized_width ![[MD4]]
; CHECK: define dso_local void @kernel1_vec_hint_float({{.*}}!vectorized_width ![[MD1]]

; CHECK: [[MD1]] = !{i32 1}
; CHECK: [[MD4]] = !{i32 4}
; CHECK: [[MD8]] = !{i32 8}
; CHECK: [[MD16]] = !{i32 16}
