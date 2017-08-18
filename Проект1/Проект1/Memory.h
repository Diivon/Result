#pragma once
#include "Result.hpp"

namespace gc {
		namespace detail {
			struct _Ptr {
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
		class Slice {
			char * _data;
			unsigned int _size;
		public:
			Slice(char * ptr, unsigned int size) noexcept :
				_data(ptr), _size(size)
			{}
			Slice(char * begin, char * end) noexcept :
				_data(begin), _size(end - begin)
			{}
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
		};
	}
	template<>
	Result<memory::Slice, Error> make<memory::Slice>(detail::_Ptr ptr, unsigned int size) {
		return Ok(memory::Slice{ ptr.as<char>(), size });
	}
	template<>
	Result<memory::Slice, Error> make<memory::Slice>(detail::_Ptr beg, detail::_Ptr end) {
		if (beg.as<char>() > end.as<char>())
			return Err(Error::InvalidArgument);
		return Ok(memory::Slice{ beg.as<char>(), end.as<char>()});
	}
}