#CDIR=$(shell ./readlink.sh ../../../..)
#Compiler setting

# CHARM
#CHARMC="$(HOME)/build_convomp/bin/charmc"

#INTEROP_CXX=$(CHARMC)
#INTEROP_CC=$(CHARMC)

#override OPTS+=-seq -language c++ -DCONV_OMP=1 -fPIC -parallel #for icc
#CFLAGS="$(OPTS) -DCONV_OMP=1"
#CXXFLAGS="$(OPTS) -DCONV_OMP=1"

#export CharmDIR=$(CDIR)
 
#HCLIB_OMP
HCLIB_ROOT=$(PWD)/../hclib/hclib-install
INTEROP_CC=gcc#$(CC)
INTEROP_CXX=g++#$(CXX)
BASE_OPTS=-O2 -fPIC 
OPTS=${BASE_OPTS} -DHCLIB_OMP=1 -I$(HCLIB_ROOT)/include

all: llvm_omp org ompcharm

org: omp_org

llvm_omp: omp

omp:
	if [ ! -d build ]; then mkdir build; fi;\
	cd build;\
	if [ ! -f ./Makefile ]; then cmake ../ -DCMAKE_C_COMPILER="$(INTEROP_CC)" -DCMAKE_CXX_COMPILER="$(INTEROP_CXX)" -DCMAKE_BUILD_TYPE=Release -DLIBOMP_USE_INTEROP=1 -DCMAKE_C_FLAGS="$(OPTS)" -DCMAKE_CXX_FLAGS="$(OPTS)"; fi; cd ../; \
	make $(MFLAGS) -C ./build

omp_org:
	if [ ! -d build_org ]; then mkdir build_org; fi;\
	cd build_org;\
	if [ ! -f ./Makefile ]; then cmake ../ -DCMAKE_C_COMPILER="$(INTEROP_CC)" -DCMAKE_CXX_COMPILER="$(INTEROP_CXX)" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="$(BASE_OPTS)" -DLIBOMP_ENABLE_SHARED=1 -DCMAKE_CXX_FLAGS="$(BASE_OPTS)"; fi; cd ../; \
	make $(MFLAGS) -C ./build_org


ompcharm: llvm_omp
	make $(MFLAGS) -C runtime/$@

clean:
	rm -rf build build_org runtime/ompcharm/*.o runtime/ompcharm/*.def.h runtime/ompcharm/*.decl.h build
