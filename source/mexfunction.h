//MexFunction
#include"functions.h"
#include "dlfcn.h"
#include"mxArray.h"
#include<string>
#include<unistd.h>
class MexFunction:public Function{
public:
void close();
static int isPresent(const std::string&) ;
void load();
void * getFunctionPtr();
void * getLibHandle();
const std::string getFileName();
void setFileName(std::string );
MexFunction();
MexFunction(std::string fileName,void*lib_handle,void*fn);
void setFunctionPtr(void*);
void setLibHandle(void*);
 MexFunction* copy() const {
	return new MexFunction(fileName,lib_handle,fn);
}
std::string toString()const {
	return "<MEXFUNCTION: "+ getFuncName()+">";
}
private :
void *lib_handle;
void *fn;
std::string fileName;
};

