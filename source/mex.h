/*
 * mex.h
 *
 *  Created on: Jun 12, 2013
 *      Author: sameer
 */
#include "mexproto.h"
#ifndef MEX_H_
#define MEX_H_
//typedef void mxArray;
//typedef int bool;
extern "C"{
void mexFunction(int nlhs,mxArray*plhs[],int nrhs,const mxArray *prhs[]);
}
//some random type definitions that are there in Octave are not defined. I will do that once I figure out why they are there. Also there are some which are there  for backward compatibility which I have ignored.
#endif /* MEX_H_ */
