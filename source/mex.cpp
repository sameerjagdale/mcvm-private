/*
 * mex.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: sameer
 */
#include<cstring>
#include<cstdlib>
#include<gc.h>
#include"mexproto.h"
#include"mexfunction.h"
#include"mxArray.h"
using namespace std;

void* mxMalloc(size_t size) {
	return GC_MALLOC(size);
}
void mxFree(void*ptr) {
	GC_FREE(ptr);
}
void* mxCalloc(size_t size, int n) {
	void* temp = GC_MALLOC(size*n);
	return memset(temp, 0, n);
}
void setPr(mxArray * mxarray, double*pr) {
	mxarray->setPr(pr);

}

void mxSetM(mxArray* mxarray, mwSize M) {
	mxarray->setM(M);
}
void mxSetN(mxArray*mxarray, mwSize N) {
	mxarray->setN(N);
}
void mxSetDimensions(mxArray*mxarray, const mwSize*dims, mwSize ndims) {
	mxarray->setDimensions(dims, ndims);
}

//Dimension extractors
size_t mxGetM(const mxArray*mxarray) {
	return mxarray->getM();
}
size_t mxGetN(const mxArray*mxarray) {
	return mxarray->getN();
}
mwSize* mxGetDimensions(const mxArray*mxarray) {
	return mxarray->getDimensions();
}
mwSize mxGetNumberOfDimensions(const mxArray*mxarray) {
	return mxarray->getNumberOfDimensions();
}
size_t mxGetNumberOfElements(const mxArray*mxarray) {
	return mxarray->getNumberOfElements();
}

//Type Predicates
int mxIsCell(mxArray*mxarray) {
	return mxarray->isCell();
}
int mxIsChar(mxArray*mxarray) {
	return mxarray->isChar();
}
//int mxIsClass(mxArray*mxarray, char *name);
int mxIsComplex(mxArray*mxarray) {
	return mxarray->isComplex();
}
int mxIsDouble(mxArray*mxarray) {
	return mxarray->isDouble();
}
int mxIsFunctionHandle(mxArray*mxarray) {
	return mxarray->isFunctionHandle();
}
int mxIsInt16(mxArray*mxarray){
	return mxarray->isInt16();
}
int mxIsInt32(mxArray*mxarray) {
	return mxarray->isInt32();
}
int mxIsInt64(mxArray*mxarray){
	return mxarray->isInt64();
}
int mxIsInt8(mxArray*mxarray){
	return mxarray->isInt8();
}
int mxIsLogical(mxArray*mxarray) {
	return mxarray->isLogical();
}
int mxIsNumeric(mxArray*mxarray) {
	return mxarray->isNumeric();
}
int mxIsSingle(mxArray*mxarray) {
	return mxarray->isSingle();
}
//int mxIsSparse(mxArray*mxarray);
int mxIsStruct(mxArray*mxarray) {
	return mxarray->isStruct();
}

//size predicate
int mxIsEmpty(mxArray*mxarray) {
	return mxarray->isEmpty();
}
int mxIsLogicalScalarTrue(mxArray*mxarray){
	return mxarray->isScalar()&&mxarray->isLogical();
}
//data extractors
double* mxGetPr( const mxArray*mxarray) {
	return mxarray->getPr();
}
double* mxGetPi( const mxArray*mxarray) {
	return mxarray->getPi();
}
double mxGetScalar( const mxArray*mxarray) {
	return mxarray->getScalar();
}
mxChar* mxGetChars( const mxArray*mxarray) {
	return mxarray->getChars();
}
mxLogical* mxGetLogical( const mxArray*mxarray) {
	return mxarray->getLogical();
}

void* mxGetData( const mxArray*mxarray) {
	return mxarray->getData();
}
void* mxGetImagData( const mxArray*mxarray) {
	return mxarray->getImagData();
}
//Data setters
void mxSetPr(mxArray*mxarray, double*data) {
	mxarray->setPr(data);
}
void mxSetPi(mxArray*mxarray, double*data) {
	mxarray->setPi(data);
}
void mxSetImagData(mxArray*mxarray, void*data) {
	mxarray->setImagData(data);
}
void mxSetData(mxArray*mxarray, void*data) {
	mxarray->setData(data);
}
mxArray* mxCreateCellArray(mwSize ndims, const mwSize*dims){
	return  new mxArray(ndims,dims,mxREAL,mxCELL_CLASS);
}
mxArray* mxCreateCellMatrix(mwSize m, mwSize n){
	return mxCreateNumericMatrix(m,n,mxCELL_CLASS,mxREAL);
}
mxArray* mxCreateCharArray(mwSize ndims, const mwSize*dims){
	return new mxArray(ndims,dims,mxREAL,mxCHAR_CLASS);
}
mxArray* mxCreateCharMatrix(mwSize m, mwSize n){
	return mxCreateNumericMatrix(m,n,mxCHAR_CLASS,mxREAL);
}	
mxArray* mxCreateDoubleMatrix(mwSize m, mwSize n, mxComplexity flag){
	return mxCreateNumericMatrix(m,n,mxDOUBLE_CLASS,flag);
}
//mxArray* mxCreateDoubleScalar(double val);
mxArray* mxCreateLogicalArray(mwSize ndims,mwSize *dims){
	return mxCreateNumericArray(ndims, dims,mxLOGICAL_CLASS,mxREAL);
}
mxArray* mxCreateLogicalMatrix(mwSize m, mwSize n){
	return mxCreateNumericMatrix(m,n,mxLOGICAL_CLASS,mxREAL); 
}
//mxArray* mxCreateLogicalScalar(mxLogical val);
mxArray* mxCreateNumericArray(mwSize ndims,  mwSize* dims, mxClassID classId,
		mxComplexity flag){
	return new mxArray(ndims,dims, flag, classId);
}
mxArray* mxCreateNumericMatrix(mwSize m, mwSize n, mxClassID classId,mxComplexity complexity){
	return new mxArray(m,n,complexity,classId);
}
//Destructor
void mxDestroyArray(mxArray *array){
	delete array;
}
//Copy Constructor
mxArray* mxDuplicateArray(const mxArray*v){
	return const_cast<mxArray*>(v)->copy();
}
mxArray* mxGetCell(const mxArray *ptr,mwIndex idx){
        return const_cast<mxArray*>(ptr)->getCell(idx);
}
void mxSetCell(mxArray*ptr,mwIndex idx,mxArray* val){
        ptr->setCell(idx,val);
}
//Interface to the Interpreter
//extern MexFunction *currentFunc;
/*const char* mexFunctionName(){
	if(currentFunc!=NULL){
		return (const char*)currentFunc->getFuncName().c_str();
	}
	else{
#ifdef DEBUG
		std::cout<<"Mexfunction pointer no assigned"<<std::endl;
#endif	
		return NULL;
	}
}*/	
