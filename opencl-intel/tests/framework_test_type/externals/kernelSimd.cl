__kernel void clIntelOfflineCompilerTest(__global int* i)
{
	long2 vec1 = (1,2);
	long2 vec2 = (-3,14);
	ulong2 diff = abs_diff(vec1,vec2);
	i[0] = (int) diff.x + diff.y;
}
