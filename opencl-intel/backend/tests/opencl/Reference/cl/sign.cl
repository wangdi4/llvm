kernel void
test(global float2 *f2in, global float3 *f3in, global float4 *f4in,
     global float8 *f8in, global float16 *f16in,

     global double2 *d2in, global double3 *d3in, global double4 *d4in,
     global double8 *d8in, global double16 *d16in,

     global float2 *f2out, global float3 *f3out, global float4 *f4out,
     global float8 *f8out, global float16 *f16out,

     global double2 *d2out, global double3 *d3out, global double4 *d4out,
     global double8 *d8out, global double16 *d16out) {
  size_t i = get_global_id(0);
  f2out[i] = sign(f2in[i]);
  f3out[i] = sign(f3in[i]);
  f4out[i] = sign(f4in[i]);
  f8out[i] = sign(f8in[i]);
  f16out[i] = sign(f16in[i]);
  d2out[i] = sign(d2in[i]);
  d3out[i] = sign(d3in[i]);
  d4out[i] = sign(d4in[i]);
  d8out[i] = sign(d8in[i]);
  d16out[i] = sign(d16in[i]);
}
