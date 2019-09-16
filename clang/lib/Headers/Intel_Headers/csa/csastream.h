/*===---- csastream.h - CSA intrinsics -------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __CSASTREAM_H
#define __CSASTREAM_H
#include <cstddef>
#include <queue>
#include <mutex>
#include <condition_variable>

/* Define the C++ interface for High-level LICs or streams on CSA */
namespace csa
{
  template<typename T, int depth = 0, int pref_depth = 0>
  class stream {
#ifdef __CSA__
    int identifier;
#else
    std::queue<T> myqueue;
    std::mutex mtx;
    std::condition_variable cv;
#endif

  public:
    __attribute__((always_inline))
#ifdef __CSA__
    stream() : identifier(__builtin_csa_lic_init(sizeof(T), depth, pref_depth))
    {}
#else
  stream(): myqueue(){}
#endif
    //delete the move constructor
    stream(stream<T, depth, pref_depth>&&) = delete;

    __attribute__((always_inline))
    T read() {
#ifdef __CSA__
      return __builtin_csa_lic_read(T(), identifier);
#else
      auto available = [this](){return !myqueue.empty();};
      std::unique_lock<std::mutex> lck(mtx);
      cv.wait(lck, available);
      T val =  myqueue.front();
      myqueue.pop();
      return val;
#endif
    }
    __attribute__((always_inline))
    void write(T val) {
#ifdef __CSA__
      __builtin_csa_lic_write(identifier, val);
#else
      std::unique_lock<std::mutex> lck(mtx);
      myqueue.push(val);
      cv.notify_one();
#endif
    }
    // rotate is used to bypass the single write site restriction
    // to the same stream in the following scenario:
    // if input stream in has {012} values,
    // this function will return this sequence:{012012012}
    // if repetition rate R is 3
    // An example is below where one loop does the memory load
    // the second loop does the rotation and use the returned values (val)
    // as follows:
    // Original code (C, C++, Fortran)
    // for (int i = 0; i < M; ++i)
    //   in.write(A[i]);
    // for (int r = 0; r < R; ++r) {
    //   for (int i = 0; i < M; ++i) {
    //     int val;
    //     if (r != 0) val = back.read();
    //     else val = in.read();
    //     use_the_values(val);
    //     if (r != R-1) back.write(val);
    //   }
    // }
    // Using rotate method:
    // #include “csastream.h”
    // for (int i=0; i<M; i++)
    //   in.write(A[i]);
    // for (int r = 0; r < R; ++r)
    //   for (int i=0; i<M; i++) {
    //     val = in.rotate(r, 0, R, 1);
    //     use_the_values(val);
    //   }
    template <size_t len> __attribute__((always_inline))
    T rotate(int range, int low, int up, int step) {
      csa::stream<T, len, len> back;
      T val;
      if(range == low) val = this->read();
      if(range != low) val = back.read();
      if(range != up-step) back.write(val);
      return val;
    }
    //Declare these as friends to stream so it can access
    //the private copy constructor
    template <typename Tf, class... In>
    friend void merge(csa::stream<Tf>& out, In&... in);
    template <typename Tf, class... Out>
    friend void scatter(csa::stream<Tf>& in, Out&... out);

  private:
#ifdef __CSA__
    //The templated constructor is the default  one
    // if the depths happened to be the same
    stream(const stream<T, depth, pref_depth> &) = default;
    template <typename U, int d2, int pd2> friend class stream;
    // allows conversion to streams that ignore depths in helper methods.
    template <int d2, int pd2>
    __attribute__((always_inline))
    stream(const stream<T, d2, pd2> &other) : identifier(other.identifier) {}
#else
    stream(const stream<T, depth, pref_depth> &) = delete;
#endif
  };
#ifdef __CSA__
  //merge is used to collect data from different streams into
  // one stream
  // this has been useful in cases where the conversion of strided
  // unrolled loop to a unit-strided loop is needed to gererate sst

  //the first version takes an an array of LICs as input
  //example: merge(lic_one, lic);
  template <size_t Start, size_t N, size_t Stride>
  struct merge_helper {
    template <typename T> __attribute__((always_inline))
    static void merge(csa::stream<T>& out, csa::stream<T> (&in)[N]) {
      csa::stream<T> merged0, merged1;
      merge_helper<Start, N, (Stride<<1)>::merge(merged0, in);
      merge_helper<Start + Stride, N, (Stride<<1)>::merge(merged1, in);
      csa::stream<bool> vals;
      bool vals_out;
      //TODO: The seq should be replaced by a fountain
      //once late tools adds support for fountains
      __asm__ volatile
        (" seqne32 %%ign, %0, %%ign, %%ign, 0, 1, 1" : "=a"(vals_out));
      vals.write(vals_out);
      out.write(vals.read() ? merged0.read() : merged1.read());
    }
  };
  template <size_t I, size_t N>
  struct merge_helper<I, N, N> {
    template <typename T> __attribute__((always_inline))
    static void merge(csa::stream<T>& out, csa::stream<T> (&in)[N]) {
      out.write(in[I].read());
    }
  };
  template <typename T, size_t N> __attribute__((always_inline))
  void merge(csa::stream<T>& out, csa::stream<T> (&in)[N]) {
    //TODO: The non-power of two will be handled
    static_assert(__builtin_popcount(N) == 1,
                  "Number of Streams to merge must be a power of two");
    return merge_helper<0, N, 1>::merge(out, in);
  }

  //The overloaded version of merge takes a variadic list of arguments
  //example: merge(lic_one, lic[0], lic[1], lic[2], lic[3]);
  template <typename T, class... In> __attribute__((always_inline))
  void merge(csa::stream<T>& out, In&... in) {
    csa::stream<T> in_array[sizeof...(in)] = { in... };
    return merge(out, in_array);
  }

  //Scatter is used to distribute data from one stream to different streams
  // this has been useful in cases where the conversion of strided
  // unrolled loop to a unit-strided loop is needed to gererate sld

  //the first version outputs an an array of LICs
  //note that the output is the second argument because in the variadic version,
  //the parameter pack has to be last
  //example: scatter(lic_one, lic);
  template <size_t Start, size_t N, size_t Stride>
  struct scatter_helper {
    template <typename T> __attribute__((always_inline))
    static void scatter(csa::stream<T>& in, csa::stream<T> (&out)[N]) {
      csa::stream<T> scattered0, scattered1;
      scatter_helper<Start, N, (Stride<<1)>::scatter(scattered0, out);
      scatter_helper<Start + Stride, N, (Stride<<1)>::scatter(scattered1, out);
      csa::stream<bool> vals;
      bool vals_out;
      //TODO: The seq should be replaced by a fountain
      //once late tools adds support for fountains
      __asm__ volatile
        (" seqne32 %%ign, %0, %%ign, %%ign, 0, 1, 1" : "=a"(vals_out));
      vals.write(vals_out);
      T a = in.read();
      (vals.read() ? scattered0.write(a) : scattered1.write(a));
    }
  };
  template <size_t I, size_t N>
  struct scatter_helper<I, N, N> {
    template <typename T> __attribute__((always_inline))
    static void scatter(csa::stream<T>& in, csa::stream<T> (&out)[N]) {
      out[I].write(in.read());
    }
  };
  template <typename T, size_t N> __attribute__((always_inline))
  void scatter(csa::stream<T>& in, csa::stream<T> (&out)[N]) {
    //TODO: The non-power of two will be handled
    static_assert(__builtin_popcount(N) == 1,
                  "Number of Streams to scatter must be a power of two");
    return scatter_helper<0, N, 1>::scatter(in, out);
  }

  //The overloaded version of scatter takes a variadic list of arguments
  //example: scatter(lic_one, lic[0], lic[1], lic[2], lic[3]);
  template <typename T, class... Out> __attribute__((always_inline))
  void scatter(csa::stream<T>& in, Out&... out) {
    stream<T> out_array[sizeof...(out)] =  { out... };
    scatter(in, out_array);
    return;
  }
#endif /*__CSA__*/
}

#endif /* __CSASTREAM_H */

