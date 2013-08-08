/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lru-cache.h
 * Copyright (C) 2013 Sameer Jagdale <sameer.jagdale@mail.mcgill.ca>
 *
 * test-cpp is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * test-cpp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
#include<boost/bimap/bimap.hpp>
#include<boost/bimap/unordered_set_of.hpp>
#include<boost/bimap/unconstrained_set_of.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include<ctime>
#include<gc_cpp.h>
//#include<gc/gc.h>
#include<gc/gc_allocator.h>

#define DEFAULT_SIZE 5;
//#define DEBUG
using namespace boost::bimaps;
template<class Key=std::string,class Value =int>
class LRUCache
{
public:
	typedef bimap<unordered_set_of<Key>,unconstrained_set_of<Value *> ,gc_allocator<Value*>> keyValMap;
	typedef bimap<unordered_set_of<Key>,set_of<time_t> > timeMap;
	typedef typename keyValMap::value_type val_type;
	typedef typename timeMap::value_type time_type;
	
LRUCache():maxSize(5)
{
	//maxSize=DEFAULT_SIZE;
}


LRUCache(int sz):maxSize(sz){
	
}

	void add(Key key,Value *val);
	void remove(Key key);
	Value* getValue(Key key);
	time_t getTimestamp(Key key);
	void prettyPrint ();
protected:

private:
	keyValMap kvMap;
	timeMap tMap;
	const int maxSize;
};

template<class Key,class Value >
void LRUCache<Key,Value>::add(Key key,Value *  val ){
#ifdef DEBUG
	std::cout<<"library being cached"<<std::endl;
#endif
	if(kvMap.size()==maxSize)
	{
	 //remove least recently used value
#ifdef DEBUG
		std::cout<<"Cache is full. Least recently used library is being closed"<<std::endl;
#endif
		auto it=tMap.right.begin();
		auto first=it->first;		
		auto tempVal=kvMap.left.find(it->second);
		auto tempKey=it->second;
		kvMap.left.erase(tempKey);
		tMap.right.erase(first);
		tempVal->second->close();
		
	}
	//Add Value
	kvMap.insert(val_type(key,val));
	tMap.insert(time_type(key,time(0)));
	
}
template<class Key, class Value >
void LRUCache<Key,Value >::remove(Key key){
	tMap.erase(key);
	kvMap.erase(key);
}

template<class Key,class Value >
Value*  LRUCache<Key,Value>::getValue(Key key){
	auto it=kvMap.left.find(key);
	if(it!=kvMap.left.end()){
		return it->second;
	}
	return NULL;
}
template<class Key,class Value>
time_t LRUCache<Key,Value >::getTimestamp (Key key){
	return tMap.left.find(key)->second;
}
template<class Key,class Value>
void LRUCache<Key, Value>::prettyPrint(){
	
	for ( auto it=tMap.left.begin(), end=tMap.left.end();it!=end;it++){
		std::cout<<"key:"<<it->first<<"--> Value:"<<it->second<<std::endl;
	}
	
}
#endif // _LRU_CACHE_H_

