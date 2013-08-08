#include "interpreter.h"
#include"mcvmstdlib.h"
#define LINUX
#define MEX
//#define DEBUG
#ifdef MEX
#include"mxArray.h"
#include<lru-cache.h>
#include<mex_utility.h>
#ifdef LINUX 
#include<unistd.h>
#include<dlfcn.h>
#include<stdarg.h>
#include<csetjmp>
#endif 
#endif 
#include<cmath>
typedef void (*mexFunction)(int,mxArray**,int,const mxArray**);
typedef void (*exitFunction)(void);
class MexManager{
	public :
		void setExitFunc(exitFunction func){
			exitFunc=func;
		}
		exitFunction getExitFunc(){
			return exitFunc;
		}
		void setExitFlag(int flag){
			exitFlag=flag;
		}	
		int getExitFlag(){
			return exitFlag;
		}
		void setCurrentFunc(MexFunction* func){
			currentFunc=func;
		}	
		MexFunction* getCurrentFunc(){
			return currentFunc;
		}
		void setTrapFlag(int flag){
			trap_flag=flag;
		}
		int getTrapFlag(){
			return trap_flag;
		}
		MexManager(MexFunction *currFun,exitFunction exitFun=NULL,int exitFlag=0,int trapFlag=0){
			currentFunc=currFun;
			exitFunc=exitFun;
			this->exitFlag=exitFlag;
			trap_flag=trapFlag;
		}
		
	private:
 	
		exitFunction exitFunc;
		int exitFlag=0;
		MexFunction *currentFunc;
		int trap_flag;
};
static LRUCache<std::string,MexFunction> libCache;
std::set<void*> PersistentMemorySet;
jmp_buf jmp;
MexManager *mexContext;
//returns a pointer to the mexfunction if one exists
MexFunction* Interpreter::loadMexFile(const std::string& fileName){
	//check if the file is present in the path provided
	if(MexFunction::isPresent(fileName)!=0){
		return NULL;
	}
	//If it is present , check whether the file is already loaded. 
	MexFunction *mexFunc=libCache.getValue(fileName);
	if(mexFunc==NULL){	
		mexFunc=new MexFunction(fileName,NULL,NULL);
		mexFunc->load();
		libCache.add(fileName,mexFunc);
	}
	
	return mexFunc;
	
	 
}
ArrayObj* Interpreter::callMexFunction(MexFunction*pFunction,ArrayObj*pArguments,size_t nargout){

#ifdef DEBUG 
	std::cout<<"calling mex function"<<std::endl;
#endif 		
		if(pFunction->getFunctionPtr()==NULL){
			if((pFunction=loadMexFile(pFunction->getFileName()))==NULL)
				throw RunError("Mex Function"+pFunction->getFileName()+ "is not in current directory");
		}
		ArrayObj* pOutput; 
		if(setjmp(jmp)==0){
			mexContext=new MexManager(pFunction);	
			const mxArray**pRhs=(const mxArray**)new mxArray*[pArguments->getSize()]; 
			int nout=nargout==0?1:nargout;	
			mxArray**pLhs=new mxArray*[nout];
			for(int i=0;i<pArguments->getSize();i++){
				pRhs[i]=new mxArray(pArguments->getObject(i)->copy());
		
			}
			mexFunction fn=(mexFunction)pFunction->getFunctionPtr();
			fn(nout,pLhs,static_cast<int>(pArguments->getSize()),pRhs);
			std::cout<<"nargout  "<<nargout<<std::endl;
			if(nargout==0&&pLhs[0]){
				std::cout<<"nargout=1"<<std::endl;
				nargout=1;
			}
			pOutput = new ArrayObj(nargout);
			for(int i=0;i<nargout;i++){
				if(pLhs[i]==NULL){
					throw RunError(pFunction->getFuncName()+" : Too Many Output Arguments in Mex Function ");
				}
				ArrayObj::addObject(pOutput,pLhs[i]->getDataObject());
			}
			if(mexContext->getExitFlag()!=0){
				mexContext->getExitFunc()();
			}
			mexContext=NULL;
		}
		else {
			mexContext=NULL;
			throw RunError(pFunction->getFuncName()+" : Returned with error");
		}
	return pOutput;	
}
extern "C"{
	const char* mexFunctionName(){
        	if(mexContext->getCurrentFunc()!=NULL){
                	return (const char*)mexContext->getCurrentFunc()->getFuncName().c_str();
        	}
        	else{
#ifdef DEBUG
      	         	std::cout<<"Mexfunction pointer no assigned"<<std::endl;
#endif
                	return NULL;
        	}
	}
	
	int mexAtExit(exitFunction exitFun){
		mexContext->setExitFunc(exitFun);
		mexContext->setExitFlag(1);	
		return 0;
	}
	
	int mexPrintf(const char* fmt,...){
		va_list args;
		va_start(args,fmt);
		std::string  temp=mcvm_vasprintf(fmt,args);
		std::cout<<temp<<std::endl;
		va_end(args);
		return 0;
	}
	void mexMakeMemoryPersistent(void*ptr){
		PersistentMemorySet.insert(ptr);
	}
	void mexMakeArrayPersistent(mxArray* ptr){
	  	mexMakeMemoryPersistent((void*)ptr);
	}	
	void mexWarnMsgTxt(const char* str){
		mexPrintf(str);
	}
	void mexWarnMsgIdAndTxt(const char*id, const char*s,... ){
		va_list args;
		va_start(args,s);
		mexPrintf(mcvm_vasprintf((std::string(id)+" "+std::string(s)).c_str(),args).c_str());
		va_end(args);
	}
	 void mex_abort(int val){
                if(val==0){
                        val=1;
                }
                longjmp(jmp,val);
        }

	void mexErrMsgTxt(const char *s){
		std::string msg(mexFunctionName());
		msg="Error using : "+msg+"\n";
		mexPrintf(msg.c_str());
		mexPrintf(s);
		mex_abort(1);
	}
	void mexErrMsgIdAndTxt(const char* id,const char*s,...){
		std::string msg(mexFunctionName());
		msg="Error using : "+msg+"\n";
		mexPrintf(msg.c_str());
		va_list args;
		va_start(args,s);
		mexPrintf(mcvm_vasprintf((std::string(id)+" "+std::string(s)).c_str(),args).c_str());
		va_end(args);
		mex_abort(1);
				
	}	
	//!<floating point predicates 
	int mxIsFinite(double v){
        	return v<DOUBLE_INFINITY;
	}

	int mxIsInf(double v){
        	return v>=DOUBLE_INFINITY;
	}

	int mxIsNaN(double v){

        	if (notANumber<double>(v))
			return 1;
		else 
			return 0;
        
	}

	//!<Floating point values
	double mxGetEps(){
		return DOUBLE_EPSILON;
	}
	double mxGetInf(){
		return DOUBLE_INFINITY;
	}
	double mxGetNaN(){
		return NAN;
	}
	int mexCallMATLAB(int nargout,mxArray* argout[],int nargin, mxArray*argin[],const char*fname){
		ArrayObj *pArguments=new ArrayObj(nargin+1);
		ArrayObj::addObject(pArguments,new CharArrayObj(std::string(fname)));	
		for(int i=0;i<nargin;i++){
			ArrayObj::addObject(pArguments,argin[i]->getDataObject());
		}	
		LibFunction::FnPointer pHostFunc = mcvm::stdlib::feval.getHostFunc();
		ArrayObj *pOutput=pHostFunc(pArguments);
		int numCopy=pOutput->getSize();
		if(pOutput->getSize()>nargout)
		{
			numCopy=nargout;
		}
		for(int i=0;i<numCopy;i++){
			argout[i]=new mxArray(pOutput->getObject(i));
		}
		while (numCopy<nargout){
			argout[numCopy++]=0;
		}
		return 0;
	}
	
	void mexSetTrapFlag(int flag){
		mexContext->setTrapFlag(flag);
	}
}

