#pragma once

#include <utility>

template <auto ... I>
struct constexpr_sequence
{};

// returns the last element
template <auto Head, auto ... I>
struct Tail
{
    constexpr auto operator()() noexcept
    {
        if constexpr (sizeof ... (I) == 0)
            return Head;
        
        else
            return Tail<I...>{}();
    }
};


template <auto condition, auto modifier, auto ... I>
constexpr auto make_constexpr_sequence_impl()
{
    constexpr auto tail = Tail<I...>{}();

    if constexpr (condition(modifier(tail)))
        return make_constexpr_sequence_impl<condition, modifier, I ..., modifier(tail)>();
    
    else
        return constexpr_sequence<I ...>{};
}


template <auto I, auto condition, auto modifier>
constexpr auto make_constexpr_sequence()
{
    return make_constexpr_sequence_impl<condition, modifier, I>();
}


template <typename LoopBody, auto I, auto condition, auto modifier>
struct constexpr_for_impl
{
    LoopBody loop_body;
    bool body_executed = false; // does this prevent constexprness?

    constexpr auto operator()(auto&& ... args)
    {
        body_executed = true;

        constexpr auto sequence = make_constexpr_sequence<I, condition, modifier>();

        [&] <auto ... Is> (constexpr_sequence<Is...>)
        {
            (loop_body.template operator()<Is>(std::forward<decltype(args)>(args)...), ...);
        }(sequence);
    }

    constexpr ~constexpr_for_impl()
    {
        if (body_executed)
            return;

        constexpr auto sequence = make_constexpr_sequence<I, condition, modifier>();

        if constexpr ( requires { loop_body(I); } )
        {
            [&] <auto ... Is> (constexpr_sequence<Is...>)
            {
                (loop_body(Is), ...);
            }(sequence);
        }
        
        else if constexpr (requires { loop_body.template operator()<I>(); })
        {
            [&] <auto ... Is> (constexpr_sequence<Is...>)
            {
                (loop_body.template operator()<Is>(), ...);
            }(sequence);
        }

        else if constexpr ( requires { loop_body(); } )
        {
            [&] <auto ... Is> (constexpr_sequence<Is...>)
            {
                int arr[] = {((void)loop_body(), Is, 1)...};
            }(sequence);
        }
    }
};

template <auto I, auto condition, auto modifier>
constexpr auto constexpr_for(auto loop_body)
{
    return constexpr_for_impl<decltype(loop_body), I, condition, modifier>{loop_body};
};
