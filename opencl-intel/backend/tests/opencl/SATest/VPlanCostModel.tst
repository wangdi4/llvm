; RUN: SATest -BUILD --vectorizer-type=vpo -cpuarch=corei7 -build-log -config=%s.cfg | FileCheck %s
; CHECK: Kernel "test_discard_vec_kernel" was not vectorized
; CHECK: Kernel "test_vec_len_hint" was successfully vectorized (4)
; CHECK: Kernel "test_redq_sub_group_size" was successfully vectorized (4)
; CHECK: Kernel "test_subgroups" was successfully vectorized (4)
; CHECK: Kernel "test_keep_vec_kernel" was successfully vectorized (4)

