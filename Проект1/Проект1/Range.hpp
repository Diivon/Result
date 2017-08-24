#pragma once

template<class T>
class Range {
	T _begin;
	T _end;
public:
	Range(T && start, T && end) noexcept :
		_begin(std::forward<T>(start)), _end(std::forward<T>(end))
	{
		//TODO static_assert noexcept
	}
	template<class F>
	Range & foreach(F && f) {
		//TODO static asserts
		for (auto i = _begin; i < _end; ++i)
			f(*i);
		return *this;
	}
	template<class F>
	Range & map(F && f) {
		//TODO static asserts
		for (auto i = _begin; i < _end; ++i)
			*i = f(*i);
		return *this;
	}
};