#include"mexfunction.h"
void MexFunction::setFileName(std::string file){
	fileName=file;
}
const std::string MexFunction::getFileName(){
	return fileName;
}
void MexFunction::close()
{
	dlclose(lib_handle);
	lib_handle=NULL;
	fn=NULL;
}

void MexFunction::load()
{
        char *error;	
        lib_handle = dlopen(fileName.c_str(), RTLD_LAZY);
        if (!lib_handle)
        {
               // fprintf(stderr, "%s\n", dlerror());
                //exit(1);
		throw RunError(std::string("Error while opening Mex File :") + dlerror());
        }
        fn =dlsym(lib_handle,"mexFunction");
        if ((error = dlerror()) != NULL)
        {
    	        //fprintf(stderr, "%s\n", error);
                //exit(1);
		throw RunError(std::string("Error while opening Mex File :")+ error);
        }

}
void* MexFunction::getFunctionPtr(){
	return fn;
}
void* MexFunction::getLibHandle(){
	return lib_handle;
}
MexFunction::MexFunction(){
	lib_handle=NULL;
	fn=NULL;
	
}
MexFunction::MexFunction(std::string str,void*handle,void*func){
	m_isProgFunction=false;
	m_isMexFunction=true;
	fileName=str;
	std::string tempStr=str.c_str();
	m_funcName=strtok(const_cast<char*>(tempStr.c_str()),".");
	lib_handle=handle;
	fn=func;
}
void MexFunction::setFunctionPtr(void*func){
	fn=func;
}
void MexFunction::setLibHandle(void*handle){
	lib_handle=handle;
}
int MexFunction::isPresent(const std::string& fileName) {
	return access(fileName.c_str(),F_OK);	
}


