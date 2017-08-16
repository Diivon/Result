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
		return gc::Ok(a * a);
	else
		if (a < 0)
			return {};
		else
			return gc::Err(Err());
}

void result_tests() {
	//1
	assert(gc::Result<int, Error>().is_err());
	//2
	assert(gc::Result<int, Error>(gc::Err(Error::B)).is_err());
	//3
	assert(gc::Result<int, Err>(gc::Ok(5)).is_ok());
	//4
	assert(gc::Result<int, Error>(gc::Ok(5)).on_success([](auto & i) {i *= 2; }).unwrap_value() == 10);
	//5
	assert(gc::Result<int, Error>(gc::Ok(5)).on_fail([](auto & i) {}).unwrap_value() == 5);
	//6
	assert(gc::Result<int, Error>(gc::Err(Error::B)).on_fail([](auto & i) { 
		if (i == Error::A)
			return 14;
		return 228;
	}).unwrap_value() == 228);
}

int main() {
	result_tests();
	std::cin.get();
	return 0;
}