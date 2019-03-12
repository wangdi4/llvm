// RUN: %clang_cc1 %s -O0 -fintel-compatibility -fhls -triple=x86_64-linux-gnu -emit-llvm -o - | FileCheck %s

struct Foo {
  char a, b, c, d;
};
enum RWMode {
  Read,
  Write
};

template <typename T, int dwidth, int awidth, int aspace, int latency,
         int maxburst, int align, int readwrite_mode, bool wait_request>
struct mm_master {
  mm_master(T *ptr, int size, bool use_socket = false) : mPtr(ptr), mSize(size),
    mUse_socket(use_socket) {
    __builtin_intel_hls_mm_master_init(ptr, size, use_socket, dwidth, awidth, aspace,
                             latency, maxburst, align, readwrite_mode,
                             wait_request);
  }

  T &operator[](int index) {
    return *__builtin_intel_hls_mm_master_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, index);
  }

  T &operator*() {
    return *__builtin_intel_hls_mm_master_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, (int)0);
  }

  T* mPtr;
  int mSize;
  bool mUse_socket;
};

void Sanity() {
  Foo f;
  mm_master<Foo, 8, 32, 0, 1, 1, 0, Write, false> mm(&f, 5, true);
  f = mm[5];
  f = *mm;
}
void SanityBool() {
  bool f;
  mm_master<bool, 8, 32, 0, 1, 1, 0, Write, false> mm(&f, 5, true);
  f = mm[5];
  f = *mm;
}

// CHECK: declare void @llvm.intel.hls.mm.master.init.s_struct.Foos(%struct.Foo*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1)
// CHECK: declare %struct.Foo* @llvm.intel.hls.mm.master.load.s_struct.Foos(%struct.Foo*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1, i32)

// CHECK: declare void @llvm.intel.hls.mm.master.init.i8(i8*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1)
// CHECK: declare i8* @llvm.intel.hls.mm.master.load.i8(i8*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1, i32)
