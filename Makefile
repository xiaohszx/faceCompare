#######################
# Makefile
#######################

# compile and lib parameter
CC      := g++
LIBS    := ./libdlib.a
LDFLAGS :=
DEFINES :=
CUR_DIR = $(shell pwd)
INCLUDE := -I${CUR_DIR}
CFLAGS  := 
CXXFLAGS:= -std=c++11

# link parameter
LIB := faceCompare.so

#link
$(LIB):dnn_face_recognition_ex.o
	$(CC) -shared -o -fPIC -o $@ $^ 
#compile
dnn_face_recognition_ex.o:dnn_face_recognition_ex.cpp 
	$(CC) -c -fPIC $^ $(CXXFLAGS) $(INCLUDE) -o $@  -L $(LIBS)

# clean
clean:
	rm -fr *.o

