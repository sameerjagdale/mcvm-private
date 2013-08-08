#include<iostream>
#include"stdlib.h"
#include<cstring>
#include "mex_utility.h"
 std::string   mcvm_vasprintf(const char* fmt,va_list args){
	 std::string retval;
  char *result;
  int status = vasprintf (&result, fmt, args); 
  if (status >= 0)
    {
      retval = result;
      ::free (result);
    }
  return retval;

}
