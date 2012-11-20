// RUN: SATest -OCL -VAL --force_ref -neat=1 -config=%s.cfg
__kernel void minii( int in, int out)
{
     out = min(in, out);
}