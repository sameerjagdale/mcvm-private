#include <mcvm.h>
#include"mxArray.h"
#include<dlfcn.h>
#include<iostream>
typedef void (*fn)(int,mxArray**,int,const mxArray**);
int main (int argc, char** argv) {
    initialize(argc,argv) ;
    run() ;
    shutdown() ;
 /*void *lib_handle;
   fn mexFunction;
   char *error;

   lib_handle = dlopen("libctest.mex", RTLD_LAZY);
   if (!lib_handle) 
   {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
   }

   mexFunction = (fn)dlsym(lib_handle, "mexFunction");
   if ((error = dlerror()) != NULL)  
   {
      fprintf(stderr, "%s\n", error);
      exit(1);
   }
 mxArray* plhs[2];
 mxArray*  prhs[2];

  prhs[0]=new mxArray(3,3,mxREAL,mxDOUBLE_CLASS); 
  double *pr=prhs[0]->getPr(); 
int count=1;
for(int i=0;i<3;i++){
	cout<<std::endl;
	for(int j=0;j<3;j++){
	    pr[j*3+i]=count++;
	}
}
   mexFunction(1,plhs,1,(const mxArray**)prhs);
  pr=plhs[0]->getPr();

for(int i=0;i<3;i++){
	cout<<std::endl;
	for(int j=0;j<3;j++){
	    cout<<pr[j*3+i]<<"\t";
	}
}
   dlclose(lib_handle);
*/
   return 0;    
}
