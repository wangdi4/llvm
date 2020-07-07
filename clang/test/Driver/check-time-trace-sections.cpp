// INTEL_CUSTOMIZATION
// We're having problems with this test since upgrading to Python3.
// It is disabled until this is fixed.
// UNSUPPORTED: true
// end INTEL_CUSTOMIZATION

// RUN: %clangxx -S -ftime-trace -ftime-trace-granularity=0 -o %T/check-time-trace-sections %s
// RUN: cat %T/check-time-trace-sections.json | %python %S/check-time-trace-sections.py

template <typename T>
void foo(T) {}
void bar() { foo(0); }
