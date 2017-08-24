#pragma once
#include "Result.hpp"
#include <utility>

namespace gc {
		namespace detail {
			struct _Ptr : INonCopyable {
				void * _ptr;
				template<class T>
				_Ptr(T * t) : _ptr(t)
				{}
				template<class T>
				T * as() {
					return reinterpret_cast<T *>(_ptr);
				}
				template<class T>
				operator T * () {
					return this->as<T>();
				}
			};
		}
	namespace memory {
		class Slice : INonCopyable {
			char * _data;
			unsigned int _size;
			Slice(char * ptr, unsigned int size) noexcept :
				_data(ptr), _size(size)
			{}
			Slice(char * begin, char * end) noexcept :
				_data(begin), _size(end - begin)
			{}
		public:
			Slice(Slice && other) noexcept {
				*this = other.move();
			}
			void operator = (Slice && other) noexcept {
				_data = other._data;
				_size = other._size;
			}
			Slice && move() noexcept {
				return static_cast<Slice &&>(*this);
			}
			Slice copy() const noexcept {
				return { _data, _size };
			}
			template<class T>
			T * begin_as() const noexcept {
				return reinterpret_cast<T *>(_data);
			}
			template<class T>
			T * end_as() const noexcept {
				return reinterpret_cast<T *>(_data + _size);
			}
			unsigned int size() const noexcept {
				return _size;
			}
			template<class T, class Y>
			static Result<Slice, Error> make(T * begin, Y * end) {
				if (begin > end)
					return Err(Error::InvalidArgument);
				else
					return Ok(std::move(Slice(begin, end)));
			}
			template<class T>
			static Slice make(T * ptr, unsigned size) {
				return std::move(Slice((char *)ptr, size));
			}
			static Slice make(Slice && s) {
				return std::move(s);
			}
			static Slice null() {
				return { nullptr, 0u };
			}
		};
	}
	namespace traits {
		template<class T>
		class is_gc_allocator {
			struct detecter {};

			static constexpr detecter detection(...) {}
			template<class Y>
			static constexpr 
			typename std::enable_if<
				   std::is_same_v<decltype(Y::allocate(std::declval<unsigned int>())), Result<memory::Slice, Error>>//if allocate return result
				&& std::is_same_v<decltype(Y::deallocate(std::declval<gc::memory::Slice &&>())), void> 				//if deallocate return void
				&& noexcept(Y::allocate(std::declval<unsigned>()))													//if allocate is noexcept
				&& noexcept(Y::deallocate(std::declval<gc::memory::Slice>()))										//if deallocate is noexcept
				, void>::type
			detection(Y &&) {}
		public:
			static constexpr bool value = !std::is_same_v<decltype(detection(std::declval<T>())), detecter>;
		};
		template<class T>
		constexpr bool is_gc_allocator_v = is_gc_allocator<T>::value;
	}
}