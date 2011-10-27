tests: src/*.cpp src/*/*.cpp src/*/*.h
	g++ \
	-I/System/Library/Frameworks/OpenCL.framework/Versions/A/Headers \
	-L/System/Library/Frameworks/OpenCL.framework/Versions/A/Libraries \
	-lz -framework OpenCL \
	src/*.cpp src/*/*.cpp -o tests

 