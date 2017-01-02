#pragma once

namespace mws { namespace common {

template<typename T>
struct RAII 
{
	RAII(T& v_) 
	: 
		_v(v_), _vCpy(v_) 
	{
	
	} 
	
	~RAII() 
	{
		_v = _vCpy; 
	}

private:
	T&      _v;
	const T _vCpy;
};

}}
