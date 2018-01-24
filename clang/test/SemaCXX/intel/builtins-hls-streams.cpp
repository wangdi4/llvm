// RUN: %clang_cc1 -fhls -fsyntax-only -verify %s

struct Foo {
  char a, b, c, d;
};
// Quick sanity check to validate the normal implementations.
template <typename T, int ReadyLatency, int BitsPerSymbol,
          bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty,
          bool UsesValid>
class stream_in {
public:
  T read() {
    return *__builtin_intel_hls_instream_read((T *)0, (__int64)this, ReadyLatency,
                                        BitsPerSymbol,
                                        FirstSymbolInHighOrderBits,
                                        UsesPackets,
                                        UsesEmpty,
                                        UsesValid);
  }
  T tryRead(bool *success) {
    return *__builtin_intel_hls_instream_tryRead((T *)0, (__int64)this, ReadyLatency,
                                           BitsPerSymbol,
                                           FirstSymbolInHighOrderBits,
                                           UsesPackets,
                                           UsesEmpty,
                                           UsesValid,
                                           success);
  }
  void write(const T *arg) {
    __builtin_intel_hls_instream_write(arg, (__int64)this, ReadyLatency,
                                 BitsPerSymbol,
                                 FirstSymbolInHighOrderBits,
                                 UsesPackets,
                                 UsesEmpty,
                                 UsesValid);
  }
  bool tryWrite(const T *arg) {
    return __builtin_intel_hls_instream_tryWrite(arg, (__int64)this, ReadyLatency,
                                           BitsPerSymbol,
                                           FirstSymbolInHighOrderBits,
                                           UsesPackets,
                                           UsesEmpty,
                                           UsesValid);
  }
};

template <typename T, int ReadyLatency, int BitsPerSymbol,
          bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty,
          bool UsesReady>
class stream_out {
public:
  T read() {
    return *__builtin_intel_hls_outstream_read((T *)0, (__int64)this, ReadyLatency,
                                         BitsPerSymbol,
                                         FirstSymbolInHighOrderBits,
                                         UsesPackets, UsesEmpty, UsesReady);
  }
  T tryRead(bool *success) {
    return *__builtin_intel_hls_outstream_tryRead((T *)0, (__int64)this, ReadyLatency,
                                            BitsPerSymbol,
                                            FirstSymbolInHighOrderBits,
                                            UsesPackets,
                                            UsesEmpty, UsesReady, success);
  }
  void write(const T *arg) {
    __builtin_intel_hls_outstream_write(arg, (__int64)this, ReadyLatency,
                                  BitsPerSymbol,
                                  FirstSymbolInHighOrderBits,
                                  UsesPackets, UsesEmpty, UsesReady);
  }
  bool tryWrite(const T *arg) {
    return __builtin_intel_hls_outstream_tryWrite(arg, (__int64)this, ReadyLatency,
                                            BitsPerSymbol,
                                            FirstSymbolInHighOrderBits,
                                            UsesPackets, UsesEmpty, UsesReady);
  }
};

void TestStreams() {
  stream_in<Foo, 5, 4, true, true, false, false> StrIn;
  stream_out<Foo, 5, 4, false, false, true, true> StrOut;
  Foo f;

  f = StrIn.read();
  bool s;
  f = StrIn.tryRead(&s);
  StrIn.write(&f);
  s = StrIn.tryWrite(&f);

  f = StrOut.read();
  f = StrOut.tryRead(&s);
  StrOut.write(&f);
  s = StrOut.tryWrite(&f);
}

void TestArgCounts() {
  // expected-error@+1 {{too few arguments to function call, expected 8, have 7}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 4, 0, true, true, true);
  // expected-error@+1 {{too few arguments to function call, expected 9, have 7}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 99, 4, true, true, true, true);

  // expected-error@+1 {{too many arguments to function call, expected 8, have 9}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 4, 0, true, true, false, true, 5);
  bool b;
  // expected-error@+1 {{too many arguments to function call, expected 9, have 10}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 99, 4, 0, true, true, true, true, &b, 5);
}

struct F;
int temp;

void ArgValues() {
  Foo f;
  // First Arg:
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read(f, 99, 0, 4, true, true, false, true);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read((F *)0, 99, 0, 4, true, true, false, true);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read((void *)0, 99, 0, 4, true, true, false, true);

  // Second Arg:
  // expected-error@+1 {{HLS builtin parameter must be an integer}}
  __builtin_intel_hls_instream_read((Foo *)0, "str", 0, 4, true, true, false, true);

  // Third Arg:
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, -1, 4, true, true, false, true);
  // expected-error@+1 {{argument to '__builtin_intel_hls_instream_read' must be a constant integer}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, temp, 4, true, true, false, true);

  // Fourth Arg:
  // expected-error@+1 {{HLS builtin parameter must be a factor of the type size}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 0, -1, true, true, false, true);
  // expected-error@+1 {{argument to '__builtin_intel_hls_instream_read' must be a constant integer}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 0, temp, true, true, false, true);
  // expected-error@+1 {{HLS builtin parameter must be a factor of the type size}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 0, 3, true, true, false, true);

  // Bool Args:
  // expected-error@+1 {{HLS builtin parameter must be a boolean value}}
  __builtin_intel_hls_instream_read((Foo *)0, 99, 3, 1, 5, true, false, true);

  // Success:
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a boolean}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 99, 3, 1, false, true, false, true, false);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a boolean}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 99, 3, 1, false, true, false, true, &temp);
}
