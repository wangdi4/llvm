// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

class __normal_iterator {
  float* _M_current;
};


template<typename F, typename... Args>
void pool_worker(void *partition, int thread_id, int partition_count, F f, Args... args);

template<typename F, typename... Args>
void scheduled_thread_pool(int thread_count, F f, Args... args) ;

template<typename F, typename... Args>
void scheduled_thread_pool_auto(int thread_count, int partition_count, F f, Args... args) {
  scheduled_thread_pool(thread_count, pool_worker<F, Args...>, partition_count, f, args...);
}

  void ungapped_stage_worker(int i, int thread_id, const void *query_seq,
                             const void *query_cb,
                             __normal_iterator target_block_ids, int *out);
  void ungapped_stage() {
    void *I;
    scheduled_thread_pool_auto(5, 5, ungapped_stage_worker,
                               I, I,
                               __normal_iterator{}, nullptr);
  }

// CHECK: define {{.*}}ungapped_stage
// CHECK: alloca ptr, align 8, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]

// CHECK: define {{.*}}scheduled_thread_pool_auto{{.*}}(i32{{.*}}, i32{{.*}}, ptr noundef "intel_dtrans_func_index"="1" %{{.*}}, ptr noundef "intel_dtrans_func_index"="2" %{{.*}}, ptr noundef "intel_dtrans_func_index"="3" %{{.*}}, ptr "intel_dtrans_func_index"="4" %{{.*}}, ptr "intel_dtrans_func_index"="5" %{{.*}}){{.*}} !intel.dtrans.func.type ![[POOL_AUTO_MD:[0-9]+]]

// CHECK: declare !intel.dtrans.func.type ![[STAGE_WORKER_MD:[0-9]+]] void @{{.*}}ungapped_stage_worker{{.*}}(i32 noundef, i32 noundef, ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4")

// CHECK: declare !intel.dtrans.func.type ![[THREAD_POOL_MD:[0-9]+]] void @{{.*}}scheduled_thread_pool{{.*}}(i32 noundef, ptr noundef "intel_dtrans_func_index"="1", i32 noundef, ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i64, ptr "intel_dtrans_func_index"="5")

// CHECK: declare !intel.dtrans.func.type ![[POOL_WORKER_MD:[0-9]+]] void @{{.*}}pool_worker{{.*}}(ptr noundef "intel_dtrans_func_index"="1", i32 noundef, i32 noundef, ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4", i64, ptr "intel_dtrans_func_index"="5")

// CHECK: !intel.dtrans.types = !{![[NORMAL_ITR:[0-9]+]]}
// CHECK: ![[NORMAL_ITR]] = !{!"S", %class._ZTS17__normal_iterator.__normal_iterator zeroinitializer, i32 1, ![[FLOAT_PTR:[0-9]+]]}
// CHECK: ![[FLOAT_PTR]] = !{float {{.*}}, i32 1}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
// CHECK: ![[POOL_AUTO_MD]] = distinct !{![[FUNC_PTR:[0-9]+]], ![[VOID_PTR]], ![[VOID_PTR]], ![[FLOAT_PTR]], ![[VOID_PTR]]}
// CHECK: ![[FUNC_PTR]] = !{![[FUNC_TY:[0-9]+]], i32 1}
// CHECK: ![[FUNC_TY]]  = !{!"F", i1 false, i32 6, ![[VOID:[0-9]+]], ![[INT:[0-9]+]], ![[INT]], ![[VOID_PTR]], ![[VOID_PTR]], ![[FLOAT_PTR]], ![[INT_PTR:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[STAGE_WORKER_MD]] = distinct !{![[VOID_PTR]], ![[VOID_PTR]], ![[FLOAT_PTR]], ![[INT_PTR]]}
// CHECK: ![[THREAD_POOL_MD]] = distinct !{![[FUNC_PTR_2:[0-9]+]], ![[FUNC_PTR]], ![[VOID_PTR]], ![[VOID_PTR]], ![[VOID_PTR]]}
// CHECK: ![[FUNC_PTR_2]]  = !{![[FUNC_TY_2:[0-9]+]], i32 1}
// CHECK: ![[FUNC_TY_2]] = !{!"F", i1 false, i32 8, ![[VOID]], ![[VOID_PTR]], ![[INT]], ![[INT]], ![[FUNC_PTR]], ![[VOID_PTR]], ![[VOID_PTR]], ![[INTPTR:[0-9]+]], ![[VOID_PTR]]}
// CHECK: ![[INTPTR]] = !{i64 0, i32 0}
// CHECK: ![[POOL_WORKER_MD]] = distinct !{![[VOID_PTR]], ![[FUNC_PTR]], ![[VOID_PTR]], ![[VOID_PTR]], ![[VOID_PTR]]}
//
