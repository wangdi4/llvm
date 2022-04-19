// RUN: %clang_cc1 %s -O0 -fintel-compatibility -fhls -triple=x86_64-linux-gnu -emit-llvm -no-opaque-pointers -o - | FileCheck %s

struct Foo {
  char a, b, c, d;
};
enum RWMode {
  Read,
  Write
};

template <typename T, int dwidth, int awidth, int aspace, int latency,
         int maxburst, int align, int readwrite_mode, bool wait_request>
struct mm_host {
  mm_host(T __attribute__((address_space(aspace))) *ptr, int size, bool use_socket = false) : mPtr(ptr), mSize(size),
    mUse_socket(use_socket) {
    __builtin_intel_hls_mm_host_init(ptr, size, use_socket, dwidth, awidth, aspace,
                             latency, maxburst, align, readwrite_mode,
                             wait_request);
  }

  T &operator[](int index) {
    return *(T*)__builtin_intel_hls_mm_host_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, index);
  }

  T &operator*() {
    return *(T*)__builtin_intel_hls_mm_host_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, (int)0);
  }

  T __attribute__((address_space(aspace))) *mPtr;
  int mSize;
  bool mUse_socket;
};

void Sanity() {
  constexpr int aspace = 1024;
  Foo __attribute__((address_space(aspace))) *f;
  mm_host<Foo, 8, 32, aspace, 1, 1, 0, Write, false> mm(f, 5, true);
  auto a = mm[5];
  auto b = *mm;
}
void SanityBool() {
  bool __attribute__((address_space(0))) *f;
  mm_host<bool, 8, 32, 0, 1, 1, 0, Write, false> mm(f, 5, true);
  auto a = mm[5];
  auto b = *mm;
}

// CHECK: declare void @llvm.intel.hls.mm.host.init.p1024s_struct.Foos(%struct.Foo addrspace(1024)*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1)
// CHECK: declare %struct.Foo addrspace(1024)* @llvm.intel.hls.mm.host.load.p1024s_struct.Foos(%struct.Foo addrspace(1024)*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1, i32)

// CHECK: declare void @llvm.intel.hls.mm.host.init.p0i8(i8*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1)
// CHECK: declare i8* @llvm.intel.hls.mm.host.load.p0i8(i8*, i32, i1, i32, i32, i32, i32, i32, i32, i32, i1, i32)
