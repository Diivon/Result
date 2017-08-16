#ifndef GC_RESULT_HPP
#define GC_RESULT_HPP

#include <variant>
#include <utility>


namespace gc {

#pragma region ok, err
	template<class T, class E>
	class Result;

	namespace detail {
		template<class T>
		class _Ok {
			friend _Ok<T> Ok(T && t);
			template<class B>
			friend class ::gc::Result<T, B>;
			T _data;;
			_Ok(T && t) :_data(std::forward<T>(t))
			{}
		};
		template<class T>
		class _Err {
			friend _Err<T> Err(T && t);
			template<class A, class B>
			friend class ::gc::Result;
			T _data;
			_Err(T && t) :_data(std::forward<T>(t))
			{}
		};
	}
	template<class T>
	detail::_Ok<T> Ok(T && t) { return { std::forward<T>(t) }; }
	template<class T>
	detail::_Err<T> Err(T && t) { return { std::forward<T>(t) }; }
#pragma endregion
	
	template<class T, class E>
	class Result {
		std::variant<T, E> _data;
		T & _get_value() noexcept {
			if (is_ok())
				return std::get<0>(_data);
			else
				return *((T*)nullptr);
		}
		E & _get_error() noexcept {
			if (is_err())
				return std::get<1>(_data);
			else
				return *((E*)nullptr);
		}
	public:
		Result() noexcept :
			_data(std::in_place_index<1>)
		{
			static_assert(std::is_nothrow_default_constructible<E>::value, "gc::Result<T, E> requires nothrow default constructor for E");
		}
		Result(detail::_Ok<T> && ok) noexcept :
			_data(std::in_place_index<0>, std::move(ok._data))
		{
			static_assert(std::is_nothrow_move_constructible<T>::value, "gc::Result<T, E> requires nothrow move constructor for T");
		}
		Result(detail::_Err<E> && err) noexcept :
			_data(std::in_place_index<1>, std::move(err._data))
		{
			static_assert(std::is_nothrow_move_constructible<E>::value, "gc::Result<T, E> requires nothrow move constructor for E");
		}
		Result(Result && other) noexcept :
			_data(std::move(other._data))
		{
			static_assert(std::is_nothrow_move_constructible<T>::value, "gc::Result<T, E> requires nothrow move constructor for T");
		}
		bool is_ok() const noexcept {
			return std::holds_alternative<T>(_data);
		}
		bool is_err() const noexcept {
			return !std::holds_alternative<T>(_data);
		}
		template<class F, class Y = decltype(std::declval<F>()(std::move(std::declval<T>())))>
		auto map_result(F && f) noexcept {
			static_assert(!std::is_same<decltype(f(std::move(std::declval<T>()))), void>::value, "gc::Result::map() argument cannot return void");

			if (is_ok())
				return Result<Y, E>(Ok(f(std::move(_get_value()))));
			else
				return Result<Y, E>(Err(std::move(_get_error())));
		}
		template<class F, class Y = decltype(std::declval<F>()(std::move(std::declval<T>())))>
		auto map_error(F && f) noexcept {
			static_assert(!std::is_same<decltype(f(std::move(std::declval<T>()))), void>::value, "gc::Result::map() argument cannot return void");

			if (is_ok())
				return Result<Y, E>(Ok(std::move(_get_value())));
			else
				return Result<Y, E>(Err(f(std::move(_get_error()))));
		}
		template<class F>
		Result & on_success(F && f) noexcept {
			if (is_ok())
				f(_get_value());
			return *this;
		}
		
		template<class F>
		typename std::enable_if<std::is_same<decltype(std::declval<F>()(std::declval<E &>())), void>::value, Result &>::type 
		on_fail(F && f) noexcept {
			//TODO: add another method with arg that takes no args
			static_assert(std::is_same<decltype(f(std::declval<E &>())), void>::value, "gc::Result::on_fail() argument cannot return anything(use map_err_to_value to convert err -> value)");

			if (is_err())
				f(_get_error());
			return *this;
		}
		template<class F>
		typename std::enable_if<!std::is_same<decltype(std::declval<F>()(std::declval<E &>())), void>::value, Result &>::type
		on_fail(F && f) {
			static_assert(std::is_nothrow_constructible<T, decltype(std::declval<F>()(std::declval<E &>()))>::value, "gc::Result::on_fail argument must return value, which can be used to construct T with no exceptions");
			if (is_err())
				_data.emplace<T>(f(_get_error()));
			return *this;
		}
		T unwrap_value() {
			return std::move(_get_value());
		}
		T unwrap_value_or(T && t) {
			static_assert(std::is_same<T, std::remove_reference<T>::type>::value, "gc::Result::unwrap_value_or takes only rvalue references as argument");

			if (is_ok())
				return std::move(_get_value());
			else
				return std::move(t);
		}
		template<class F>
		T unwrap_value_or_do(F && f) {
			static_assert(std::is_same<decltype(std::declval<F>()()), void>::value, "gc::Result::unwrap_value_or_do cannot return void");

			if (is_ok())
				return std::move(_get_value());
			else
				return std::move(f());
		}
		E unwrap_error() {
			return std::move(_get_error());
		}
	};
}
#endif // include guard end