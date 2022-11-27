#设置cmake版本号
cmake_minimum_required(VERSION 3.8)

project(udpTools VERSION 0.1.0)

# 设置C++的版本号
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

message(test-env-var-QTDIR -----------> $ENV{QTDIR})
set(CMAKE_PREFIX_PATH $ENV{QTDIR})

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") 
elseif(UNIX)
	set(CMAKE_PREFIX_PATH /home/zhoutk/Qt5.12.9/5.12.9/gcc_64)
endif()

#打开全局moc
set(CMAKE_AUTOMOC ON)
#打开全局uic
set(CMAKE_AUTOUIC ON)
#打开全局rcc
# set(CMAKE_AUTORCC ON)
ADD_DEFINITIONS(-DUNICODE)

message("Looking for Qt...")
# Qt modules (https://doc.qt.io/qt-5/qtmodules.html) you're using in your application
find_package(Qt5 REQUIRED Widgets)
# find_package(Qt5 COMPONENTS Widgets REQUIRED)

if (${Qt5_FOUND})
    message("Found Qt " ${Qt5_VERSION})
else()
    message("Couldn't find Qt")
endif()

#查找需要的Qt库文件，最好每一个库都要写，Qt也会根据依赖关系自动添加
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Network)

#递归查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB_RECURSE SRC_FILES "./src/*.cpp") 
#同理，递归查找当前文件夹中所有的头文件
FILE(GLOB_RECURSE HEAD_FILES "./src/*.h") 
#同理，递归查找当前文件夹中所有的ui文件
FILE(GLOB_RECURSE UI_FILES "./src/*.ui") 

FILE(GLOB_RECURSE QRC_SOURCE_FILES "./src/*.qrc")
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

source_group("Head Files" FILES ${HEAD_FILES} )
source_group("Form Files" FILES ${UI_FILES} )

# include(CTest)
# enable_testing()

#创建工程文件
add_executable(${PROJECT_NAME} ${QRC_FILES} ${SRC_FILES} ${HEAD_FILES} ${UI_FILES} )

#添加Qt5依赖项
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)
