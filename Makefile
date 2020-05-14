#######################
# Makefile
#######################

# compile and lib parameter
CC      := g++
source  := /home/yyx/桌面/dlib-19.19/dlib/all/source.cpp
LIBS    := -lpthread -ljpeg -lpng -lX11
DEFINES := -DBUILD_DLL
INCLUDE := -I. 
CXXFLAGS:= -std=c++11

# link parameter
LIB := libfaceCompare.so

# build so file
$(LIB):
	$(CC) $(CXXFLAGS) ${source} dnn_face_recognition_ex.cpp $(INCLUDE) ${LIBS} ${DEFINES} -fPIC -shared -o ${LIB}

# clean
clean:
	rm -fr ${LIB}
	rm -rf test

# build a test
test:
	$(CC) $(CXXFLAGS) ${source} dnn_face_recognition_ex.cpp $(INCLUDE) ${LIBS} -o test

testlib:
	$(CC) $(CXXFLAGS) dnn_face_recognition_ex.cpp $(INCLUDE) -L. -lfaceCompare -o testlib

