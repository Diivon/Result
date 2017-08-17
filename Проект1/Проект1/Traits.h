#pragma once

namespace gc {
	namespace traits {
		template<class T, class ... Args>
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
		class return_type {
			static_assert(is_able_to_call<T, Args...>::value, "passing noncallable object to 'return_type' metafunction");
		public:
			///type contain type of return value of functor
			using type = decltype(std::declval<T>()(std::forward<Args>(std::declval<Args>())...));
		};
		template<class T>
		struct rvalue_reference {
			using type = T &&;
		};
		template<class T>
		struct rvalue_reference<T &> {
			using type = typename std::remove_reference<T>::type &&;
		};
		template<class T>
		struct rvalue_reference<T &&> {
			using type = T &&;
		};
		template<class T>
		struct TypeName
		{
			inline static std::string get() {
				static constexpr size_t FRONT_SIZE = sizeof("gc::traits::TypeName<");
				static constexpr size_t BACK_SIZE = sizeof(">::get");
				static const char * firstPtr = __FUNCTION__ + FRONT_SIZE - 1;
				static const char * lastPtr = __FUNCTION__ + sizeof(__FUNCTION__) - BACK_SIZE;
				return std::string(firstPtr, lastPtr - firstPtr);
			}
		};
#define GC_GENERATE_IS_HAS_TYPEDEF(_arg_type)\
		template<class T>\
		class is_has_typedef_##_arg_type{\
			struct detection{};\
			static detection\
				detect(...);\
			template<class Y>\
			static typename Y::##_arg_type\
				detect(const Y &);\
		public:\
			static constexpr bool value = !std::is_same<detection, decltype(detect(std::declval<T>()))>::value;\
		};
#define GC_GENERATE_IS_HAS_METHOD(_arg_meth)\
		template<class T, class ... Args>\
		class is_has_method_##_arg_meth{\
			struct detection{};\
			static detection \
				detect(...);\
			template<class Y>\
			static decltype(std::declval<Y>().##_arg_meth(std::forward<Args>(std::declval<Args>())...))\
				detect(const Y &);\
		public:\
			static constexpr bool value = !std::is_same<detection, decltype(detect(std::declval<T>()))>::value;\
		}
	}
}