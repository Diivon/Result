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

void assert(bool cond) {
	static uint32_t count = 1;
	std::cout << "test #" << count++;
	if (cond)
		std::cout << " succeded";
	else
		std::cout << " failed";
	std::cout << std::endl;
}

gc::Result<int, gc::Error> get(int a) {
	if (a > 20)
		return gc::Ok(a * a);
	else
		return gc::Err(gc::Error::DomainError);
}

template<class T>
void debug(T && t) {
	std::cout << gc::traits::TypeName<T>::get() << ':' << ' ' << t << std::endl;
}

struct A : INonCopyable {
	int a;
	A(A &&) noexcept {}
	A(int && b) noexcept : a(b) {
		std::cout << "&&" << std::endl;
	}
};

int main() {
	using namespace gc::container;
	Vector<A>::make_with_capacity(5)
		.on_success([](Vector<A> && v) {
			assert(v.capacity() == 5);
			return gc::Ok(std::move(v));
		})
	;
	std::cin.get();
	return 0;
}