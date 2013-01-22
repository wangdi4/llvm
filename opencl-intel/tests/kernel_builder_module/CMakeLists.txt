# set project's name

set(TARGET KernelBuilder_module)


if (CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BOOST_LIB_DIR_SUFFIX x64)
else (CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BOOST_LIB_DIR_SUFFIX x86)
endif (CMAKE_SIZEOF_VOID_P MATCHES "8")


link_directories( ${Boost_LIBRARY_DIRS}/${BOOST_LIB_DIR_SUFFIX} )

# with SET() command you can change variables or define new ones
# here we define SAMPLE_SRCS variable that contains a list of all .cpp files
# note that we don't need \ at the end of line
SET( iocQtgui_SRCS
      ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/buildoptionsdialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/finddialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/gotolinedialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocmainwindow.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocopencleditor.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioctab.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioctabwidget.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/main.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/optionsdialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iochighlighter.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioclinkfilesdialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/kernelanalyzerfacade.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelAnalyzer.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelAnalyzerCache.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelArgument.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KBDeviceInfo.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/variablecreatordialog.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/variables.cpp
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/showcontentdialog.cpp
     #${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocicon.rc
     #${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocgui.rc
)
  
# another list, this time it includes all header files that should be treated with moc
SET( iocQtgui_MOC_HDRS
      ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/buildoptionsdialog.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/definitions.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/finddialog.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/gotolinedialog.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocmainwindow.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocopencleditor.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioctab.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioctabwidget.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/optionsdialog.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iochighlighter.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/ioclinkfilesdialog.h   
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/kernelanalyzerfacade.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelAnalyzer.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelAnalyzerCache.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KernelArgument.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/KBDeviceInfo.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/variablecreatordialog.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/variables.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/resource.h
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/showcontentdialog.h
)
  
# some .ui files
# SET( IOCQtGUI_UIS
     # ./src/ui/Dialog1.ui
     # ./src/ui/Dialog2.ui
# )
  
# and finally an resource file
SET( iocQtgui_RCS
     ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui/iocresources.qrc
 )
  
# enable warnings
# ADD_DEFINITIONS( -Wall )
  
# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
# SET( QT_USE_QT3SUPPORT TRUE )   
# SET( QT_USE_QTXML TRUE )
  
# this command finds Qt4 libraries and sets all required variables
# note that it's Qt4, not QT4 or qt4
FIND_PACKAGE( Qt4 REQUIRED )
  
# add some useful macros and variables
# (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
INCLUDE( ${QT_USE_FILE} )
  
# this command will generate rules that will run rcc on all files from SAMPLE_RCS
# in result SAMPLE_RC_SRCS variable will contain paths to files produced by rcc
QT4_ADD_RESOURCES( iocQtgui_RC_SRCS ${iocQtgui_RCS} )
  
# this will run uic on .ui files:
# QT4_WRAP_UI( SAMPLE_UI_HDRS ${SAMPLE_UIS} )
  
# and finally this will run moc:
QT4_WRAP_CPP( iocQtgui_MOC_SRCS ${iocQtgui_MOC_HDRS} )
  
# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
include_directories(
					${CMAKE_BINARY_DIR}
                    ${OCL_GLOBAL_INCLUDE}
                    ${OCL_SOURCE_DIR}/cl_api
					${Boost_INCLUDE_DIR}
					${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
					${OCL_SOURCE_DIR}/tools/ToolsUtils/
					${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui
					)
  

# here we instruct CMake to build "kernel builder" library from all of the source files
ADD_LIBRARY( ${TARGET} ${iocQtgui_SRCS} ${iocQtgui_MOC_SRCS})
  
if(WIN32)
	add_dependencies( ${TARGET}
					OpenCL
					cl_sys_utils
					ToolsUtils
					)
	if(BUILD_X64)
	  SET( QT_LIBS debug ${OCL_SOURCE_DIR}/externals/Qt/Win64/$(Configuration)/QtGuid4.lib
				     debug ${OCL_SOURCE_DIR}/externals/Qt/Win64/$(Configuration)/QtCored4.lib
                     optimized ${OCL_SOURCE_DIR}/externals/Qt/Win64/$(Configuration)/QtGui4.lib
				     optimized ${OCL_SOURCE_DIR}/externals/Qt/Win64/$(Configuration)/QtCore4.lib )
        SET( SHARED_DEBUG_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/QtGuid4.dll 
                               ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/QtCored4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/Qt3Supportd4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/QtNetworkd4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/QtXmld4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win64/Debug/QtSqld4.dll )
        SET( SHARED_RELEASE_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/QtGui4.dll 
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/QtCore4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/Qt3Support4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/QtNetwork4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/QtXml4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win64/Release/QtSql4.dll )
	else(BUILD_X64)
        SET( QT_LIBS debug ${OCL_SOURCE_DIR}/externals/Qt/Win32/$(Configuration)/QtGuid4.lib
				     debug ${OCL_SOURCE_DIR}/externals/Qt/Win32/$(Configuration)/QtCored4.lib
                     optimized ${OCL_SOURCE_DIR}/externals/Qt/Win32/$(Configuration)/QtGui4.lib
				     optimized ${OCL_SOURCE_DIR}/externals/Qt/Win32/$(Configuration)/QtCore4.lib )
        SET( SHARED_DEBUG_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/QtGuid4.dll 
                               ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/QtCored4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/Qt3Supportd4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/QtNetworkd4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/QtXmld4.dll
                               ${OCL_SOURCE_DIR}/externals/Qt/Win32/Debug/QtSqld4.dll )
        SET( SHARED_RELEASE_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/QtGui4.dll 
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/QtCore4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/Qt3Support4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/QtNetwork4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/QtXml4.dll
                                 ${OCL_SOURCE_DIR}/externals/Qt/Win32/Release/QtSql4.dll )

  endif(BUILD_X64)
    INSTALL(FILES ${SHARED_DEBUG_LIBS} DESTINATION bin CONFIGURATIONS Debug)
    INSTALL(FILES ${SHARED_RELEASE_LIBS} DESTINATION bin CONFIGURATIONS Release)
else(WIN32)
  SET( SHARED_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtCore.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtCore.so.4.8.1
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtGui.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtGui.so.4.8.1 )
  SET( SQUISH_LIBS ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQt3Support.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQt3Support.so.4.8.1
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtNetwork.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtNetwork.so.4.8.1
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtSql.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtSql.so.4.8.1
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtXml.so.4
                   ${OCL_SOURCE_DIR}/externals/Qt/Linux/libQtXml.so.4.8.1 )

	SET( QT_LIBS ${SHARED_LIBS} )
  INSTALL(FILES ${SHARED_LIBS} DESTINATION bin)
  INSTALL(FILES ${SQUISH_LIBS} DESTINATION bin)
endif(WIN32)
# last thing we have to do is to tell CMake what libraries our executable needs,
# luckily FIND_PACKAGE prepared QT_LIBRARIES variable for us:
TARGET_LINK_LIBRARIES( ${TARGET} OpenCL ToolsUtils cl_sys_utils ${QT_LIBS} )
SET_TARGET_PROPERTIES( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
#linking flags
if(WIN32)
	if(BUILD_X64)
		SET_TARGET_PROPERTIES(${TARGET} PROPERTIES STATIC_LIBRARY_FLAGS  "/MACHINE:X64")
	else(BUILD_X64)
		SET_TARGET_PROPERTIES(${TARGET} PROPERTIES STATIC_LIBRARY_FLAGS  "/MACHINE:X86")
	endif(BUILD_X64)
endif(WIN32)

INSTALL(TARGETS ${TARGET} DESTINATION bin )
