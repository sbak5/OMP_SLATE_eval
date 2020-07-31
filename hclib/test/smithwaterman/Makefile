include $(HCLIB_ROOT)/include/hclib.mak

EXE=smith_waterman

all: clean $(EXE) clean-obj

smith_waterman: smith_waterman.cpp
	$(CXX) -O2 -g -fopenmp  $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS) 

clean-obj:
	rm -rf *.o *.dSYM

clean:
	rm -rf *.o $(EXE) *.dSYM *.out
