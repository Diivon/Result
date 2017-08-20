#pragma once
#include "Result.hpp"
#include "Memory.hpp"

namespace gc {
	namespace memory {
		class Allocator {
		public:
			static gc::Result<Slice, gc::Error> allocate(unsigned int size) noexcept {
				auto res = new(std::nothrow) char[size];
				if (!res)
					return gc::Err(gc::Error::BadAlloc);
				return gc::Ok(Slice::make(res, size));
			}
			static void deallocate(Slice && slice) noexcept {
				delete(std::nothrow, slice.begin_as<char>());
			}
		};
	}
}