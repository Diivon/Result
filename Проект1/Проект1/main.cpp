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
void println(T && t) {
	std::cout << t << std::endl;
}

class A {
public:
	A() noexcept { println("A()"); }
	A(int) noexcept { println("A(int)"); }
	A(A &&) noexcept { println("A(&&)"); }
	A(const A &) noexcept { println("A(const &)"); }
	void operator = (const A &) noexcept { println("= (const &)"); }
	void operator = (A &&) noexcept { println("= (&&)"); }
	~A() noexcept { println("~A()"); }
	bool operator == (const A &) noexcept { return true; }
};

template<class T>
void debug(T && t) {
	std::cout << gc::TypeName<T>::get() << ':' << ' ' << t << std::endl;
}

int main() {
	using namespace gc::container;
	/*
	Vector<int>::make(5, 45)
		.on_success([](Vector<int> && v1) {
			v1.whole()
				.map([](const int & i) {
					return i * i; 
				})
				.foreach([](const int & i) {
					debug(i);
				})
			;
			return gc::Ok(v1.move());
		})
	;
	*/
	Vector<A>::make(3)
		.on_success([](Vector<A> && a) {
			Vector<int>::make(3)
				.map_result_type<Vector<A>>([&](Vector<int> && v) {
					return Vector<A>::make_with_capacity(3)
						.on_success([&](Vector<A> && a) {
							a.push();
							a.push();
							a.push();
							return gc::Ok(a.move());
						}).move();
				})
				.on_success([&](Vector<A> && b) {
					println("answer");
					println(b == a);
					return gc::Ok(b.move());
				});
			return gc::Ok(a.move());
		})
	;
	std::cin.get();
	return 0;
}