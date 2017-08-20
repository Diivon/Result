#pragma once
#include <variant>

#include "Traits.hpp"


namespace gc {

#pragma region ok, err
	namespace detail {
		template<class T>
		struct _Ok {
			static_assert(!std::is_reference_v<T>,
				"gc::detail::_Ok<T> cannot contain reference as T");
			T _data;
			explicit _Ok(T && t) :_data(std::forward<T>(t))
			{}
		};
		template<class T>
		struct _Err {
			static_assert(!std::is_reference_v<T>,
				"gc::detail::_Err<T> cannot contain reference as T");
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
		static_assert(!std::is_reference_v<T>,
			"gc::Result<T, E> cannot contain reference as T");
		static_assert(!std::is_reference_v<E>,
			"gc::Result<T, E> cannot contain reference as E");
		std::variant<T, E> _data;
		T && _get_value() noexcept {
			if (is_ok())
				return std::move(std::get<0>(_data));
			else
				return std::move(*((T*)nullptr));
		}
		E && _get_error() noexcept {
			if (is_err())
				return std::move(std::get<1>(_data));
			else
				return std::move(*((E*)nullptr));
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
			return _data.index() == 0;
		}
		bool is_err() const noexcept {
			return !is_ok();
		}
		template<class Y, class F>
		Result<Y, E> map_result_type(F && f) noexcept {
			static_assert(gc::traits::function::is_able_to_call<F, T &&>::value,
				"gc::Result<T, E>::map_result_type<Y>(f); f must be callable with T && as argument");
			static_assert(!gc::traits::function::is_return_void_v<F, T &&>,
				"gc::Result<T, E>::map_result_type<Y>(f) -> gc::Result<Y, E> f cannot return void");
			static_assert(std::is_nothrow_constructible_v<Result<Y, E>, gc::traits::function::return_type<F, T &&>::type>,
				"gc::Result<T, E>::map_result_type<Y>(f) -> gc::Result<Y, E> f must return value, which can be used to construct Y with no exceptions");

			if (is_ok())
				return std::move(f(_get_value()));
			else
				return std::move(Err(_get_error()));
		}
		template<class Y, class F>
		Result<T, Y> map_error_type(F && f) noexcept {
			static_assert(gc::traits::function::is_able_to_call<F, E &&>::value,
				"gc::Result<T, E>::map_error_type<Y>(f); f must be callable with T && as argument");
			static_assert(!gc::traits::function::is_return_void_v<F, E&&>,
				"gc::Result<T, E>::map_error_type<Y>(f) -> gc::Result<T, Y> f cannot return void");
			static_assert(std::is_nothrow_constructible_v<Y, gc::traits::function::return_type<F, E &&>::type>,
				"gc::Result<T, E>::map_error_type<Y>(f) -> gc::Result<T, Y> f must return value, which can be used to construct Y with no exceptions");

			return std::move(is_ok() ? Ok(_get_value()) : f(_get_error()));
		}
		template<class F>
		Result & on_success(F && f) noexcept {
			static_assert(gc::traits::function::is_able_to_call<F, T &&>::value,
				"gc::Result<T, E>::on_success(f) f must be callable with T && as argument");
			static_assert(!gc::traits::function::is_return_void_v<F, T &&>,
				"gc::Result<T, E>::on_success(f) f cannot return void");
			static_assert(std::is_nothrow_constructible_v<Result, gc::traits::function::return_type<F, T &&>::type>,
				"gc::Result<T, E>::on_success(f) f must return value, which can be used to construct gc::Result<T, E> with no exceptions");

			if (is_ok())
				*this = std::move(f(_get_value()));
			return *this;
		}
		template<class F>
		Result & on_error(F && f) {
			static_assert(gc::traits::function::is_able_to_call<F, E &&>::value,
				"gc::Result<T, E>::on_error(f); f must be callable with E && as argument");
			static_assert(!gc::traits::function::is_return_void_v<F, E &&>,
				"gc::Result<T, E>::on_success(f) f cannot return void");
			static_assert(std::is_nothrow_constructible_v<decltype(*this), gc::traits::function::return_type<F, E &&>::type>,
				"gc::Result<T, E>::on_success(f) f must return value, which can be used to construct gc::Result<T, E> with no exceptions");

			if (is_err())
				*this = std::move(f(_get_error()));
			return *this;
		}
		T unwrap_value() {
			return std::move(_get_value());
		}
		template<class ... Args>
		T unwrap_value_or(Args && ... args) {
			static_assert(std::is_nothrow_constructible_v<T, Args ...>,
				"gc::Result<T, E>::unwrap_value_or(args ...) T{args ...} must be nothrow constructible");

			if (is_ok())
				return _get_value();
			else
				return {std::forward<Args>(args)...};
		}
		template<class F>
		T unwrap_value_or_do(F && f) {
			static_assert(gc::traits::function::is_able_to_call_v<F>, 
				"gc::Result<T, E>::unwrap_value_or_do(f) f must be callable with no arguments (use gc::Result<T, E>::on_error to map error -> value)");
			static_assert(!gc::traits::function::is_return_void_v<F>,
				"gc::Result<T, E>::unwrap_value_or_do(f) f cannot return void");
			static_assert(std::is_nothrow_constructible_v<T, gc::traits::function::return_type<F>::type>,
				"gc::Result<T, E>::unwrap_value_or_do(f) f must return value, which can be used to noexcept construct T");

			return std::move(is_ok() ? _get_value() : f());
		}
		E unwrap_error() {
			return std::move(_get_error());
		}
		template<class ... Args>
		E unwrap_error_or(Args && ... args){
			static_assert(std::is_nothrow_constructible_v<E, Args ...>,
				"gc::Result<T, E>::unwrap_value_or(args ...) T{args ...} must be nothrow constructible");
			if (is_err())
				return _get_error();
			return {std::forward<Args>(args)...};
		}
		template<class F>
		E unwrap_error_or_do(F && f) {
			static_assert(gc::traits::function::is_able_to_call_v<F>,
				"gc::Result<T, E>::unwrap_error_or_do argument must be callable with no arguments (use gc::Result<T, E>::on_error to map error -> value)");
			static_assert(!gc::traits::function::is_return_void_v<F>,
				"gc::Result<T, E>::unwrap_error_or_do(f) f cannot return void");
			static_assert(std::is_nothrow_constructible_v<E, gc::traits::function::return_type<F>::type>,
				"gc::Result<T, E>::unwrap_error_or_do argument must return value, which can be used to noexcept construct E");

			return std::move(is_ok() ? f() : _get_error());
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