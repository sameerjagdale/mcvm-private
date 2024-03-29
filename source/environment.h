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
#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

// Header files
#include <vector>
#include "symbolexpr.h"
#include "objects.h"
#include "utility.h"
#include <unordered_map>
#include<gc_cpp.h>
#include<gc/gc_allocator.h>
/***************************************************************
* Class   : Environment
* Purpose : Represent an execution environment
* Initial : Maxime Chevalier-Boisvert on November 12, 2008
****************************************************************
Revisions and bug fixes:
*/
class Environment 
#ifdef MCVM_USE_GC
: public gc
#endif
{
public:
	
#ifdef MCVM_USE_GC
	typedef std::vector<SymbolExpr*, gc_allocator<SymbolExpr*> > SymbolVec;
#else
	typedef std::vector<SymbolExpr*> SymbolVec;
#endif
	
	// Public constructor
	Environment();

	// Method to copy this environment object
	Environment* copy() const;
	
	// Method to create a new binding
	static void bind(Environment* pEnv, const SymbolExpr* pSymbol, DataObject* pObject);
	
	// Method to remove a binding
	static bool unbind(Environment* pEnv, const SymbolExpr* pSymbol);
	
	// Method to lookup a symbol
	static DataObject* lookup(const Environment* pEnv, const SymbolExpr* pSymbol);

	// Method to extend an environment object
	static Environment* extend(Environment* pParent);
	
	// Method to get the symbols bound in this environment
	SymbolVec getSymbols() const;
	
private:
	
	// Private constructor for extension
	Environment(Environment* pParent);
	
	// Symbol map type definition
	typedef std::unordered_map<SymbolExpr*, DataObject*> SymbolMap;
	
	// Bindings of the environment
	SymbolMap m_bindings;

	// Pointer to parent environment
	Environment* m_pParent;
};

#endif // #ifndef ENVIRONMENT_H_
