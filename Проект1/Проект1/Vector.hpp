#pragma once
#include <new>

#include "Memory.hpp"
#include "Allocator.hpp"
#include "Result.hpp"

namespace gc {
	namespace container {
		template<class T, class Allocator = gc::memory::Allocator>
		class Vector {
			static_assert(gc::traits::is_gc_allocator_v<Allocator>,
				"second template argument do not match gc_allocator trait");
			///slice of allocated memory
			memory::Slice _mem;
			///ptr to memory behind last element
			T * _last;
			Vector(memory::Slice && sl, T * end) noexcept :
				_mem(std::move(sl)), _last(end)
			{}
		public:
			Vector(Vector && v) noexcept :
				_mem(std::move(v._mem)), _last(std::move(v._last))
			{
				_mem = memory::Slice::null();
				_last = nullptr;
			}

			unsigned length() const noexcept {
				return _last - _mem.begin_as<T>();
			}
			template<class ... Args>
			static Result<Vector<T, Allocator>, Error> make(unsigned count, Args ...) {
				static_assert(std::is_nothrow_constructible_v<T>, 
					"gc::container::Vector<T>::make(size) T must be nothrow default constructible");
				return Allocator::allocate(sizeof(T) * count)
					.on_success([&count](memory::Slice && sl) {
						T * ptr = sl.begin_as<T>();
						for (unsigned i = 0; i < count; ++i)
							new(ptr + i) T();//asserted to be noexcept
						return Ok(std::move(sl));
					})//
					.map_result_type<Vector<T, Allocator>>([&count](memory::Slice && sl) {
						T * ptr = sl.begin_as<T>() + count;
						return Ok(Vector<T, Allocator>{std::move(sl), ptr});
					});
			}
		};
	}
}