/*
 * mxArray.cpp
 *
 *  Created on: Jun 18, 2013
 *      Author: sameer
 */
#include "mxArray.h"
//Dimension Setters
void mxArray::setM(mwSize M) {
	dataobject->setM(M);
}
void mxArray::setN(mwSize N) {
	dataobject->setN(N);
}
void mxArray::setDimensions(const mwSize*dims, mwSize ndims) {
	dataobject->setDimensions(const_cast<long int*>((const long int*) dims),
			ndims);
}

//Dimension extractors
size_t mxArray::getM() const{
	DimVector dim=dataobject->getDataSize();
	#ifdef DEBUG
	std::cout<<"mxArray.getM() invoked "<<std::endl;
	#endif
	return dataobject->getDataSize()[0];
}
size_t mxArray::getN() const {
	return dataobject->getDataSize()[1];
}
mwSize* mxArray::getDimensions() const {
	DimVector vec=dataobject->getDataSize();
	mwSize *dims=new mwSize[vec.size()];
	for(int i=0;i<vec.size();i++){
		dims[i]=vec[i];
	}
	return dims;
}
mwSize mxArray::getNumberOfDimensions() const{
	return dataobject->getNumDims();
}
size_t mxArray::getNumberOfElements() const {
	return dataobject->getNumElems();
}

int mxArray::isCell() {
	return dataobject->getType() == DataObject::CELLARRAY;
}
int mxArray::isChar() {
	return dataobject->getType() == DataObject::CHARARRAY;
}
/*int isClass(char *name){

 }*/
int mxArray::isComplex() {
	return dataobject->getType() == DataObject::MATRIX_C128;
}
//TODO check in octave
int mxArray::isDouble() {
	return dataobject->getType() == DataObject::MATRIX_F64;
}

int mxArray::isFunctionHandle() {
	return dataobject->getType() == DataObject::FN_HANDLE;
}
int mxArray::isInt16(){
 return dataobject->getType()==DataObject::MATRIX_I16;
 }
int mxArray::isInt32() {
	return dataobject->getType() == DataObject::MATRIX_I32;
}
int mxArray::isInt64(){
	return dataobject->getType()==DataObject::MATRIX_I16;
}
int mxArray::isInt8(){
	return dataobject->getType()==DataObject::MATRIX_I8;
}
int mxArray::isLogical() {
	return dataobject->getType() == DataObject::LOGICALARRAY;
}
int mxArray::isNumeric() {
	return dataobject->getType() <= DataObject::MATRIX_C128
			&& dataobject->getType() >= DataObject::MATRIX_I32;
}
int mxArray::isSingle() {
	return dataobject->getType() == DataObject::MATRIX_F32;
}


//TODO: Once sparse is supported in McVM
//int isSparse();


int mxArray::isStruct() {
	return dataobject->getType() == DataObject::STRUCT_INST;
}

//size predicates
int mxArray::isEmpty() {
	return (int) dataobject->isEmpty();
}
int mxArray::isScalar(){
	return (int) dataobject->isScalar();
}

//data extractors
double* mxArray::getPr()const   {
	return static_cast<double*>(dataobject->getData());
}
double* mxArray::getPi() const{
	return static_cast<double*>(dataobject->getImagData());
}
double mxArray::getScalar() const {
	return dataobject->getScalarDouble();
}
mxChar* mxArray::getChars()const  {
	return static_cast<mxChar*>(dataobject->getData());
}
mxLogical* mxArray::getLogical()const  {
	return static_cast<mxLogical*>(dataobject->getData());
}

void* mxArray::getData() const {
	return dataobject->getData();
}
void* mxArray::getImagData() const {
	return dataobject->getImagData();
}

//Data setters
void mxArray::setPr(double*data) {
	dataobject->setData(data);
}
void mxArray::setPi(double*data) {
	dataobject->setImagData(data);
}
void mxArray::setImagData(void*data) {
	dataobject->setImagData(data);
}
void mxArray::setData(void*data) {
	dataobject->setData(data);
}

mxArray* mxArray::copy(){
	return new mxArray(dataobject->copy(),id,complexFlag); 
}
DataObject* mxArray::getDataObject(){
	return dataobject;
}
void mxArray::setDataObject(DataObject*obj){
	dataobject=obj;
}

int mxArray::getString(char* buf,mwSize strlen){
	int flag=0;
	if(!isChar()){
		return 1;	
	}
	size_t size =getM()*getN();
	if(size>(strlen-1)){
		flag=1;
	}
	memcpy(buf,getData(),(strlen-1)*sizeof(char));
	buf[strlen]='\0';
	return flag;
}
char* mxArray::mxArrayToString(){
	if(!isChar()){
		return NULL;	
	}
	//FIXME: replace with specialised function for char*
	return (char*)getData();
}
//Classes
mxClassID mxArray::getClassID(){
	return id;
}
const char* mxArray::mxGetClassName(){
	switch(id){
		case mxUNKNOWN_CLASS : 
			return "UNKNOWN_CLASS";
        	case mxCELL_CLASS : 
			return "CELL_CLASS";
        	case mxSTRUCT_CLASS :
			return "STRUCT_CLASS";

		case mxLOGICAL_CLASS:
			return "LOGICAL_CLASS";
        	case mxCHAR_CLASS :
			return "CHAR_CLASS";
        	case mxUNUSED_CLASS :
			return "UNUSED_CLASS";
        	case mxDOUBLE_CLASS: 
			return "DOUBLE_CLASS";
        	case mxSINGLE_CLASS :
			return "SINGLE_CLASS";
        	case mxINT8_CLASS :
			return "INT8_CLASS";
        	case mxUINT8_CLASS :
			return "UINT8_CLASS";
        	case mxINT16_CLASS :
			return "INT16_CLASS";
        	case mxUINT16_CLASS :
			return "UINT16_CLASS";
        	case mxINT32_CLASS :
			return "INT32_CLASS";
        	case mxUINT32_CLASS :
			return "UINT32_CLASS";
        	case mxINT64_CLASS :
			return "INT64_CLASS";
        	case mxUINT64_CLASS :
			return "UINT64_CLASS";	
		default :
			return NULL;
	}
}
void mxArray::setClassName(const char* name){
	string str(name);
	if(str=="UNKNOWN_CLASS"){
		id=mxUNKNOWN_CLASS;
	}
	else if(str=="CELL_CLASS"){
		id=mxCELL_CLASS;
	}
	else if(str=="STRUCT_CLASS"){
		id=mxSTRUCT_CLASS;
	}
	else if (str=="LOGICAL_CLASS"){
		id=mxLOGICAL_CLASS;
	}
	else if(str=="CHAR_CLASS"){
		id=mxCHAR_CLASS;
	}
	else if(str=="UNUSED_CLASS"){
		id=mxUNUSED_CLASS;
	}
	else if(str=="DOUBLE_CLASS"){
		id=mxDOUBLE_CLASS;
	}
	else if(str=="SINGLE_CLASS"){
		id=mxSINGLE_CLASS;
	}
	else if(str=="INT8_CLASS"){
		id=mxINT8_CLASS;
	}
	else if(str=="UINT8_CLASS"){
		id=mxUINT8_CLASS;
	}
	else if(str=="INT16_CLASS"){
		id=mxINT16_CLASS;
	}
	else if(str=="UINT16_CLASS"){
		id=mxUINT16_CLASS;
	}
	else if(str=="INT32_CLASS"){
		id=mxINT32_CLASS;
	}
	else if(str=="UINT32_CLASS"){
		id=mxUINT32_CLASS;
	}
	else if(str=="INT64_CLASS"){
		id=mxINT64_CLASS;
	}
	else if(str=="UINT64_CLASS"){
		id=mxUINT64_CLASS;
	} 

		
}

mxArray* mxArray::getCell(mwIndex idx)const {
	return new mxArray(dataobject->getCell(idx));
}

void mxArray::setCell(mwIndex idx,mxArray*val){
	dataobject->setCell(idx,val->getDataObject());
}
