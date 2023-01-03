#pragma once

#include <utility>

namespace metautils::details
{

template<auto ... I>
struct constexpr_sequence
{
};

// returns the last element
template<auto Head, auto ... I>
struct Tail
{
	constexpr auto operator()() const

	noexcept
	{
		if constexpr(sizeof ... (I) == 0)
		return Head;

		else
		return Tail<I...>{}();
	}
};


template<auto condition, auto modifier, auto ... I>
constexpr auto make_constexpr_sequence_impl()

noexcept
{
constexpr auto tail = Tail<I...>{}();

if

constexpr (condition(modifier(tail)))

return

make_constexpr_sequence_impl<condition, modifier, I ..., modifier(tail)>();

else
return constexpr_sequence<I ...>
{
};
}


template<auto I, auto condition, auto modifier>
constexpr auto make_constexpr_sequence()

noexcept
{
return

make_constexpr_sequence_impl<condition, modifier, I>();

}


template<typename LoopBody, auto I, auto condition, auto modifier>
struct constexpr_for_impl
{
	LoopBody&& loop_body;
	bool body_executed = false; // does this prevent constexprness?

	constexpr auto operator()(auto&& ... args)
	{
		body_executed = true;

		constexpr auto sequence = make_constexpr_sequence<I, condition, modifier>();

		if constexpr(requires{loop_body.template operator()<I>(args...);})
		{
			[&]<auto ... Is>(constexpr_sequence<Is...>)
			{
				(loop_body.template operator()<Is>(std::forward<decltype(args)>(args)...), ...);
			}(sequence);
		}

		else if constexpr(requires{loop_body(I, args...);})
		{
			[&]<auto ... Is>(constexpr_sequence<Is...>)
			{
				(loop_body(Is, std::forward<decltype(args)>(args)...), ...);
			}(sequence);
		}
	}

	constexpr ~constexpr_for_impl()
	{
		if (body_executed)
			return;

		constexpr auto sequence = make_constexpr_sequence<I, condition, modifier>();

		if constexpr(requires{loop_body(I);})
		{
			[&]<auto ... Is>(constexpr_sequence<Is...>)
			{
				(loop_body(Is), ...);
			}(sequence);
		}

		else if constexpr(requires{loop_body.template operator()<I>();})
		{
			[&]<auto ... Is>(constexpr_sequence<Is...>)
			{
				(loop_body.template operator()<Is>(), ...);
			}(sequence);
		}

		else if constexpr(requires{loop_body();})
		{
			[&]<auto ... Is>(constexpr_sequence<Is...>)
			{
				[[maybe_unused]] int arr[] = {((void) loop_body(), (void) Is, 1)...};
			}(sequence);
		}
	}
};

} // end of namespace metautils::details

namespace metautils
{

template <auto I, auto condition, auto modifier>
constexpr auto constexpr_for(auto&& loop_body) noexcept
{
return details::constexpr_for_impl<decltype(loop_body), I, condition, modifier>{std::forward<decltype(loop_body)>(loop_body)};
};

template <auto start, auto end, auto increment = [](decltype(start) i){ return ++i; }>
constexpr auto constexpr_for(auto loop_body)
{
	if constexpr (start <= end)
	{
		constexpr auto condition = [](auto e) { return e < end; };

		return constexpr_for_impl<decltype(loop_body), start, condition, increment>{std::forward<decltype(loop_body)>(loop_body)};
	}

	else
	{
		constexpr auto condition = [](auto e) { return e > end; };
		constexpr auto modifier = [](decltype(start) i){ return --i; };

		return constexpr_for_impl<decltype(loop_body), start, condition, modifier>{std::forward<decltype(loop_body)>(loop_body)};
	}
};

} // end of namespace metautils
