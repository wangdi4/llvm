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
#ifdef __CSA__
    __attribute__((always_inline))
    stream() : identifier(__builtin_csa_lic_init(sizeof(T), depth, pref_depth))
    {}
#endif
    __attribute__((always_inline))
  T read() {
#ifdef __CSA__
      return __builtin_csa_lic_read((T)0, identifier);
#else
      return 0;
#endif
    }
    __attribute__((always_inline))
    void write(T val) {
#ifdef __CSA__
      __builtin_csa_lic_write(identifier, val);
#endif
    }
  };
}

#endif /* __CSASTREAM_H */

