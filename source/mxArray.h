/*
 * mxArray.h
 *
 *  Created on: Jun 14, 2013
 *      Author: sameer
 */

#ifndef MXARRAY_H_
#define MXARRAY_H_
#include"objects.h"
#include"cellarrayobj.h"
#include"chararrayobj.h"
#include<iostream>
//#define DEBUG
//#include"matrixobjs.h"
//define types for Mex.
typedef char mxChar;
typedef bool mxLogical;
typedef size_t mwSize;
typedef size_t mwIndex;
using namespace std;
/** Enum type to define complexity of a MxArray object
    The complexity can be mxREAL for real types and mxCOMPLEX for complex types
*/
typedef enum mxComplexity {
	mxREAL, mxCOMPLEX
} mxComplexity;
/**
   Enum type to define the type of the dataobject inside the MxArray class 
   The types can be numeric, structures or cell arrays
*/
typedef enum {
	mxUNKNOWN_CLASS = 0,
	mxCELL_CLASS,
	mxSTRUCT_CLASS,
	mxLOGICAL_CLASS,
	mxCHAR_CLASS,
	mxUNUSED_CLASS,
	mxDOUBLE_CLASS,
	mxSINGLE_CLASS,
	mxINT8_CLASS,
	mxUINT8_CLASS,
	mxINT16_CLASS,
	mxUINT16_CLASS,
	mxINT32_CLASS,
	mxUINT32_CLASS,
	mxINT64_CLASS,
	mxUINT64_CLASS,
	mxFUNCTION_CLASS
} mxClassID;
/** mxArray Class. This class in Mex which serves as a wrapper  for matlab datatypes in C/C++. The Class extends the Boehm GC class(gc) for garbage collection. 
The private member dataobject is a pointer to the actual matlab datatype.  
*/
class mxArray 
#ifdef MCVM_USE_GC
:public gc
#endif 
{
	// virtual Destructor. declared since the class contains virtual methods.
	//virtual ~mxArray()=0;
public:
	//!<data extractors
	double* getPr() const ; //!<returns a double pointer to the real data
	double* getPi() const;  //!<returns a double pointer to the imaginary data
	double getScalar() const ; //!<returns a scalar value. If the data is an array, the first real value is returned.
	mxChar* getChars() const ; //!<returns a pointer to a char array.
	mxLogical* getLogical()const ; //<! returns a  pointer to a logical array
	void* getData()const ; //<! generic method to return a void pointer to the real data
	void* getImagData()const ; //<!generic method to return a void pointer to the imaginary data

	//<!Data setters
	void setPr(double*); //<!sets real double value of the array
	void setPi(double*); //!<sets imaginary double value of the array
	void setImagData(void*); //!<generic method to set real part of the array
	void setData(void*);     //!<generic method to set imaginary part of the array

	//<!Dimension Setters
	void setM(mwSize M);  //!< method to set the number of rows of the array
	void setN(mwSize N);  // !<method to set the number of columns of the array
	void setDimensions(const mwSize*, mwSize ndims); //!< method to set the dimensions

	//<!Dimension extractors
	size_t getM()const; //<! method to get the number of rows of an array  
	size_t getN()const ; //<! method to get the number of columns of an array
	mwSize* getDimensions()const ; //<! method to get the dimensions of the array
	mwSize getNumberOfDimensions()const ; 
	size_t getNumberOfElements()const ;

	//type Predicates
	int isCell();
	int isChar();
	int isClass(char *name);
	int isComplex();
	int isDouble();
	int isFunctionHandle();
	int isInt16();
	int isInt32();
	int isInt64();
	int isInt8();
	int isLogical();
	int isNumeric();
	int isSingle();
	int isSparse();
	int isStruct();

	//size predicate
	int isEmpty();
	int isScalar();
	//classes
	mxClassID getClassID();
	const char* mxGetClassName();
	void setClassName(const char* name);
		
        //Cell  Support
	mxArray* getCell(mwIndex val)const ;
	void setCell(mwIndex idx,mxArray*val);
	//floating point predicates
	//int isFinite(double v);
	//int isInf(double v);
	//int isNaN(double v);

	//floating point values
	//double getEps();
	//double getInf();
	//double getNaN();
	int getString(char* buf,mwSize buflen);
	char* mxArrayToString();
	//constructors
	mxArray(mwSize row, mwSize col, mxComplexity complexity = mxREAL,
			mxClassID classId = mxUNKNOWN_CLASS) {
		id = classId;
		complexFlag=complexity;
		switch (classId) {
		case mxUNKNOWN_CLASS:
#ifdef DEBUG
			cout<<"class id unknown"<<std::endl;
#endif

			break;
		case mxCELL_CLASS:
#ifdef DEBUG
			cout<<"cell array constructor invoked"<<std::endl;
#endif
			dataobject = new CellArrayObj(row, col);
			break;

		case mxSTRUCT_CLASS:
#ifdef DEBUG
			cout<<"struct class not supported "<<std::endl;
#endif
			break;
		case mxLOGICAL_CLASS:
#ifdef DEBUG
			cout<<"Logical matrix constructor called"<<std::endl;
#endif
			dataobject = new LogicalArrayObj(row, col);
			break;
		case mxCHAR_CLASS:
#ifdef DEBUG
			cout<<"Char class constructor invoked "<<std::endl;
#endif
			dataobject = new MatrixObj<char>(row, col);
			break;

		case mxUNUSED_CLASS:
#ifdef DEBUG
			cout<<"unused class not supported"<<std::endl;
#endif
			break;
		case mxDOUBLE_CLASS:
#ifdef DEBUG
			cout<<"Double Matrix Constructor invoked"<<std::endl;
#endif
			if (complexity == mxREAL) {
				dataobject = new MatrixObj<float64>(row, col);
			} else if (complexity == mxCOMPLEX) {
				dataobject = new MatrixObj<Complex128>(row, col);
			}
			break;
		case mxSINGLE_CLASS:
#ifdef DEBUG
			cout<<"Single Precision  Matrix Constructor invoked"<<std::endl;
#endif			
			if (complexity == mxREAL) {
				dataobject = new MatrixObj<float32>(row, col);
			} /*else if (complexity == mxCOMPLEX) {
				dataobject = new MatrixObj<Complex64>(row, col);
			}*/
			break;
		case mxINT32_CLASS:
#ifdef DEBUG
			cout<<"32 bit Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<int32>(row, col);

			break;
		case mxINT8_CLASS:

#ifdef DEBUG
			cout<<"8 bit Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<int8>(row,col);
			break;
		case mxINT16_CLASS:

#ifdef DEBUG
			cout<<"16 bit Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<int16>(row,col);
			break;
		case mxINT64_CLASS:

#ifdef DEBUG
			cout<<"64 bit Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<int16>(row,col);
			break;
		case mxUINT32_CLASS:

#ifdef DEBUG
			cout<<" 32 bit Unsigned Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<int32>(row,col);
			break;
		case mxUINT8_CLASS:

#ifdef DEBUG
			cout<<"8 bit Unsigned Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<uint8>(row,col);
			break;
		case mxUINT16_CLASS:

#ifdef DEBUG
			cout<<"16 bit Unsigned Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<uint16>(row,col);
			break;
		case mxUINT64_CLASS:

#ifdef DEBUG
			cout<<"16 bit Unsigned Integer Matrix Constructor invoked"<<std::endl;
#endif
			dataobject=new MatrixObj<uint64>(row,col);
			break;
		default:
#ifdef DEBUG
			cout<<"Ended up with something that was not supported"<<std::endl;
#endif
			break;
		}
	
	}
	mxArray(mwSize ndims, const mwSize* dims, mxComplexity flag,mxClassID classId) {
		id = classId;
		complexFlag=flag;
		switch (classId) {
		case mxUNKNOWN_CLASS:
#ifdef DEBUG
			cout<<"class id unknown"<<std::endl;
#endif

			break;
		case mxCELL_CLASS:
#ifdef DEBUG
			cout<<"cell array constructor invoked"<<std::endl;
#endif

			dataobject = new CellArrayObj(DimVector(const_cast<mwSize*>(dims), const_cast<mwSize*>(dims) + ndims ));
			break;

		case mxSTRUCT_CLASS:
#ifdef DEBUG
			cout<<"struct class not supported "<<std::endl;
#endif
			break;
		case mxLOGICAL_CLASS:
#ifdef DEBUG
			cout<<"Logical array constructor invoked"<<std::endl;
#endif
			dataobject = new LogicalArrayObj(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));
			break;
		case mxCHAR_CLASS:
#ifdef DEBUG
			cout<<"Char class constructor invoked "<<std::endl;
#endif
			dataobject = new MatrixObj<char>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));
			break;

		case mxUNUSED_CLASS:
#ifdef DEBUG
			cout<<"unused class not supported"<<std::endl;
#endif
			break;
		case mxDOUBLE_CLASS:
#ifdef DEBUG
			cout<<"Double Array Constructor invoked"<<std::endl;
#endif
			if (flag == mxREAL) {
				dataobject = new MatrixObj<float64>(
						DimVector(const_cast<mwSize*>(dims), const_cast<mwSize*>(dims) + ndims));
			} else if (flag == mxCOMPLEX) {
				dataobject = new MatrixObj<Complex128>(
						DimVector(const_cast<mwSize*>(dims), const_cast<mwSize*>(dims) + ndims));
			}
			break;
		case mxSINGLE_CLASS:
#ifdef DEBUG
		cout<<"Single Precision  Array Constructor invoked"<<std::endl;
#endif
			if (flag == mxREAL) {
				dataobject = new MatrixObj<float32>(
						DimVector(const_cast<mwSize*>(dims), const_cast<mwSize*>(dims) + ndims));
			} /*else if (flag == mxCOMPLEX) {
				dataobject = new MatrixObj<Complex64>(
						DimVector(dims, dims + ndims));
			}*/
			break;
		case mxINT32_CLASS:
#ifdef DEBUG
			cout<<"32 bit Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<int32>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxINT16_CLASS:
#ifdef DEBUG
			cout<<"16 bit Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<int16>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxINT8_CLASS:
#ifdef DEBUG
			cout<<"8 bit Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<int8>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxINT64_CLASS:
#ifdef DEBUG
			cout<<"64 bit Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<int64>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxUINT32_CLASS:
#ifdef DEBUG
			cout<<"32 bit Unsigned  Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<uint32>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxUINT16_CLASS:
#ifdef DEBUG
			cout<<"16 bit Unsigned  Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<uint16>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxUINT8_CLASS:
#ifdef DEBUG
			cout<<"8 bit Unsigned Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<uint8>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		case mxUINT64_CLASS:
#ifdef DEBUG
			cout<<"64 bit Unsigned  Integer Array Constructor invoked"<<std::endl;
#endif
			dataobject = new MatrixObj<uint64>(DimVector(const_cast<mwSize*>(dims),const_cast<mwSize*>(dims)+ndims));

			break;
		default:
#ifdef DEBUG
			cout<<"Ended up with something that was not supported"<<std::endl;
#endif
			break;
		}


	}
	mxArray(){
		id=mxUNKNOWN_CLASS;
		dataobject=NULL;
		complexFlag=mxREAL;
	}
	mxArray(DataObject* obj, mxClassID classid,mxComplexity complexity){
		dataobject=obj;
		id=classid;
		complexFlag=complexity;
	}
	mxArray(char*str){
		int n=strlen(str);
		id=mxCHAR_CLASS;
		complexFlag=mxREAL;
		dataobject=new MatrixObj<char>(1,n);
		setData(str);
	}
	mxArray(int m,char** str){
		id=mxCHAR_CLASS;
		complexFlag=mxREAL;
		int maxLen=0;
		for(int i=0;i<m;i++){
			int len=strlen(str[i]);
			maxLen<len?maxLen=len:maxLen=maxLen;
		}	
		dataobject=new MatrixObj<char>(m,maxLen);
		setData((void*)str);
	}
	mxArray(DataObject* obj){
		switch(obj->getType()){
			case DataObject::MATRIX_I32 :
				id=mxINT32_CLASS;
				complexFlag=mxREAL;	
			break;
			case DataObject::MATRIX_I8 :
				id=mxINT8_CLASS;
				complexFlag=mxREAL;	
			case DataObject::MATRIX_I16 :
				id=mxINT16_CLASS;
				complexFlag=mxREAL;	
				break;
			case DataObject::MATRIX_I64 :
				id=mxINT64_CLASS;
				complexFlag=mxREAL;	
				break;
			case DataObject::MATRIX_F32 :
				id=mxDOUBLE_CLASS;
				complexFlag=mxREAL;	
				break;
			case DataObject::MATRIX_F64 :
				id=mxSINGLE_CLASS;
				complexFlag=mxREAL;	
				break;
			case DataObject::MATRIX_C128 :
				id=mxDOUBLE_CLASS;
				complexFlag=mxCOMPLEX;	
				break;
			case DataObject::MATRIX_C64 :
				id=mxSINGLE_CLASS;
				complexFlag=mxCOMPLEX;
				break;
			case DataObject::LOGICALARRAY:
				id=mxLOGICAL_CLASS;
				complexFlag=mxREAL;
				break;
			case DataObject::CHARARRAY : 
				id=mxCHAR_CLASS;
				complexFlag=mxREAL;	
				break;
			case DataObject::CELLARRAY :
				id=mxCELL_CLASS;
				complexFlag=mxREAL;
				break;
			case DataObject::STRUCT_INST :
				id=mxSTRUCT_CLASS;
				complexFlag=mxREAL;
				break;
			case DataObject::CLASS_INST :
				std::cout<<"not supported"<<std::endl;
				break;
			case DataObject::FUNCTION :
				std::cout<<"not supported"<<std::endl;
				break;
			case DataObject::RANGE :
				std::cout<<"not supported"<<std::endl;
				break;
			case DataObject::ARRAY :
				std::cout<<"not supported"<<std::endl;
				break;
			case DataObject::FN_HANDLE:
				std::cout<<"not supported"<<std::endl;
				break;
			default :
				std::cout<<"not found"<<std::endl;
				break;
		}
		dataobject=obj;
	}
	mxArray(double scalarVal){
		dataobject=new MatrixObj<double>(scalarVal);	
	}
	mxArray(mxLogical scalarVal){
		dataobject=new LogicalArrayObj(scalarVal);	
	}
	DataObject* getDataObject();
	void setDataObject(DataObject*);
	mxArray* copy();

private:
	DataObject* dataobject;
	mxClassID id;
	mxComplexity complexFlag;
};

#endif /* MXARRAY_H_ */
