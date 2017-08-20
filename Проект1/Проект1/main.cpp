#include <vector>
#include <iostream>
#include <string>

#include "Result.hpp"
#include "Traits.hpp"
#include "Allocator.hpp"
#include "Memory.hpp"
#include "Vector.hpp"

struct INonCopyable {
	INonCopyable() {}
	INonCopyable(const INonCopyable &) = delete;
	void operator = (const INonCopyable &) = delete;
};
struct A : INonCopyable{
	A(A &&) noexcept {}
	void operator = (A &&) noexcept {}
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
	gc::container::Vector<int>::make(5)
		.on_success([](gc::container::Vector<int> && v){
			std::cout << v.length();
			return gc::Ok(std::move(v));
		})
	;
	std::cin.get();
	return 0;
}