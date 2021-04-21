// RUN: %clang_cc1 -fhls -fsyntax-only -verify %s

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
  mm_host(T *ptr, int size, bool use_socket = false) : mPtr(ptr), mSize(size),
    mUse_socket(use_socket) {
    __builtin_intel_hls_mm_host_init(ptr, size, use_socket, dwidth, awidth, aspace,
                             latency, maxburst, align, readwrite_mode,
                             wait_request);
  }

  T &operator[](int index) {
    return *__builtin_intel_hls_mm_host_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, index);
  }

  T &operator*() {
    return *__builtin_intel_hls_mm_host_load(mPtr, mSize, mUse_socket, dwidth, awidth,
                                     aspace, latency, maxburst, align,
                                     readwrite_mode, wait_request, (int)0);
  }
  T* mPtr;
  int mSize;
  bool mUse_socket;
};

void Sanity() {
  Foo f;
  mm_host<Foo, 8, 32, 0, 1, 1, 0, Write, false> mm(&f, 5, true);
  f = mm[5];
  f = *mm;
}

void ArgCounts() {
  Foo f;
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{too few arguments to function call, expected 11, have 10}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, 0, Write);
  // expected-error@+1 {{too few arguments to function call, expected 12, have 11}}
  __builtin_intel_hls_mm_host_load(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false);

  // expected-error@+1 {{too many arguments to function call, expected 11, have 12}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false, 5);
  // expected-error@+1 {{too many arguments to function call, expected 12, have 13}}
  __builtin_intel_hls_mm_host_load(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false, 3, 1);
}

struct F;

void ArgConstraints() {
  Foo f;

  // First Arg (T*):
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_mm_host_init(f, 5, false, 8, 32, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_mm_host_init((F*)&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_mm_host_init((void*)0, 5, false, 8, 32, 0, 1, 1, 0, Write, false);

  // Second Arg (Size):
  // expected-error@+1 {{HLS builtin parameter must be an integer}}
  __builtin_intel_hls_mm_host_init(&f, "str", false, 8, 32, 0, 1, 1, 0, Write, false);

  // Third Arg (use_socket):
  // expected-error@+1 {{HLS builtin parameter must be a boolean value}}
  __builtin_intel_hls_mm_host_init(&f, 5, 3, 8, 32, 0, 1, 1, 0, Write, false);

  // Fourth Arg (dwidth):
  // expected-error@+1 {{HLS builtin parameter must be a power of two integer between 8 and 1024}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 9, 32, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be a power of two integer between 8 and 1024}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 0, 32, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be a power of two integer between 8 and 1024}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 2048, 32, 0, 1, 1, 0, Write, false);

  // Fifth Arg (awidth):
  // expected-error@+1 {{HLS builtin parameter must be an integer between 1 and 64}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 0, 0, 1, 1, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be an integer between 1 and 64}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 65, 0, 1, 1, 0, Write, false);

  // Sixth Arg (aspace):
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, -1, 1, 1, 0, Write, false);
  // expected-error@+1 {{argument to '__builtin_intel_hls_mm_host_init' must be a constant integer}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, "str", 1, 1, 0, Write, false);
  int i = 5;
  // expected-error@+1 {{argument to '__builtin_intel_hls_mm_host_init' must be a constant integer}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, i, 1, 1, 0, Write, false);

  // Seventh Arg (latency):
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, -1, 1, 0, Write, false);
  // expected-error@+1 {{argument to '__builtin_intel_hls_mm_host_init' must be a constant integer}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, i, 1, 0, Write, false);

  // Eighth Arg (maxburst):
  // expected-error@+1 {{HLS builtin parameter must be an integer between 1 and 1024 inclusive}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 0, 0, Write, false);
  // expected-error@+1 {{HLS builtin parameter must be an integer between 1 and 1024 inclusive}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1025, 0, Write, false);

  // Ninth Arg (align):
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, -1, Write, false);

  // Tenth Arg (readwrite_mode):
  // expected-error@+1 {{argument to '__builtin_intel_hls_mm_host_init' must be a constant integer}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, 0, "str", false);

  // Eleventh Arg(waitrequest):
  // expected-error@+1 {{HLS builtin parameter must be a boolean value}}
  __builtin_intel_hls_mm_host_init(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, 1);


  // Twelfth Arg(index):
  double d;
  // expected-error@+1 {{HLS builtin parameter must be an integer}}
  __builtin_intel_hls_mm_host_load(&f, 5, false, 8, 32, 0, 1, 1, 0, Write, false, d);
}
