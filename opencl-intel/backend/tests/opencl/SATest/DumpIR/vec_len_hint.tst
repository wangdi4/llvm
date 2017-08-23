; Positive checks:
; RUN: SATest -BUILD -cpuarch=skx -dump-llvm-file=%t -config=%s.cfg
; RUN: FileCheck %s --input-file=%t

; Check that unsupported vectorization width is handled properly:
; RUN: SATest -BUILD -cpuarch=corei7 -build-log -dump-llvm-file=%t -config=%s.cfg > %t.build_log
; RUN: FileCheck %s --input-file=%t --check-prefix=UNSUPPORTED
; RUN: FileCheck %s --input-file=%t.build_log --check-prefix=UNSUPPORTED_LOG

; CHECK: define void @kernel1{{.*}}!vectorized_width ![[MD1:[0-9]+]]
; CHECK: define void @kernel4{{.*}}!vectorized_width ![[MD4:[0-9]+]]
; CHECK: define void @kernel8{{.*}}!vectorized_width ![[MD8:[0-9]+]]
; CHECK: define void @kernel16{{.*}}!vectorized_width ![[MD16:[0-9]+]]
; CHECK: define void @kernel8_vec_hint_float8{{.*}}!vectorized_width ![[MD8]]
; CHECK: define void @kernel4_vec_hint_int{{.*}}!vectorized_width ![[MD4]]
; CHECK: define void @kernel1_vec_hint_float{{.*}}!vectorized_width ![[MD1]]

; CHECK: [[MD1]] = !{i32 1}
; CHECK: [[MD4]] = !{i32 4}
; CHECK: [[MD8]] = !{i32 8}
; CHECK: [[MD16]] = !{i32 16}

; UNSUPPORTED: define void @kernel8{{.*}}!vectorized_width ![[MD8:[0-9]+]]
; UNSUPPORTED: define void @kernel16{{.*}}!vectorized_width ![[MD16:[0-9]+]]

; UNSUPPORTED-NOT: [[MD8]] = !{i32 8}
; UNSUPPORTED-NOT: [[MD16]] = !{i32 16}

; UNSUPPORTED_LOG: Warning! Specified vectorization width for kernel <kernel8> is not supported by the architecture. Fall back to autovectorization mode.
; UNSUPPORTED_LOG-NEXT: Kernel <kernel8> was successfully vectorized (4)
; UNSUPPORTED_LOG: Warning! Specified vectorization width for kernel <kernel16> is not supported by the architecture. Fall back to autovectorization mode.
; UNSUPPORTED_LOG-NEXT: Kernel <kernel16> was successfully vectorized (4)
