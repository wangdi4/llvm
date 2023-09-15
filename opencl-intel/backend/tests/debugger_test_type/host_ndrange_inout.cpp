// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "host_program_common.h"
#include "test_utils.h"
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

using namespace std;

// Auxiliary function that extracts a "tuple" (arity 'ndim') from the comma
// separated string argument.
//
vector<size_t> get_arg_tuple(size_t ndim, const string &arg) {
  vector<string> toks = tokenize(arg, ",");
  if (toks.size() != ndim)
    throw "Argument error: invalid number of items in '" + arg + "'";

  vector<size_t> tuple;
  for (vector<string>::const_iterator i = toks.begin(); i != toks.end(); ++i) {
    tuple.push_back(atoi(i->c_str()));
  }
  return tuple;
}

// Turn any type supporting the ostream&<< operator into a string
//
template <typename T> string stringify(const T &x) {
  stringstream out;
  out << x;
  return out.str();
}

// Turn a vector of types supporting operator ostream&<< into a string with
// the given separator.
//
template <typename T>
string stringify_vector(const vector<T> &v, const string &sep = ",") {
  stringstream ss;
  typedef typename vector<T>::const_iterator vec_iterator;
  for (vec_iterator i = v.begin(); i != v.end(); ++i) {
    if (i != v.begin())
      ss << sep;
    ss << *i;
  }
  return ss.str();
}

// Convert a vector of size 1, 2 or 3 into an appropriate cl::NDRange object.
//
cl::NDRange vector2NDRange(const vector<size_t> &vec) {
  if (vec.size() == 1)
    return cl::NDRange(vec[0]);
  else if (vec.size() == 2)
    return cl::NDRange(vec[0], vec[1]);
  else if (vec.size() == 3)
    return cl::NDRange(vec[0], vec[1], vec[2]);

  assert(0 && "no can be");
  return cl::NullRange;
}

// Run a N-dimensional NDrange on the given kernel, with kernel arguments
// uchar* buf_in, uchar* buf_out.
//
// buf_in is initialized to a running sequence of 0, 1, ..., data_size - 1
//
// The extra arguments are:
//    <data_size> <ndim> <global_sizes> <local_sizes> <offsets>
//
// All arguments are optional, but if supplied have a fixed order.
//
// data_size:   Length of the buf_in and buf_out buffers. default: 1024
// ndim:        Number of dimensions in the NDRange. default: 1
//
// global_sizes, local_sizes, offsets:
//              Each of these is a comma-separated list of numbers. Their
//              amount must be 'ndim'.
//
// global_sizes: default: 32 per dimension
// local_sizes:  default: 1 per dimension
// offsets:      default: 0 per dimension
//
//
static void host_ndrange_inout_internal(cl::Context context, cl::Device device,
                                        cl::Program program,
                                        HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "main_kernel");
  cl::CommandQueue queue(context, device, 0);

  size_t data_size = 1024;
  if (extra_args.size() > 0) {
    // we have a 'data_size' argument
    data_size = atoi(extra_args[0].c_str());
  }

  size_t ndim = 1;
  if (extra_args.size() > 1) {
    // we have a 'ndim' argument
    ndim = atoi(extra_args[1].c_str());
    if (ndim < 1 || ndim > 3)
      throw runtime_error("Argument error: ndim not in range 1-3");
  }

  vector<size_t> global_sizes(ndim, 32);
  if (extra_args.size() > 2) {
    // we have a 'global_sizes' argument
    global_sizes = get_arg_tuple(ndim, extra_args[2]);
  }

  vector<size_t> local_sizes(ndim, 1);
  if (extra_args.size() > 3) {
    // we have a 'local_sizes' argument
    local_sizes = get_arg_tuple(ndim, extra_args[3]);
  }

  vector<size_t> offsets(ndim, 0);
  if (extra_args.size() > 4) {
    // we have a 'offsets' argument
    offsets = get_arg_tuple(ndim, extra_args[4]);
  }

  // Data for the input buffer
  vector<cl_uchar> databuf(data_size);
  for (size_t i = 0; i < data_size; ++i) {
    databuf[i] = static_cast<unsigned char>(i);
  }

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(cl_uchar) * data_size, &databuf[0], 0);
  kernel.setArg(0, buf_in);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(cl_uchar) * data_size,
                     0);
  kernel.setArg(1, buf_out);

  DTT_LOG("Executing kernel in NDRange...");
  DTT_LOG("  data_size = " + stringify(data_size));
  DTT_LOG("  ndim = " + stringify(ndim));
  DTT_LOG("  global_sizes = " + stringify_vector(global_sizes));
  DTT_LOG("  local_sizes = " + stringify_vector(local_sizes));
  DTT_LOG("  offsets = " + stringify_vector(offsets));

  queue.enqueueNDRangeKernel(kernel, vector2NDRange(offsets),
                             vector2NDRange(global_sizes),
                             vector2NDRange(local_sizes));
  queue.finish();
}

// Export
//
HostProgramFunc host_ndrange_inout = host_ndrange_inout_internal;
