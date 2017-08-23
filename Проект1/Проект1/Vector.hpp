#pragma once
#include <new>

#include "Memory.hpp"
#include "Allocator.hpp"
#include "Result.hpp"

namespace gc {
	namespace container {
		template<class T, class Alloc = gc::memory::Allocator>
		struct Vector : INonCopyable {
			static_assert(gc::traits::is_gc_allocator_v<Alloc>,
				"second template argument do not match gc_allocator trait");
		public:
			Vector(Vector && v) noexcept;
			~Vector() noexcept;

			unsigned length() const noexcept;
			unsigned capacity() const noexcept;
			Result<T &, Error> operator [] (unsigned i) noexcept;
			Result<const T &, Error> operator [] (unsigned i) const noexcept;

			Vector && move() noexcept;

			template<class ... Args>
			static Result<Vector<T, Alloc>, Error> make(unsigned count, Args && ... args) noexcept;
			static Result<Vector<T, Alloc>, Error> make_with_capacity(unsigned capacity) noexcept;
			static Result<Vector<T, Alloc>, Error> make_from_raw(T * ptr, unsigned size, T * last = ptr) noexcept
		private:
			///slice of allocated memory
			memory::Slice _mem;
			///ptr to memory behind last element
			T * _last;
			Vector(memory::Slice && sl, T * end) noexcept;
		};



















#pragma region Vector implementation
	#pragma region constructors / destructor
		//move constructor
		template<class T, class Alloc> 
		Vector<T, Alloc>::Vector(Vector && v) noexcept:
			_mem(std::move(v._mem)), _last(std::move(v._last))
		{
			v._mem = memory::Slice::null();
			v._last = nullptr;
		}
		//constructor
		template<class T, class Alloc> 
		Vector<T, Alloc>::Vector(memory::Slice && sl, T * end) noexcept:
			_mem(std::move(sl)), _last(end)
		{}
		//destructor
		template<class T, class Alloc> 
		Vector<T, Alloc>::~Vector() noexcept{
			static_assert(std::is_nothrow_destructible_v<T>,
				"gc::container::Vector<T, Alloc> T destructor must be noexcept");

			if (_mem.begin_as<void>() != nullptr){
				//if T is nontrivial_destructible
				for (T * i = _mem.begin_as<T>(); i < _last; ++i)
					i->~T();//call destructor for everyone, asserted to be noexcept
				
				Alloc::deallocate(std::move(_mem));//guaranteed to be noexcept by allocator trait
			}
		}
#pragma endregion
	#pragma region make
		template<class T, class Alloc>
		template<class ... Args>
		Result<Vector<T, Alloc>, Error> Vector<T, Alloc>::make(unsigned count, Args && ... args) noexcept {
			static_assert(std::is_nothrow_constructible<T, Args && ...>::value, 
				"gc::container::Vector<T>::make(size, args...) T must be nothrow constructible with args");
			if (count == 0)
				return Ok(Vector<T, Alloc>{memory::Slice::null, nullptr});
			return Alloc::allocate(sizeof(T) * count)
				.on_success([&args..., &count](memory::Slice && sl) {
					T * ptr = sl.begin_as<T>();
					for (unsigned i = 0; i < count; ++i)
						new(ptr + i) T(std::forward<Args>(args)...);//asserted to be noexcept
					return Ok(std::move(sl));
				})
				.map_result_type<Vector<T, Alloc>>([&count](memory::Slice && sl) {
					T * ptr = sl.begin_as<T>() + count;
					return Ok(Vector<T, Alloc>{std::move(sl), ptr});
				})
			;
		}
		template<class T, class Alloc>
		Result<Vector<T, Alloc>, Error> Vector<T, Alloc>::make_with_capacity(unsigned count) noexcept {
			return Alloc::allocate(sizeof(T) * count)
				.map_result_type<Vector<T, Alloc>>([](memory::Slice && sl) {
					T * ptr = sl.begin_as<T>();
					return Ok(Vector<T, Alloc>{std::move(sl), ptr});
				})
			;
		}
	#pragma endregion
	#pragma region methods
		template<class T, class Alloc>
		unsigned Vector<T, Alloc>::length() const noexcept {
			return _last - _mem.begin_as<T>();
		}
		template<class T, class Alloc>
		unsigned Vector<T, Alloc>::capacity() const noexcept {
			return _mem.end_as<T>() - _mem.begin_as<T>();
		}
		template<class T, class Alloc>
		Result<T &, Error> Vector<T, Alloc>::operator [](unsigned i) noexcept {
			if (i > length() || (_mem.begin_as<void>() == nullptr))
				return Err(Error::OutOfRange);
			return Ok(_mem.begin_as<T>()[i]);
		}
		template<class T, class Alloc>
		Result<const T &, Error> Vector<T, Alloc>::operator [](unsigned i) const noexcept {
			if (i > length() || (_mem.begin_as<void>() == nullptr))
				return Err(Error::OutOfRange);
			return Ok(_mem.begin_as<T>()[i]);
		}
		template<class T, class Alloc>
		Vector<T, Alloc> && Vector<T, Alloc>::move() noexcept {
			return std::move(*this);
		}
	#pragma endregion
#pragma endregion
	}
}