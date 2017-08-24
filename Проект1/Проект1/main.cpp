#include <iostream>
#include <string>

#include "Result.hpp"
#include "Traits.hpp"
#include "Allocator.hpp"
#include "Memory.hpp"
#include "Vector.hpp"

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

struct A : gc::INonCopyable {
	int a;
	A(A &&) noexcept {}
	A(int && b) noexcept : a(b) {
		std::cout << "&&" << std::endl;
	}
};

int main() {
	using namespace gc::container;
	Vector<int>::make(5, 45)
		.on_success([](Vector<int> && v1) {
			Vector<int>::make(5, 45)
				.on_success([&](Vector<int> && v2) {
					assert(v1 == v2);
					return gc::Ok(v2.move());
				});
			return gc::Ok(v1.move());
		})
	;
	std::cin.get();
	return 0;
}