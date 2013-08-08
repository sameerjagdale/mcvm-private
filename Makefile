INCLUDE = -I /usr/share/gettext/intl/ -I/home/2012/sjagda/boost/include/ -Ivendor/include -Ilib/include -I /home/2012/sjagda/llvm/include -I /home/2012/sjagda/CBLAS/include/ -I/home/2012/sjagda/lapack-3.4.2/lapacke/include -I source/ 
CXXFLAGS =   $(INCLUDE) -Wall -g -std=c++11 -Wno-deprecated
CXXFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DMCVM_USE_LAPACKE -Wfatal-errors 
LLVMLIBS = $(shell llvm-config --libfiles)
LIBS = -pthread  -ldl 
LIBS +=  vendor/lib/libgccpp.a  vendor/lib/libgc.a  /home/2012/sjagda/CBLAS/lib/cblas_LINUX.a /home/2012/sjagda/lapack-3.4.2/liblapacke.a /usr/lib/lapack/liblapack.so.3gf /usr/lib/libblas.so.3gf
CXX = g++-4.7 -rdynamic

all:	source/analysis_arraycopy.o source/analysis/boundcheck.o source/analysis/value/liveness.o source/analysis/value/value.o source/analysis/typeinference/analysisfw_typeinference.o source/analysis/typeinference/lattice.o source/analysis_boundscheck.o source/analysis_copyplacement.o source/analysis_livevars.o \
	source/analysismanager.o source/analysis_metrics.o source/analysis_reachdefs.o source/analysis_typeinfer.o source/arrayobj.o \
	source/assignstmt.o source/binaryopexpr.o source/cellarrayexpr.o source/cellarrayobj.o source/cellindexexpr.o \
	source/chararrayobj.o source/client.o source/clientsocket.o source/configmanager.o  source/endexpr.o \
	source/dotexpr.o source/environment.o source/expressions.o source/exprstmt.o source/filesystem.o source/fnhandleexpr.o source/functions.o source/ifelsestmt.o \
	source/interpreter.o source/jitcompiler.o source/lambdaexpr.o source/loopstmts.o source/main.o source/matrixexpr.o source/matrixobjs.o source/matrixops.o source/mcvm.o source/objects.o \
	source/paramexpr.o source/parser.o source/plotting.o source/process.o source/profiling.o source/rangeexpr.o source/rangeobj.o source/runtimebase.o source/mcvmstdlib.o source/stmtsequence.o \
	source/switchstmt.o source/symbolexpr.o source/transform_endexpr.o source/transform_logic.o source/transform_loops.o source/transform_split.o source/transform_switch.o source/lru-cache.o\
	source/mex_utility.o source/typeinfer.o source/unaryopexpr.o source/utility.o source/xml.o source/mex.o source/mxArray.o source/mexfunction.o source/interpreter_mex.o
	$(CXX) source/*.o source/analysis/*.o source/analysis/typeinference/*.o source/analysis/value/*.o  $(LLVMLIBS) $(LIBS)  -L/home/2012/sjagda/CBLAS/lib/ -L /home/2012/sjagda/boost/lib/ -o mcvm
clean:
	rm source/*.o mcvm
