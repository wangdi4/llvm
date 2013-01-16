# set project's name
set(TARGET KernelBuilder_module)

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

#
# We rely on find_package(QT4) to be called earlier in the project hierarcy
# 
 
# enable warnings
# ADD_DEFINITIONS( -Wall )
  
# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
# SET( QT_USE_QT3SUPPORT TRUE )   
# SET( QT_USE_QTXML TRUE )
 
  
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
                    ${OCL_SOURCE_DIR}/externals/boost_1_46_1 
                    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
                    ${OCL_SOURCE_DIR}/tools/ToolsUtils/
                    ${OCL_SOURCE_DIR}/tools/OfflineCompiler/iocQtgui
                    )
  

# here we instruct CMake to build "kernel builder" library from all of the source files
ADD_LIBRARY( ${TARGET} ${iocQtgui_SRCS} ${iocQtgui_MOC_SRCS})
SET_TARGET_PROPERTIES( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
