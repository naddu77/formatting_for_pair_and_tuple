import std;

using namespace std::literals;

#define STATICALLY_WIDEN(STR)                                   \
    []                                                          \
    {                                                           \
        if constexpr (std::is_same_v<CharT, char>)              \
        {                                                       \
            return STR##sv;                                     \
        }                                                       \
        else if constexpr (std::is_same_v<CharT, wchar_t>)      \
        {                                                       \
            return L##STR##sv;                                  \
        }                                                       \
    }()

namespace std
{
    template <template <typename...> typename TupleLike, typename CharT, typename... Ts>
    struct formatter<TupleLike<Ts...>, CharT>
    {
    public:
        constexpr void set_separator(basic_string_view<CharT> sep)
        {
            separator = sep;
        }

        constexpr void set_brackets(basic_string_view<CharT> opening, basic_string_view<CharT> closing)
        {
            opening_bracket = opening;
            closing_bracket = closing;
        }

        template <typename ParseContext>
        constexpr typename ParseContext::iterator parse(ParseContext& ctx)
        {
            auto it{ std::begin(ctx) };
            auto e_it{ std::end(ctx) };

            if (it != e_it)
            {
                if (*it == STATICALLY_WIDEN("m")[0])
                {
                    if constexpr (sizeof...(Ts) == 2)
                    {
                        set_separator(STATICALLY_WIDEN(": "));
                        set_brackets({}, {});
                        ++it;
                    }
                }
                else if (*it == STATICALLY_WIDEN("n")[0])
                {
                    set_brackets({}, {});
                    ++it;
                }
            }

            return it;
        }

        template <typename FormatContext, std::size_t... Is>
        constexpr typename FormatContext::iterator format_impl(TupleLike<Ts...> const& elems, FormatContext& ctx, std::index_sequence<Is...>) const
        {
            auto out{ ctx.out() };

            if (!std::empty(opening_bracket))
            {
                out = std::format_to(out, STATICALLY_WIDEN("{}"), opening_bracket);
            }
            
            ((Is == 0 ? out = std::format_to(out, STATICALLY_WIDEN("{}"), std::get<Is>(elems)) : std::format_to(out, STATICALLY_WIDEN("{}{}"), separator, std::get<Is>(elems))), ...);

            if (!std::empty(closing_bracket))
            {
                out = std::format_to(out, STATICALLY_WIDEN("{}"), closing_bracket);
            }

            return out;
        }

        template <typename FormatContext>
        constexpr typename FormatContext::iterator format(TupleLike<Ts...> const& elems, FormatContext& ctx) const
        {
            return format_impl(elems, ctx, std::make_index_sequence<sizeof...(Ts)>{});
        }

    private:
        //tuple<formatter<remove_cvref_t<Ts>, CharT>...> underlying;
        basic_string_view<CharT> separator{ STATICALLY_WIDEN(",") };
        basic_string_view<CharT> opening_bracket{ STATICALLY_WIDEN("(")};
        basic_string_view<CharT> closing_bracket{ STATICALLY_WIDEN(")")};
    };
}

class TestClass
{

};

int main()
{
    std::tuple t{ 1, L"2"s };

    std::wcout << std::format(L"{}\n", t);
    std::wcout << std::format(L"{:m}\n", t);
    std::wcout << std::format(L"{:n}\n", t);

    std::tuple t2{ 1, L"2"s, 3.0 };

    std::wcout << std::format(L"{}\n", t2);
    //std::wcout << std::format(L"{:m}\n", t2); // std::format_error
    std::wcout << std::format(L"{:n}\n", t2);
    //std::wcout << std::format(L"{:t}", t2);   // std::format_error

    std::pair p1{ 1, L"1"s };

    std::wcout << std::format(L"{}\n", p1);
    std::wcout << std::format(L"{:m}\n", p1);
    std::wcout << std::format(L"{:n}\n", p1);

    std::pair<TestClass, int> p2{ {}, 1 };

    std::tuple<int, int, int> t3{ 1, 2, 3 };

    std::cout << std::format("{}\n", t3);

    //std::wcout << std::format(L"{}\n", p2); // static_assert failed
}
