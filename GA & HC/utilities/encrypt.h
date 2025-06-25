#pragma once

#include <string>

#ifdef _KERNEL_MODE
namespace std
{
    template <class _Ty>
    struct remove_reference {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_reference<_Ty&> {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_reference<_Ty&&> {
        using type = _Ty;
    };

    template <class _Ty>
    using remove_reference_t = typename remove_reference<_Ty>::type;

    template <class _Ty>
    struct remove_const {
        using type = _Ty;
    };

    template <class _Ty>
    struct remove_const<const _Ty> {
        using type = _Ty;
    };

    template <class _Ty>
    using remove_const_t = typename remove_const<_Ty>::type;
}
#else
#include <type_traits>
#endif

namespace skc
{
    template<class _Ty>
    using clean_type = typename std::remove_const_t<std::remove_reference_t<_Ty>>;

    template <int _size, char _key1, char _key2, typename T>
    class E_CRYPTer
    {
    public:
        constexpr E_CRYPTer(const T(&data)[_size]) {
            for (int i = 0; i < _size; i++) {
                _storage[i] = data[i] ^ static_cast<T>(_key1 + i % (1 + _key2));
            }
        }

        std::basic_string<T> decrypt() const {
            std::basic_string<T> result;
            result.reserve(_size);
            for (int i = 0; i < _size; i++) {
                result += _storage[i] ^ static_cast<T>(_key1 + i % (1 + _key2));
            }
            return result;
        }

    private:
        T _storage[_size]{};
    };
}

// Macro for encrypting string literals (char or wchar_t)
#define _encrypt(str) ([]() { \
    using T = skc::clean_type<decltype(str[0])>; \
    constexpr auto crypted = skc::E_CRYPTer<sizeof(str)/sizeof(T), __TIME__[1], __TIME__[7], T>(str); \
    return crypted.decrypt(); \
}())

// Macro for std::string or std::wstring
#define encrypt(str) _encrypt(str).c_str()