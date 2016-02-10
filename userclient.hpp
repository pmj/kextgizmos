/* IOUserClient helpers.
 * Copyright 2013-2016 Phillip Dennis-Jordan. All rights reserved.*/

#pragma once

#include <IOKit/IOReturn.h>
#include <libkern/c++/OSObject.h>

class IOMemoryMap;

/* I don't like casting function pointers, it's fragile as accidental prototype
 * changes are not caught. External methods are expected to be of type
 * IOExternalMethodAction. So wrap each member function in this shim with the
 * correct type that forwards the arguments:
 * &br_external_method<&SomeServiceUserClient::startReceivingEvents>
 * is of type IOExternalMethodAction, even though
 * SomeServiceUserClient::startReceivingEvents isn't.
 *
 * Eventually, I'd like to use variadic templating to auto-generate the
 * wrapping/unwrapping?
 */
namespace
{
	template <class UCC> struct userclient_external_methods
	{
		template <IOReturn (UCC::*method)(void* reference, IOExternalMethodArguments* arguments)>
			static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments)
		{
			UCC* uc = OSDynamicCast(UCC, target);
			if (!uc)
				return kIOReturnBadArgument;
			return (uc->*method)(reference, arguments);
		}
	};
	
	template<typename... Args> struct scalar_input_arg_count;

	template<>
	struct scalar_input_arg_count<> {
			static const unsigned count = 0;
	};

	template<typename... Args>
	struct scalar_input_arg_count<uint64_t*, Args...> {
			static const int count = scalar_input_arg_count<Args...>::count;
	};
	template<typename T, typename... Args>
	struct scalar_input_arg_count<T, Args...> {
			static const int count = 1 + scalar_input_arg_count<Args...>::count;
	};

	template<typename... Args> struct scalar_output_arg_count;

	template<>
	struct scalar_output_arg_count<> {
			static const unsigned count = 0;
	};

	template<typename... Args>
	struct scalar_output_arg_count<uint64_t*, Args...> {
			static const int count = 1 + scalar_output_arg_count<Args...>::count;
	};
	template<typename T, typename... Args>
	struct scalar_output_arg_count<T, Args...> {
			static const int count = scalar_output_arg_count<Args...>::count;
	};
	

	/* We want the wrapped function call to expand to something like this:
	 * return (target->*METHOD)(
	 *   static_cast<int>(arguments->scalarInput[0]),
	 *   static_cast<enum blah>(arguments->scalarInput[1]));
   *
	 * Or something like that. Whatever matches the argument types of the method.
	 * C++ lets you iteratively apply a template function for each item in a
	 * type or value pack (typename... Typepack, or int... Valuepack).
	 * So given a PACK where each item encapsulates the necessary information for
	 * get_element<>(), our call can look like this:
	 * return (target->*METHOD)(
	 *   get_element<PACK>(arguments->scalarInput) ...);
	 *
	 * So let's define get_element such that its template argument is an argument
	 * selector type which encapsulates the argument type, and the index in the
	 * inputs array, which we'll call arg_sel:
	 */

	template <typename ARGSEL>
		static uint64_t get_element(const uint64_t* array)
	{
		return static_cast<typename ARGSEL::type>(array[ARGSEL::index]);
	}

	template <typename T, int I>
		struct arg_sel
	{
		typedef T type;
		static const int index = I;
	};
	
	/* That leaves us with the problem of generating a pack of arg_sel types that
	 * corresponds to our wrapped method's parameters.
	 *
	 * We define a template gen_arg_seq, parameterised only on the original
	 * parameter pack, which uses an accumulator type that recursively adds
	 * items to an arg_seq<> whose template argument is indeed a pack of arg_sel
	 * types.
	 * The accumulator keeps track the index of the next array element, as well
	 * as the remaining parameter types, with the completion condition of
	 * no remaining parameters.
	 */
	
	// dummy type that we only use for its type pack. (which will be a sequence of arg_sel types)
	template <typename... ARGSELS> struct arg_seq
	{};
	
	// Recursively invoked type; result will be in the 'seq' member type.
	template <int NEXT_INPUT, typename ARG_SEQ, typename... REMAIN_ARGS>
		struct arg_seq_accum;
	
	// The recurrence relation; current argument has type T and index NEXT_INPUT
	template <int NEXT_INPUT, typename... ARGSELS, typename T, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, arg_seq<ARGSELS...>, T, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT + 1, // increment index
			arg_seq<ARGSELS..., arg_sel<T, NEXT_INPUT> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// Termination condition
	template <int NEXT_INPUT, typename... ARGSELS>
		struct arg_seq_accum<NEXT_INPUT, arg_seq<ARGSELS...> /* note: no remaining args */>
	{
		typedef arg_seq<ARGSELS...> seq; // our arg_sel sequence is complete, wrapped in a type
	};
	
	// This is really just a convenience for starting the accumulator off with the right initial conditions
	template <typename... ARGS> struct gen_arg_seq
	{
		// start with an empty arg_sel sequence, index 0, and all parameters remaining
		typedef typename arg_seq_accum<0, arg_seq<>, ARGS...>::seq type;
	};
	
	template <typename MethodPointerSignature, MethodPointerSignature METHOD> struct userclient_method;
	template <class UCC, typename... Args, IOReturn(UCC::*METHOD)(Args...)>
	struct userclient_method<IOReturn(UCC::*)(Args...), METHOD>
	{
		static constexpr const unsigned inputs = scalar_input_arg_count<Args...>::count;
		static constexpr const unsigned outputs = scalar_output_arg_count<Args...>::count;
		
		template <typename... ARGSELS>
			static IOReturn apply_fn(UCC* target, IOExternalMethodArguments* arguments, arg_seq<ARGSELS...>)
			{
				return (target->*METHOD)(get_element<ARGSELS>(arguments->scalarInput) ...);
			}
	
		static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments)
		{
			UCC* uc = OSDynamicCast(UCC, target);
			if (!uc)
				return kIOReturnBadArgument;
			return apply_fn(uc, arguments, typename gen_arg_seq<Args...>::type());
		}
		
		static constexpr const IOExternalMethodDispatch dispatch = {
			.function = external_method,
			.checkScalarInputCount = inputs,
			.checkScalarOutputCount = outputs,
			.checkStructureInputSize = 0,
			.checkStructureOutputSize = 0
		};
	};
}

const void* dj_iouserclient_map_input_struct(
	IOExternalMethodArguments* args, IOMemoryMap*& out_map, uint32_t& out_size);

