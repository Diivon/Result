#pragma once

namespace gc {
	namespace traits {
		namespace function {
			template<class T, class ... Args>
		//is_able_to_call
			class is_able_to_call {
				struct detection {};
				template<class Y>
				static decltype(std::declval<Y>()(std::forward<Args>(std::declval<Args>())...))
					detect(const Y &) {}
				static detection
					detect(...) {}
			public:
				///if type T has round brackets which takes passed arguments, then value == true
				static constexpr bool value = !std::is_same<detection, decltype(detect(std::declval<T>()))>::value;
			};
			template<class T, class ... Args>
			constexpr bool is_able_to_call_v = is_able_to_call<T, Args ... >::value;
		//return_type
			template<class T, class ... Args>
			class return_type {
				static_assert(is_able_to_call<T, Args...>::value, "passing noncallable object to 'return_type' metafunction");
			public:
				///type contain type of return value of functor
				using type = decltype(std::declval<T>()(std::forward<Args>(std::declval<Args>())...));
			};
		//is_return_void
			template<class F, class ... Args>
			class is_return_void {
				static_assert(is_able_to_call<F, Args ...>::value, "passing noncallable object to 'is_return_void' metafunction");
			public:
				static constexpr bool value = std::is_same<void, return_type<F, Args ...>::type>::value;
			};
			template<class T, class ... Args>
			constexpr bool is_return_void_v = is_return_void<T, Args ... >::value;
		}
		namespace type {
		//lvalue_reference
			template<class T>
			struct lvalue_reference {
				using type = T &;
			};
			template<class T>
			struct lvalue_reference<T &> {
				using type = T &;
			};
			template<class T>
			struct lvalue_reference<T &&> {
				using type = T &;
			};
			
		//rvalue_reference
			template<class T>
			struct rvalue_reference {
				using type = T &&;
			};
			template<class T>
			struct rvalue_reference<T &> {
				using type = T &&;
			};
			template<class T>
			struct rvalue_reference<T &&> {
				using type = T &&;
			};
		//is_const
			template<class T>
			struct is_const {
				static constexpr bool value = false;
			};
			template<class T>
			struct is_const<const T> {
				static constexpr bool value = true;
			};
		}
	}
	class INonCopyable {
		INonCopyable(const INonCopyable &) = delete;
		void operator = (const INonCopyable &) = delete;
	};
	class INonMoveable {
		INonMoveable(INonMoveable &&) = delete;
		void operator = (INonCopyable &&) = delete;
	};
	class INonConstructible : INonCopyable, INonMoveable {
		INonConstructible() = delete;
	};
	template<class T>
	struct TypeName
	{
		inline static std::string get() {
			static constexpr size_t FRONT_SIZE = sizeof("gc::TypeName<");
			static constexpr size_t BACK_SIZE = sizeof(">::get");
			static const char * firstPtr = __FUNCTION__ + FRONT_SIZE - 1;
			static const char * lastPtr = __FUNCTION__ + sizeof(__FUNCTION__) - BACK_SIZE;
			return { firstPtr, (const unsigned)(lastPtr - firstPtr) };
		}
	};
}