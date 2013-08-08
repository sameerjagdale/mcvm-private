// =========================================================================== //
//                                                                             //
// Copyright 2008 Maxime Chevalier-Boisvert and McGill University.             //
//                                                                             //
//   Licensed under the Apache License, Version 2.0 (the "License");           //
//   you may not use this file except in compliance with the License.          //
//   You may obtain a copy of the License at                                   //
//                                                                             //
//       http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                             //
//   Unless required by applicable law or agreed to in writing, software       //
//   distributed under the License is distributed on an "AS IS" BASIS,         //
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//   See the License for the specific language governing permissions and       //
//  limitations under the License.                                             //
//                                                                             //
// =========================================================================== //

// Include guards
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

// Header files
#include <complex>
#include<stdint.h>

// Pointer size on the current platform
const size_t PLATFORM_POINTER_SIZE = sizeof(void*);

// Platform alignment multiple
const size_t PLATFORM_ALIGNMENT = PLATFORM_POINTER_SIZE;

// Platform word size definition
#if (POINTER_SIZE == 4)
#define PLATFORM_32BIT
#elif (POINTER_SIZE == 8)
#define PLATFORM_64BIT
#endif
#define MCVM_USE_GC
// Basic byte type
typedef unsigned char byte;

// Signed integer types
typedef  int8_t      int8;
typedef  int16_t     int16;
typedef  int32_t       int32;
typedef  int64_t int64;

// Unsigned integer types
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t       uint32;
typedef uint64_t uint64;

// Floating point types
typedef float  float32;
typedef double float64;

// Complex number types
typedef std::complex<float32> Complex64;
typedef std::complex<float64> Complex128;


// Macro to obtain the offset of a class/struct member variable
#define MEMBER_OFFSET(CLASS, MEMBER) ((uint8*)&((CLASS*)PLATFORM_POINTER_SIZE)->MEMBER - (uint8*)PLATFORM_POINTER_SIZE)

/***************************************************************
* Function: IsAligned()
* Purpose : Test if an index or value is aligned
* Initial : Maxime Chevalier-Boisvert on January 1, 2008
****************************************************************
Revisions and bug fixes:
*/
inline bool IsAligned(size_t Value, size_t Multiple = PLATFORM_ALIGNMENT)
{
	// If the value is not aligned
	return (Value % Multiple == 0);
}

/***************************************************************
* Function: AlignValue()
* Purpose : Align an index or value to a multiple
* Initial : Maxime Chevalier-Boisvert on January 1, 2008
****************************************************************
Revisions and bug fixes:
*/
inline size_t AlignValue(size_t Value, size_t Multiple = PLATFORM_ALIGNMENT)
{
	// If the value is not aligned
	if (Value % Multiple != 0)
	{
		// Return the padded size
		return ((Value / Multiple) + 1) * Multiple;
	}
	else
	{
		// Return the value unchanged
		return Value;
	}
}

#endif // #ifndef __PLATFORM_H__
