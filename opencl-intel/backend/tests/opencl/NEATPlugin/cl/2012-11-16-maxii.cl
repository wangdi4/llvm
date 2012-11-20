// RUN: SATest -OCL -VAL --force_ref -neat=1 -config=%s.cfg
__kernel void maxii( int in, int out)
{
     out = max(in, out);
}