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

/* Define the C++ interface for High-level LICs or streams on CSA */
namespace csa
{
  template<typename T, int depth = 0, int pref_depth = 0>
  class stream {
    int identifier;
  public:
    __attribute__((always_inline))
    stream() : identifier(__builtin_csa_lic_init(sizeof(T), depth, pref_depth))
    {}
    __attribute__((always_inline))
    T read() {
      return __builtin_csa_lic_read((T)0, identifier);
    }
    __attribute__((always_inline))
    void write(T val) {
      __builtin_csa_lic_write(identifier, val);
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
    __attribute__((always_inline))
    T rotate(int range, int low, int up, int step) {
      csa::stream<T> back;
      T val;
      if(range == low) val = this->read();
      if(range != low) val = back.read();
      if(range != up-step) back.write(val);
      return val;
    }
  };
}

#endif /* __CSASTREAM_H */

