#pragma once
#include <variant>

#include "Traits.h"


namespace gc {

#pragma region ok, err
	namespace detail {
		template<class T>
		struct _Ok {
			T _data;
			explicit _Ok(T && t) :_data(std::forward<T>(t))
			{}
		};
		template<class T>
		struct _Err {
			T _data;
			explicit _Err(T && t) :_data(std::forward<T>(t))
			{}
		};
	}
	template<class T>
	detail::_Ok<T> Ok(T && t) {
		static_assert(std::is_nothrow_move_constructible<T>::value,
			"gc::Ok<T> requires nothrow move constructor for T");
		return detail::_Ok<T>{ std::move(t) };
	}
	template<class T>
	detail::_Err<T> Err(T && t) { 
		static_assert(std::is_nothrow_move_constructible<T>::value,
			"gc::Err<T> requires nothrow move constructor for T");
		return detail::_Err<T>{ std::move(t) };
	}
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
			static_assert(std::is_nothrow_default_constructible<E>::value, 
				"default constructor for gc::Result<T, E> requires nothrow default constructor for E");
		}
		Result(detail::_Ok<T> && ok) noexcept :
			_data(std::in_place_index<0>, std::move(ok._data))
		{
			static_assert(std::is_nothrow_move_constructible<T>::value, 
				"gc::Result<T, E>(Ok(T)) requires nothrow move constructor for T");
		}
		Result(detail::_Err<E> && err) noexcept :
			_data(std::in_place_index<1>, std::move(err._data))
		{
			static_assert(std::is_nothrow_move_constructible<E>::value, 
				"gc::Result<T, E>(Err(E)) requires nothrow move constructor for E");
		}
		Result(Result && other) noexcept :
			_data(std::move(other._data))
		{
			static_assert(std::is_nothrow_move_constructible<T>::value,
				"gc::Result<T, E>(gc::Result<T, E> &&) requires nothrow move constructor for T");
			static_assert(std::is_nothrow_move_constructible<E>::value,
				"gc::Result<T, E>(gc::Result<T, E> &&) requires nothrow move constructor for E");
		}
		void operator = (Result && r) {
			_data = std::move(r._data);
		}
		void operator = (detail::_Err<E> && e) {
			_data.emplace<E>(std::move(e._data));
		}
		void operator = (detail::_Ok<T> && v) {
			_data.emplace<T>(std::move(v._data));
		}
		bool is_ok() const noexcept {
			return std::holds_alternative<T>(_data);
		}
		bool is_err() const noexcept {
			return !is_ok();
		}
		template<class Y = gc::traits::return_type<F, gc::traits::rvalue_reference<T>::type>::type, class F>
		auto map_result_type(F && f) noexcept {
			static_assert(!std::is_same<gc::traits::return_type<F, gc::traits::rvalue_reference<T>::type>::type, void>::value,
				"gc::Result::map() argument cannot return void");

			if (is_ok())
				return Result<Y, E>(Ok(f(std::move(_get_value()))));
			else
				return Result<Y, E>(Err(std::move(_get_error())));
		}
		template<class F, class Y = decltype(std::declval<F>()(std::move(std::declval<T>())))>
		auto map_error_type(F && f) noexcept {
			static_assert(!std::is_same<decltype(f(std::move(std::declval<T>()))), void>::value, 
				"gc::Result::map() argument cannot return void");

			if (is_ok())
				return Result<Y, E>(Ok(std::move(_get_value())));
			else
				return Result<Y, E>(Err(f(std::move(_get_error()))));
		}
		template<class F>
		Result & on_success(F && f) noexcept {
			static_assert(gc::traits::is_able_to_call<F, gc::traits::rvalue_reference<T>::type>::value,
				"gc::Result<T, E>::on_success(f); f must take T && as argument");
			static_assert(std::is_same<gc::Result<T, E>, gc::traits::return_type<F, gc::traits::rvalue_reference<T>::type>::type>::value
				|| std::is_same<gc::detail::_Err<E>, gc::traits::return_type<F, gc::traits::rvalue_reference<T>::type>::type>::value
				|| std::is_same<gc::detail::_Ok<T>, gc::traits::return_type<F, gc::traits::rvalue_reference<T>::type>::type>::value
				,"gc::Result<T, E>::on_success(f); f must return gc::Result<T, E>");

			if (is_ok())
				*this = std::move(f(std::move(_get_value())));
			return *this;
		}
		template<class F>
		Result & on_error(F && f) {
			static_assert(gc::traits::is_able_to_call<F, gc::traits::rvalue_reference<E>::type>::value,
				"gc::Result<T, E>::on_error(f); f must take E && as argument");
			static_assert(std::is_same<gc::Result<T, E>, gc::traits::return_type<F, gc::traits::rvalue_reference<E>::type>::type>::value,
				"gc::Result<T, E>::on_error(f); f must return gc::Result<T, E>");

			if (is_err())
				*this = f(std::move(_get_error())).move();
			return *this;
		}
		T unwrap_value() {
			return std::move(_get_value());
		}
		T unwrap_value_or(T && t) {
			static_assert(std::is_same<T, std::remove_reference<T>::type &&>::value, 
				"gc::Result::unwrap_value_or takes only rvalue references as argument");

			if (is_ok())
				return std::move(_get_value());
			else
				return std::move(t);
		}
		template<class F>
		T unwrap_value_or_do(F && f) {
			static_assert(!std::is_same<decltype(std::declval<F>()()), void>::value,
				"gc::Result::unwrap_value_or_do argument cannot return void");

			if (is_ok())
				return std::move(_get_value());
			else
				return std::move(f());
		}
		E unwrap_error() {
			return std::move(_get_error());
		}
		//***************************gc::IClass implementation************************
		Result && move() {
			return std::move(*this);
		}
	};

	enum class Error {
		InvalidArgument,
		DomainError,
		SizeError,
		OutOfRange,
		FutureError,
		RangeError,
		OverflowError,
		UnderflowError,
		BadAlloc,
		InsufficientRights,
		UnknownError
	};

	template<class T, class ... Args>
	Result<T, Error> make(Args ... args) {
		return Ok(T{std::forward<Args>(args)...});
	}
}