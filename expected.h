#ifndef QUERY_LANG_EXPECTED_H
#define QUERY_LANG_EXPECTED_H

#include <type_traits>
#include <variant>


struct ExpectTag_t {
};

static constexpr ExpectTag_t ExpectTag{};

struct UnexpectTag_t {
};

static constexpr UnexpectTag_t UnexpectTag{};

template <typename E>
class Unexpected {
public:
    explicit Unexpected(E const &e)
        : err(e) {
    }

    explicit Unexpected(E &&e)
        : err(std::move(e)) {
    }

    auto error() & -> E & { return err; }
    auto error() const & -> E const & { return err; }
    auto error() && -> E && { return std::move(err); }
    auto error() const && -> E const && { return std::move(err); }

    constexpr auto swap(
        Unexpected &other) noexcept(std::is_nothrow_swappable_v<E>)
        requires std::is_swappable_v<E> {
        return std::swap(this->err, other.err);
    }

private:
    E err;
};

template <typename T, typename E>
    requires std::is_destructible_v<T> &&
             (!std::convertible_to<T, Unexpected<E>>)
class Expected {
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = Unexpected<E>;

    constexpr Expected(const Expected &other) = default;

    constexpr explicit Expected(value_type &&value)
        : _variant(std::move(value)) {
    }

    constexpr explicit Expected(error_type &&e)
        : _variant(unexpected_type{std::move(e)}) {
    }

    template <typename U>
    constexpr Expected(ExpectTag_t, U const &value)
        : _variant(value_type{value}) {
    }

    template <typename U>
    constexpr Expected(UnexpectTag_t, U const &err)
        : _variant(unexpected_type{err}) {
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
        return std::holds_alternative<value_type>(_variant);
    }

    [[nodiscard]] constexpr auto has_error() const noexcept -> bool {
        return not has_value();
    }

    [[nodiscard]] constexpr auto value() & -> value_type & {
        return std::get<value_type>(_variant);
    }

    [[nodiscard]] constexpr auto value() const & -> value_type const & {
        return std::get<value_type>(_variant);
    }

    [[nodiscard]] constexpr auto value() && -> value_type && {
        return std::get<value_type>(std::move(_variant));
    }

    [[nodiscard]] constexpr auto value() const && -> value_type const && {
        return std::get<value_type>(std::move(_variant));
    }

    [[nodiscard]] constexpr auto error() & -> error_type & {
        return std::get<unexpected_type>(_variant).error();
    }

    [[nodiscard]] constexpr auto error() const & -> error_type const & {
        return std::get<unexpected_type>(_variant).error();
    }

    [[nodiscard]] constexpr auto error() && -> error_type && {
        return std::get<unexpected_type>(std::move(_variant)).error();
    }

    [[nodiscard]] constexpr auto error() const && -> error_type const && {
        return std::get<unexpected_type>(std::move(_variant)).error();
    }

    template <typename U>
        requires std::convertible_to<U &&, T> && std::move_constructible<T>
    [[nodiscard]] constexpr auto value_or(U &&default_value) && -> T {
        if (has_value()) {
            return std::move(this->value());
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
        requires std::convertible_to<U &&, T> && std::copy_constructible<T>
    [[nodiscard]] constexpr auto value_or(U &&default_value) const & -> T {
        if (has_value()) {
            return value();
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    template <typename Fn>
        requires std::is_invocable_v<Fn, T &>
    [[nodiscard]] constexpr auto transform(Fn &&fn) & {
        if (has_value()) {
            return Expected<std::remove_cv_t<std::invoke_result_t<Fn>>, E>{
                ExpectTag, fn(value())
            };
        }
        return Expected{UnexpectTag, error()};
    }

    template <typename Fn>
        requires std::is_invocable_v<Fn, T const &>
    [[nodiscard]] constexpr auto transform(Fn &&fn) const & {
        if (has_value()) {
            return Expected<std::remove_cv_t<std::invoke_result_t<Fn>>, E>{
                ExpectTag, fn(value())
            };
        }
        return Expected{UnexpectTag, error()};
    }

    template <typename Fn>
        requires std::is_invocable_v<Fn, T &&>
    [[nodiscard]] constexpr auto transform(Fn &&fn) && {
        if (has_value()) {
            return Expected<std::remove_cv_t<std::invoke_result_t<Fn>>, E>{
                ExpectTag, fn(std::move(value()))
            };
        }
        return Expected{UnexpectTag, error()};
    }

    template <typename Fn>
        requires std::is_invocable_v<Fn, T const &&>
    [[nodiscard]] constexpr auto transform(Fn &&fn) && {
        if (has_value()) {
            return Expected<std::remove_cv_t<std::invoke_result_t<Fn>>, E>{
                ExpectTag, fn(std::move(value()))
            };
        }
        return Expected{UnexpectTag, error()};
    }

private:
    constexpr Expected() = default;

    std::variant<value_type, unexpected_type> _variant;
};

#endif //QUERY_LANG_EXPECTED_H