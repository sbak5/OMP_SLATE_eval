HCLIB_ROOT=$(PWD)/../hclib/hclib-install
include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.post.mak

CDIR=$(HCLIB_ROOT)#$(HOME)/build_convomp/multicore-linux64-omp-icc#$(CharmDIR)
CXX=g++ $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) #$(CDIR)/bin/charmc #$(OPTS)
INC=-g -O2 -I../src -I../../build/runtime/src -fPIC -std=c++11
LIB=$(CDIR)/lib/libhclib_omp.so
OMPOBJ=$(shell find ../../build/runtime/src/CMakeFiles/omp.dir -name "*.o")

LIBS=$(HCLIB_LDLIBS)#-L$(CDIR)/lib -lhclib -lhclib_system #-L$(CDIR)/lib -lcharm

all: $(LIB)  
#$(MODULE).decl.h
$(LIB): $(OMPOBJ)
	$(CXX) $(INC) -shared -o $@ $^ $(LIBS) -lpthread #-language c++ 
clean:
	rm -f *.o *.decl.h *.def.h $(LIB) 
