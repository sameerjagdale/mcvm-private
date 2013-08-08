#ifndef MEX_PROTO_H
#define MEX_PROTO_H
#include<cstdlib>
#include"mxArray.h"
extern "C"{
//Memory Allocation
void* mxMalloc(size_t size);
void* mxCalloc(size_t size, int n);
void mxFree(void*);


//Dimension Setters
void mxSetM(mxArray*, mwSize M);
void mxSetN(mxArray*, mwSize N);
void mxSetDimensions(mxArray*, const mwSize*, mwSize ndims);

//Dimension extractors
size_t mxGetM(const mxArray*);
size_t mxGetN(const mxArray*);
mwSize* mxgetDimensions(const mxArray*);
mwSize mxGetNumberOfDimensions(const mxArray*);
size_t mxGetNumberOfElements(const mxArray*);

//type predicates
int mxIsCell(mxArray*mxarray);
int mxIsChar(mxArray*mxarray);
//TODO check octave
int mxIsClass(mxArray*mxarray, char *name);
int mxIsComplex(mxArray*mxarray);
int mxIsDouble(mxArray*mxarray);
int mxIsFunctionHandle(mxArray*mxarray);
int mxIsInt16(mxArray*mxarray);
int mxIsInt32(mxArray*mxarray);
int mxIsInt64(mxArray*mxarray);
int mxIsInt8(mxArray*mxarray);
int mxIsLogical(mxArray*mxarray);
int mxIsNumeric(mxArray*mxarray);
int mxIsSingle(mxArray*mxarray);
int mxIsSparse(mxArray*mxarray);
int mxIsStruct(mxArray*mxarray);

//size predicate
int mxIsEmpty(mxArray*mxarray);
int mxIsLogicalScalarTrue(const mxArray*ptr);
//data extractors
double* mxGetPr(const mxArray*mxarray);
double* mxGetPi(const mxArray*mxarray);
double mxGetScalar(const mxArray*mxarray);
mxChar* mxGetChars(const mxArray*mxarray);
mxLogical* mxGetLogical(const mxArray*mxarray);

void* mxGetData(mxArray*mxarray);
void* mxGetImagData(mxArray*mxarray);
//Data setters
void mxSetPr(mxArray*mxarray, double*data);
void mxSetPi(mxArray*mxarray, double*data);
void mxSetImagData(mxArray*mxarray, void*data);
void mxSetData(mxArray*mxarray, void*data);
//Constructors
mxArray* mxCreateCellArray(mwSize ndims, const mwSize*dims);
mxArray* mxCreateCellMatrix(mwSize m, mwSize n);
mxArray* mxCreateCharArray(mwSize ndims, const mwSize*dims);
mxArray* mxCreateCharMatrix(mwSize m, mwSize n);
mxArray* mxCreateCharMatrixFromStrings(mwSize m, const char**str);
mxArray* mxCreateDoubleMatrix(mwSize m, mwSize n, mxComplexity flag);
mxArray* mxCreateDoubleScalar(double val);
mxArray* mxCreateLogicalArray(mxLogical val);
mxArray* mxCreateLogicalMatrix(mwSize m, mwSize n);
mxArray* mxCreateLogicalScalar(mxLogical val);
mxArray* mxCreateNumericArray(mwSize ndims, mwSize* dims, mxClassID classId,
		mxComplexity flag);
mxArray* mxCreateNumericMatrix(mwSize m, mwSize n, mxClassID classId,
		mxComplexity flag);

mxArray* mxCreateString(const char *str);
mxArray* mxGetCell(const mxArray *ptr,mwIndex idx);
void mxSetCell(mxArray*ptr,mwIndex idx,mxArray* val);



//TODO once strutures are supported in McVM
//mxArray* mxCreateStructArray(mwSize ndims, const mwSize*dims, int num_keys,
//		const char** keys);
//mxArray* mxCreateStructMatrix(mwSize rows, mwSize cols, int num_keys,
//		const char** keys);
//mxArray* mxCreateSparse(mwSize m, mwSize n , mwSize nzmax, mxComplexity flag);
//mxArray* mxCreateSparseLogicalMatrix(mwSize m, mwSize n, mwSize nzmax);
//Interface to the interpreter
const char* mexFunctionName(void);
int mexPrintf(const char* fmt,...);
void mexMakeArrayPersistent(mxArray*ptr);
void mexMakeMemoryPersistent(void*ptr);
int mexAtExit(void(*f)(void));

void mexWarnMsgTxt(const char* str);
void mexWarnMsgIdAndTxt(const char*id, const char*s,... );
void mexErrMsgTxt(const char *s);
void mexErrMsgIdAndTxt(const char* id, const char *s,...);
int mexCallMATLAB(int nargout,mxArray* argout[],int nargin, mxArray* argin[],const char*fname);
//!<floating point predicates 
int mxIsFinite(double v);

int mxIsInf(double v);

int mxIsNaN(double v);

//!<Floating point values
double mxGetEps();
double mxGetInf();
double mxGetNaN();
void mexSetTrapFlag(int flag);
}
#endif
