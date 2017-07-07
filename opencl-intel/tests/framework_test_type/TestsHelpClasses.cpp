#include "TestsHelpClasses.h"

extern cl_device_type gDeviceType;

cl_device_id baseOcl::device;
cl_platform_id baseOcl::platform;
cl_context baseOcl::context;
cl_command_queue baseOcl::cmd_queue;


void baseOcl::SetUpTestCase()
{
	ASSERT_NO_FATAL_FAILURE(initPlatform());
	ASSERT_NO_FATAL_FAILURE(initDevices());
	ASSERT_NO_FATAL_FAILURE(initContext());
	ASSERT_NO_FATAL_FAILURE(initCommandQueue());
}

void baseOcl::TearDownTestCase()
{
	NullCheckAndExecute(cmd_queue,clReleaseCommandQueue(cmd_queue));
	NullCheckAndExecute(context,clReleaseContext(context));
}

baseOcl::baseOcl()
{

}

baseOcl::~baseOcl()
{

}

void baseOcl::TearDown()
{
	NullCheckAndExecute(kernel,clReleaseKernel(kernel));
	NullCheckAndExecute(program,clReleaseProgram(program));	
}




void baseOcl::initPlatform()
{
	//init platform
	cl_int err=clGetPlatformIDs(1,&platform,NULL);
	ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clGetPlatformIDs");
}

void baseOcl::initDevices()
{
	// init Devices (only one CPU...)
	cl_int err=clGetDeviceIDs(platform,gDeviceType,1,&device,NULL);
	ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clGetDeviceIDs");
}

void baseOcl::initContext()
{
	cl_int err = ERROR_RESET;
	//init context
	context=clCreateContext(NULL,1,&device,NULL,NULL,&err);
	ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clCreateContext");
}

void baseOcl::initCommandQueue()
{
	cl_int err = ERROR_RESET;
	//init Command Queue
	cmd_queue=clCreateCommandQueue(context,device,0,&err);
	ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clCreateCommandQueue");
}

void baseOcl::simpleProgramCreation( const char **kernelCode )
{
	cl_err_code err = ERROR_RESET;

	program = clCreateProgramWithSource(context, 1, kernelCode, NULL, &err);
	ASSERT_OCL_SUCCESS(err,clCreateProgramWithSource);
}

void baseOcl::simpleProgramBuild()
{
	cl_err_code err = ERROR_RESET;

	ASSERT_FALSE(program == NULL);
	err = clBuildProgram(program,0,NULL,NULL,NULL,NULL);

	cl_build_status build_status=ERROR_RESET;
	err|= clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_STATUS,MAX_SOURCE_SIZE,&build_status,NULL);	

	if (err!=CL_SUCCESS || build_status==CL_BUILD_ERROR){
		cout << "build failed, status is: " << build_status << endl;
		char err_str[MAX_SOURCE_SIZE];	// instead of dynamic allocation
		char* err_str_ptr=err_str;
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,MAX_SOURCE_SIZE,err_str_ptr,NULL);
		if (err!=CL_SUCCESS)
			cout << "Build Info error: " << oclErr(err) << endl;
		cout << err_str_ptr << endl;
		FAIL();
	}
}

void baseOcl::simpleKernelCreation()
{
	cl_err_code err = ERROR_RESET;

	ASSERT_FALSE(program == NULL);
	err = clCreateKernelsInProgram(program,sizeof(kernel),&kernel,NULL);
	ASSERT_OCL_SUCCESS(err,clCreateKernelsInProgram);
}
inline bool checkFileExistence( char* fileName )
{
	FILE* pIRfile = NULL;
	if ( 0 == crossPlatformFOpen(fileName,"r", pIRfile) ){ 
		fclose(pIRfile); //file already exists
		return true;
	}
	return false;
}
int crossPlatformFOpen( char* fileName, const char* optns, FILE* file )
{
	 int err=ERROR_RESET;
#ifdef _WIN32	
	err=FOPEN(file,fileName,optns); 
#else
	FOPEN(file,fileName,optns); 
	err = errno;
	if (file != NULL)
	{
		err = 0;
	}
#endif
	return err;
}

void removeFiles(string files[], int num){
	for (int i = 0; i<num; i++){
		remove(files[i].c_str());
	}
}
void runIoc(string arguments){
	std::string execString;
	std::string archType = (IS_64_BIT) ? "64" : "32";

	execString = "ioc";
	execString += archType + " ";
	execString += arguments;

	system(execString.c_str());
}

void validateEqualityOfFiles(string fileName1, string fileName2, bool isEqual,int linesToSkip){
	string const files = "The Files " + fileName1 + " and " + fileName2 ;
	ifstream file1(fileName1.c_str());
	ASSERT_TRUE(file1.is_open()) << "file " + fileName1 + " does not exist";
	ifstream file2(fileName2.c_str());
	ASSERT_TRUE(file2.is_open()) << "file " + fileName2 + " does not exist";
	string line1,line2;
	int i;
	for (i = 1 ; getline(file1, line1) && getline(file2, line2); i++){
		if ( i - 1 < linesToSkip)
			continue;
		if (0 != line1.compare(line2) ){
			if (isEqual){
				FAIL() << files << " are different on line number " << i << "\n"  + line1 + "\n vs\n" + line2; 	
				break;
			} else {
				file1.close();
				file2.close();
				return;
			}
		}
	}
	if ( getline(file1,line1) || getline(file2, line2) ){
		if ( isEqual){
			FAIL() << files << " are not the same, they even don't have the same number of lines";
		}
	} else if ( !isEqual){
		FAIL() << files << " are the same, they contain " << i - 1 << " lines" ;
	}
	file1.close();
	file2.close();
}

void validateSubstringInFile(string fileName, string subString, bool doesExist){
	string const theFile = "The File " + fileName;
	ifstream file(fileName.c_str());
	ASSERT_TRUE(file.is_open()) << theFile + " does not exist";
	string line;
	int i;
	for (i = 1; file.good(); i++){
		getline(file, line);
		if (string::npos != line.find(subString)){
			if (doesExist){
				file.close();
				return;
			} else {
				FAIL() << theFile + "contains the string " + subString + " in line number" << i << "which is:\n" + line;
				break;
			}
		}
	}
	if ( doesExist){
		FAIL() << theFile + " does not contain the string " + subString + " , it is " << i + 1 << " lines long";
	}
	file.close();
}




oclErr::oclErr( cl_err_code errCode_ )
{
	errCode = errCode_;
	err = ClErrTxt(errCode);
}

string oclErr::gerErrString() const
{
	return err;
}

bool oclErr::operator==( const oclErr& other ) const
{
	return errCode == other.errCode;
}

::std::ostream& operator<<(::std::ostream& os, const oclErr& OclErr) 
{
	return os << OclErr.gerErrString();
}

::std::ostream& operator<<(::std::ostream& os, const clMemWrapper& mem) 
{
	return mem.print(os);
}

bool operator==(const int num, const clMemWrapper& mem){
	return mem == num;
}

Ocl2DImage::Ocl2DImage( size_t size_, bool Random /*= false*/ )
{
	init(size_, Random);
	if (Random == false){
		memset(pImage,0,Size);
	}
}

Ocl2DImage::Ocl2DImage( const Ocl2DImage &copy )
{
	init(copy.Size, false);
	memcpy(pImage,copy.pImage,copy.Size);
}

Ocl2DImage::Ocl2DImage( int numOfImages, const Ocl2DImage &copy )
{
	init(copy.Size*numOfImages, false);
	for ( int i = 0; i<numOfImages; i++){
		memcpy(pImage + i*copy.Size,copy.pImage,copy.Size);
	}
}

void Ocl2DImage::init( size_t size_, bool Random )
{
	Size = size_;
	pImage = new cl_uchar[Size];
	if (Random == false){
		return;
	}
	for( int i = 0; i < (int)Size; i++ )
	{
		pImage[i] = (cl_uchar)( rand() & 255 );
	}
}

Ocl2DImage::~Ocl2DImage( void )
{
	delete[] pImage;
}


Ocl2DImage& Ocl2DImage::operator=( const Ocl2DImage &other )
{
	if (this == &other)
		return *this;
	NullCheckAndExecute(pImage,delete[] pImage);
	Size = other.Size;
	init(other.Size, false);
	memcpy(pImage,other.pImage,other.Size);
	return *this;
}
bool Ocl2DImage::operator==( const Ocl2DImage &other ) const
{
	if (Size != other.Size){
		return false;
	}
	return memcmp(pImage, other.pImage, Size) == 0 ? true : false;
}

