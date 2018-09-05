#QT += core
#QT -= gui

QT -= gui core widgets
LIBS   -= -lQtGui -lQtCore
CONFIG -= qt

CONFIG += c++11 debug
TARGET = FaceLib
#CONFIG += console
#CONFIG -= app_bundle
TEMPLATE = app

DEFINES += USE_CAFFE
DEFINES += USE_OPENCV

SOURCES += main.cpp \
    Tracker.cpp \
    Utils.cpp \
    FaceRecognition.cpp \
    Detector.cpp \
    Classifier.cpp \
    CaffeDetector.cpp \
    base64.cpp \
    Define.cpp \
    SocketIoUtils.cpp \
#    vwscapture/src/Buffer.cpp \
#    vwscapture/src/cvBufferCapture.cpp \
#    vwscapture/src/decoder.cpp \
#    vwscapture/src/libdownload.cpp \
#    vwscapture/src/util.cpp \
#    vwscapture/src/websocket.cpp \
#    vwscapture/src/wscapture.cpp \
#    vwscapture/src/wsconnection.cpp \
    StorageUtils.cpp \
    restclient-cpp/connection.cc \
    restclient-cpp/helpers.cc \
    restclient-cpp/restclient.cc \
    VideoReader.cpp

INCLUDEPATH += /home/phuclb/Documents/socket.io-client-cpp/build/include
INCLUDEPATH+= /usr/local/include
INCLUDEPATH+= /usr/local/cuda-9.2/include
INCLUDEPATH+= /usr/local/include/opencv2
INCLUDEPATH+= /usr/include
INCLUDEPATH+= /usr/include/hdf5/serial


DEFINES += DLIB_USE_CUDA
DEFINES += DLIB_USE_BLAS
DEFINES += DLIB_JPEG_SUPPORT
DEFINES += DLIB_PNG_SUPPORT


INCLUDEPATH += $$PWD/../dlib
INCLUDEPATH+= $$PWD/caffe/include  $$PWD/caffe/build/src
INCLUDEPATH+= $$PWD/vwscapture/include

CONFIG += link_pkgconfig
PKGCONFIG += opencv



QMAKE_CFLAGS += -fopenmp -Wpedantic -Wextra -Wfatal-errors -MMD -MP -pthread -fPIC -std=c++11 -O3 -Wall
QMAKE_LINK += -fopenmp -pthread -fPIC -std=c++11 -O3 -Wall
QMAKE_CXXFLAGS += -std=c++11

#LIBS += -L/home/vp9/workspace/FeatureExtract/dlib-19.6/build/dlib -ldlib
LIBS += -L$$PWD/../dlib/build/dlib -ldlib

LIBS += -L/usr/local/lib -L/usr/lib -L/usr/lib/x86_64-linux-gnu/hdf5/serial

LIBS += -L/usr/local/cuda-9.2/lib64

LIBS += -lcudart -lcublas -lcurand -lglog -lgflags #-lcaffe
LIBS += -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread -lboost_date_time -lm -lhdf5_hl -lhdf5 -lleveldb -lsnappy -llmdb
LIBS +=  -lstdc++ -lcudnn -lcblas -latlas
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_bgsegm -lopencv_imgcodecs -lopencv_videoio -lopencv_objdetect #-lopencv_calib3d

LIBS += -lpng -ljpeg
LIBS += -lcblas -llapack
LIBS += -lcurl -lpthread -ljsoncpp
#LIBS += -lavcodec -lavformat -lavutil -lswscale
LIBS += -lX11  -lcryptopp
LIBS += -L$$PWD/caffe/build/lib -lcaffe
LIBS += -L/home/phuclb/Documents/socket.io-client-cpp/build/lib/Release -lsioclient -lboost_system

LIBS += -laws-cpp-sdk-core -laws-cpp-sdk-s3
HEADERS += \
    Tracker.hpp \
    Utils.hpp \
    Configuration.hpp \
    Define.hpp \
    FaceRecognition.hpp \
    CaffeDetector.hpp \
    base64.h \
    SocketIoUtils.hpp \
#    vwscapture/include/vwscapture/Buffer.h \
#    vwscapture/include/vwscapture/cvBufferCapture.h \
#    vwscapture/include/vwscapture/decodebase.h \
#    vwscapture/include/vwscapture/decoder.h \
#    vwscapture/include/vwscapture/libdownload.h \
#    vwscapture/include/vwscapture/opencvheaders.h \
#    vwscapture/include/vwscapture/util.h \
#    vwscapture/include/vwscapture/websocket.h \
#    vwscapture/include/vwscapture/wsconnection.h \
#    vwscapture/include/wscapture.h \
    Data.hpp \
    StorageUtils.hpp \
    restclient-cpp/connection.h \
    restclient-cpp/helpers.h \
    restclient-cpp/restclient.h \
    restclient-cpp/version.h \
    VideoReader.hpp

