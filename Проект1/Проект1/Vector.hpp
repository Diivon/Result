#pragma once
#include <new>

#include "Memory.hpp"
#include "Allocator.hpp"
#include "Result.hpp"
#include "Range.hpp"

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
			bool operator == (const Vector &) const noexcept;
			bool operator != (const Vector &) const noexcept;

			Vector && move() noexcept;
			Result<Vector<T, Alloc>, Error> copy() const noexcept;

			static Vector<T, Alloc> make() noexcept;
			template<class ... Args>
			static Result<Vector<T, Alloc>, Error> make(unsigned count, Args && ... args) noexcept;
			static Result<Vector<T, Alloc>, Error> make_with_capacity(unsigned capacity) noexcept;
			static Result<Vector<T, Alloc>, Error> make_from_raw(T * ptr, unsigned size, T * last = ptr) noexcept;
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
		Vector<T, Alloc> Vector<T, Alloc>::make() noexcept {
			return {memory::Slice::null(), nullptr};
		}
		template<class T, class Alloc>
		template<class ... Args>
		Result<Vector<T, Alloc>, Error> Vector<T, Alloc>::make(unsigned count, Args && ... args) noexcept {
			static_assert(std::is_nothrow_constructible<T, Args && ...>::value, 
				"gc::container::Vector<T>::make(size, args...) T must be nothrow constructible with args");
			if (count == 0)
				return Ok(make());
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
		template<class T, class Alloc>
		Result<Vector<T, Alloc>, Error> Vector<T, Alloc>::make_from_raw(T * ptr, unsigned count, T * last = ptr) noexcept {
			if (ptr == nullptr || count == 0u || last == nullptr || (last < ptr))
				return Err(Error::InvalidArgument);
			return Ok(Vector<T, Alloc> {memory::Slice{ ptr, count }.move(), last});
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
		bool Vector<T, Alloc>::operator == (const Vector<T, Alloc> & v) const noexcept{
			if (length() != v.length())
				return false;
			T * i1 = _mem.begin_as<T>();
			for(T * i2 = v._mem.begin_as<T>(); i1 < _mem.end_as<T>(); ++i1, ++i2)
				if (!(*i1 == *i2)) return false;
			return true;
		}
		template<class T, class Alloc>
		bool Vector<T, Alloc>::operator == (const Vector<T, Alloc> & v) const noexcept{
			return !(*this == v);
		}

		template<class T, class Alloc>
		Vector<T, Alloc> && Vector<T, Alloc>::move() noexcept {
			return std::move(*this);
		}
		template<class T, class Alloc>
		inline Result<Vector<T, Alloc>, Error> Vector<T, Alloc>::copy() const noexcept {
			if (length() == 0)
				return Ok(make());
			return Alloc::allocate(sizeof(T) * count)
				.on_success([this](memory::Slice && sl) {
					T * ptr = sl.begin_as<T>();
					for (unsigned i = 0; i < count; ++i)
						new(ptr + i) T(_mem.begin_as<T>()[i].copy());//asserted to be noexcept
					return Ok(sl.move());
				})
				.map_result_type<Vector<T, Alloc>>([this](memory::Slice && sl) {
					T * ptr = sl.begin_as<T>() + length();
					return Ok(Vector<T, Alloc>{sl.move(), ptr});
				})
			;
		}

	#pragma endregion
#pragma endregion
	}
}