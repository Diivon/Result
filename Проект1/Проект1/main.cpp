#include <vector>
#include <iostream>

#include "Result.hpp"

struct INonCopyable {
	INonCopyable() {}
	INonCopyable(const INonCopyable &) = delete;
	void operator = (const INonCopyable &) = delete;
};

struct A :INonCopyable {
	std::vector<int> data;
	A() {}
	A(A && a) noexcept :
		data(std::move(a.data))
	{}
	void push(int a) {
		data.push_back(a);
	}
};
class Err : INonCopyable {
public:
	Err() noexcept {}
	Err(Err && e) noexcept {}
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

gc::Result<int, Err> get(int a) {
	if (a > 20)
		return gc::ok(a * a);
	else
		if (a < 0)
			return {};
		else
			return gc::err(Err());
}

void result_tests() {
	assert(gc::Result<int, Error>().is_err());
	assert(gc::Result<int, Error>(Error::B).is_err());
	assert(gc::Result<int, Err>(5).is_ok());
	assert(gc::Result<int, Error>(5).on_success([](auto & i) {i *= 2; }).unwrap_value() == 10);
	assert(gc::Result<int, Error>(5).on_fail([](auto & i) {}).unwrap_value() == 5);
}

int main() {
	result_tests();
	std::cin.get();
	return 0;
}