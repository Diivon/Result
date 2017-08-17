#include <vector>
#include <iostream>
#include <string>

#include "Result.hpp"
#include "Traits.h"

struct INonCopyable {
	INonCopyable() {}
	INonCopyable(const INonCopyable &) = delete;
	void operator = (const INonCopyable &) = delete;
};

enum class Error {
	A, B, C
};

void assert(bool cond) {
	static uint32_t count = 1;
	std::cout << "test #" << count++;
	if (cond)
		std::cout << " succeded";
	else
		std::cout << " failed";
	std::cout << std::endl;
}

gc::Result<int, Error> get(int a) {
	if (a > 20)
		return gc::Ok(a * a);
	else
		if (a < 0)
			return {};
		else
			return gc::Err(Error::A);
}

template<class T>
void debug(T && t) {
	std::cout << gc::traits::TypeName<T>::get() << ':' << ' ' << t << std::endl;
}

int main() {
	get(-5)
		.on_error([](Error && e) -> gc::Result<int, Error>{
			if (e == Error::A)
				return { gc::Ok(700) };
			return { gc::Err(std::move(e)) };
		})
		.on_success([](int && val) {
			std::cout << val << std::endl;
			return gc::Ok(std::move(val));
		})
		.map_result_type<std::vector<int>>([](int && val) {
			return std::vector<int>{ val * val, 2, 3, 4, 5, 6 };
		})
		.on_success([](std::vector<int> && val) {
			for (auto & i : val)
				std::cout << i << ' ';
			return gc::Ok(std::move(val));
		})
	;
	std::cin.get();
	return 0;
}