#pragma once
#include "Result.hpp"
#include "Memory.h"

namespace gc {
	namespace memory {
		class Allocator {
		public:
			gc::Result<Slice, gc::Error> allocate(unsigned int size) noexcept {
				auto res = new(std::nothrow) char[size];
				if (res) 
					return gc::Err(gc::Error::BadAlloc);
				return gc::Ok(Slice{res, size});
			}
			gc::Result<Slice, gc::Error> deallocate(Slice && slice) noexcept {
				delete(std::nothrow, slice.begin_as<char>());
			}
		};
	}
	namespace traits {
		template<class T>
		struct is_gc_allocator {
			
		};
	}
}