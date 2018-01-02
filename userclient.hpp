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
	IOReturn map_struct_arguments(IOMemoryMap*& in_map, IOMemoryMap*& out_map, IOExternalMethodArguments* arguments)
	{
				if (arguments->structureInputSize == 0 && arguments->structureInputDescriptor != nullptr)
				{
					in_map = arguments->structureInputDescriptor->createMappingInTask(kernel_task, 0, kIOMapAnywhere | kIOMapReadOnly);
					if (in_map != nullptr)
					{
						arguments->structureInputSize = static_cast<uint32_t>(in_map->getLength());
						arguments->structureInput = reinterpret_cast<const void*>(in_map->getAddress());
						kprintf("mapped struct input at %p, %u\n", arguments->structureInput, arguments->structureInputSize);
					}
					else
					{
						kprintf("mapping struct input failed\n");
					}
				}
				if (arguments->structureOutputSize == 0 && arguments->structureOutputDescriptor != nullptr)
				{
					out_map = arguments->structureOutputDescriptor->createMappingInTask(kernel_task, 0, kIOMapAnywhere);
					if (out_map != nullptr)
					{
						arguments->structureOutputSize = static_cast<uint32_t>(out_map->getLength());
						arguments->structureOutput = reinterpret_cast<void*>(out_map->getAddress());
						kprintf("mapped struct output at %p, %u\n", arguments->structureOutput, arguments->structureOutputSize);
					}
					else
					{
						kprintf("mapping struct output failed");
					}
				}
		return kIOReturnSuccess;
	}


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
	
#if (__cplusplus >= 201103L) || __has_feature(cxx_variadic_templates)
	template<typename... Args> struct scalar_input_arg_count;

	// termination condition
	template<>
	struct scalar_input_arg_count<> {
			static const unsigned count = 0;
	};
	// ignore output scalar
	template<typename... Args>
	struct scalar_input_arg_count<uint64_t*, Args...> {
			static const int count = scalar_input_arg_count<Args...>::count;
	};
	// ignore variable sized input struct
	template<typename... Args>
	struct scalar_input_arg_count<const void*, size_t, Args...> {
			static const int count = scalar_input_arg_count<Args...>::count;
	};
	// ignore variable sized output struct
	template<typename... Args>
	struct scalar_input_arg_count<void*, size_t, Args...> {
			static const int count = scalar_input_arg_count<Args...>::count;
	};
	// ignore output struct
	template<typename T, typename... Args>
	struct scalar_input_arg_count<T*, Args...> {
			static const int count = scalar_input_arg_count<Args...>::count;
	};
	// ignore the async argument triple
	template<typename... Args>
	struct scalar_input_arg_count<mach_port_t, io_user_reference_t*, uint32_t, Args...> {
		static const int count = scalar_input_arg_count<Args...>::count;
	};
	// everything else must be a scalar input
	template<typename T, typename... Args>
	struct scalar_input_arg_count<T, Args...> {
			static const int count = 1 + scalar_input_arg_count<Args...>::count;
	};

	template<typename... Args> struct struct_input_arg_size;
	template<>
	struct struct_input_arg_size<> {
			static const unsigned size = 0;
	};
	template<typename... Args>
	struct struct_input_arg_size<const void*, size_t, Args...> {
		static_assert(struct_input_arg_size<Args...>::size == 0,"Only one struct input allowed.");
			static const int size = kIOUCVariableStructureSize;
	};
	template<typename T, typename... Args>
		struct struct_input_arg_size<const T*, Args...> {
			static_assert(struct_input_arg_size<Args...>::size == 0,"Only one struct input allowed.");
			static const int size = sizeof(T);
			static_assert(size != 0,"zero-sized input structs not allowed");
		};
	template<typename T, typename... Args>
	struct struct_input_arg_size<T, Args...> {
			static const int size = struct_input_arg_size<Args...>::size;
	};
	
	template<typename... Args> struct struct_output_arg_size;
	template<>
	struct struct_output_arg_size<> {
			static const unsigned size = 0;
	};
	template<typename... Args>
		struct struct_output_arg_size<const void*, size_t, Args...> {
			static const int size = struct_output_arg_size<Args...>::size;
		};
	template<typename T, typename... Args>
		struct struct_output_arg_size<const T*, Args...> {
			static const int size = struct_output_arg_size<Args...>::size;
		};
	// ignore the async argument triple
	template<typename... Args>
		struct struct_output_arg_size<mach_port_t, io_user_reference_t*, uint32_t, Args...> {
			static const int size = struct_output_arg_size<Args...>::size;
		};

	template<typename... Args>
	struct struct_output_arg_size<void*, size_t, Args...> {
		static_assert(struct_output_arg_size<Args...>::size == 0,"Only one struct output allowed.");
			static const int size = kIOUCVariableStructureSize;
	};
	template<typename T, typename... Args>
		struct struct_output_arg_size<T*, Args...> {
			static_assert(struct_output_arg_size<Args...>::size == 0,"Only one struct output allowed.");
			static const int size = sizeof(T);
			static_assert(size != 0, "No zero-sized output struct types");
		};
	template<typename... Args>
	struct struct_output_arg_size<uint64_t*, Args...> {
			static const int size = struct_output_arg_size<Args...>::size;
	};
	template<typename T, typename... Args>
	struct struct_output_arg_size<T, Args...> {
			static const int size = struct_output_arg_size<Args...>::size;
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
	// ignore the async argument triple
	template<typename... Args>
	struct scalar_output_arg_count<mach_port_t, io_user_reference_t*, uint32_t, Args...> {
			static const int count = scalar_output_arg_count<Args...>::count;
	};
	
	template<typename... Args> struct is_async_method;

	// termination condition for non-async methods
	template<> struct is_async_method<> {
		static const bool value = false;
	};
	
	template<typename... Args> struct is_async_method<mach_port_t, io_user_reference_t*, uint32_t, Args...> {
		static const bool value = true;
	};
	template<typename T, typename... Args> struct is_async_method<T, Args...> {
		static const bool value = is_async_method<Args...>::value;
	};
#endif

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
		static typename ARGSEL::type get_element(const IOExternalMethodArguments* arguments)
	{
		return static_cast<typename ARGSEL::type>(ARGSEL::extract_argument(arguments));
	}

	template <typename T, int I>
		struct arg_sel_scalar_input
	{
		typedef T type;
		static const int index = I;
		static uint64_t extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->scalarInput[I];
		}
	};
	
	template <int I>
		struct arg_sel_scalar_output
	{
		typedef uint64_t* type;
		static const int index = I;
		static uint64_t* extract_argument(const IOExternalMethodArguments* arguments)
		{
			return &arguments->scalarOutput[I];
		}
	};
	
	struct arg_sel_variable_struct_input_ptr
	{
		typedef const void* type;
		static const void* extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->structureInput;
		}
	};
	template <typename T>
		struct arg_sel_fixed_struct_input_ptr
		{
			typedef const T* type;
			static const void* extract_argument(const IOExternalMethodArguments* arguments)
			{
				return arguments->structureInput;
			}
		};
	
	struct arg_sel_variable_struct_input_size
	{
		typedef size_t type;
		static size_t extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->structureInputSize;
		}
	};
	template <typename T>
		struct arg_sel_fixed_struct_output_ptr
	{
		typedef T* type;
		static_assert(sizeof(T) > 0,"");
		static void* extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->structureOutput;
		}
	};
	
	struct arg_sel_variable_struct_output_ptr
	{
		typedef void* type;
		static void* extract_argument(const IOExternalMethodArguments* arguments)
		{
			kprintf("arg_sel_variable_struct_output_ptr: %p\n", arguments->structureOutput);
			return arguments->structureOutput;
		}
	};
	struct arg_sel_variable_struct_output_size
	{
		typedef size_t type;
		static size_t extract_argument(const IOExternalMethodArguments* arguments)
		{
			kprintf("arg_sel_variable_struct_output_size: %u\n", arguments->structureOutputSize);
			return arguments->structureOutputSize;
		}
	};
	
	struct arg_sel_async_mach_port
	{
		typedef mach_port_t type;
		static mach_port_t extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->asyncWakePort;
		}
	};
	struct arg_sel_async_ref
	{
		typedef io_user_reference_t* type;
		static io_user_reference_t* extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->asyncReference;
		}
	};
	struct arg_sel_async_ref_count
	{
		typedef uint32_t type;
		static uint32_t extract_argument(const IOExternalMethodArguments* arguments)
		{
			return arguments->asyncReferenceCount;
		}
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

#if __cplusplus >= 201103L || __has_feature(cxx_variadic_templates)
	// dummy type that we only use for its type pack. (which will be a sequence of arg_sel types)
	template <typename... ARGSELS> struct arg_seq
	{};
	
	// Recursively invoked type; result will be in the 'seq' member type.
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARG_SEQ, typename... REMAIN_ARGS>
		struct arg_seq_accum;
	
	// The recurrence relation; current argument has type T and index NEXT_INPUT
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS, typename T, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, T, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT + 1, // increment index
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_scalar_input<T, NEXT_INPUT> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for fixed-sized struct inputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename T, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, const T*, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_fixed_struct_input_ptr<T> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'const T*' from parameters
			>::seq seq;
	};

	// The recurrence relation for variable-sized struct inputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, const void*, size_t, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_variable_struct_input_ptr, arg_sel_variable_struct_input_size>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for variable-sized struct outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, void*, size_t, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_variable_struct_output_ptr, arg_sel_variable_struct_output_size>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for the async triple
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, mach_port_t, io_user_reference_t*, uint32_t, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_async_mach_port, arg_sel_async_ref, arg_sel_async_ref_count>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop the triple from parameters
			>::seq seq;
	};

	// The recurrence relation for scalar outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, uint64_t*, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT + 1, // increment index
			arg_seq<ARGSELS..., arg_sel_scalar_output<NEXT_OUTPUT> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for fixed-sized struct outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename T, typename... ARGSELS, typename... REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...>, T*, REMAIN_ARGS...>
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			arg_seq<ARGSELS..., arg_sel_fixed_struct_output_ptr<T> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS... // Drop 'T' from parameters
			>::seq seq;
	};

	// Termination condition
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename... ARGSELS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, arg_seq<ARGSELS...> /* note: no remaining args */>
	{
		typedef arg_seq<ARGSELS...> seq; // our arg_sel sequence is complete, wrapped in a type
	};
	
	// This is really just a convenience for starting the accumulator off with the right initial conditions
	template <typename... ARGS> struct gen_arg_seq
	{
		// start with an empty arg_sel sequence, index 0, and all parameters remaining
		typedef typename arg_seq_accum<0, 0, arg_seq<>, ARGS...>::seq type;
	};
	
	template <typename MethodPointerSignature, MethodPointerSignature METHOD> struct userclient_method;
	template <class UCC, typename... Args, IOReturn(UCC::*METHOD)(Args...)>
	struct userclient_method<IOReturn(UCC::*)(Args...), METHOD>
	{
		static constexpr const unsigned inputs = scalar_input_arg_count<Args...>::count;
		static constexpr const unsigned outputs = scalar_output_arg_count<Args...>::count;
		static constexpr const uint32_t struct_input_size = struct_input_arg_size<Args...>::size;
		static constexpr const uint32_t struct_output_size = struct_output_arg_size<Args...>::size;
		static constexpr const bool is_async = is_async_method<Args...>::value;
		
		template <typename... ARGSELS>
			static IOReturn apply_fn(UCC* target, IOExternalMethodArguments* arguments, arg_seq<ARGSELS...>)
			{
				IOMemoryMap* in_map = nullptr;
				IOMemoryMap* out_map = nullptr;
				IOReturn result = map_struct_arguments(in_map, out_map, arguments);
				if (result != kIOReturnSuccess)
					return result;
				result = (target->*METHOD)(get_element<ARGSELS>(arguments) ...);
				OSSafeReleaseNULL(in_map);
				OSSafeReleaseNULL(out_map);
				return result;
			}
	
		static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments)
		{
			UCC* uc = OSDynamicCast(UCC, target);
			if (!uc)
				return kIOReturnBadArgument;
			if (is_async_method<Args...>::value && (arguments->asyncWakePort == MACH_PORT_NULL || arguments->asyncReference == nullptr || arguments->asyncReferenceCount == 0))
				return kIOReturnBadArgument;
			return apply_fn(uc, arguments, typename gen_arg_seq<Args...>::type());
		}
		
#if __cplusplus >= 201103L
		constexpr static const IOExternalMethodDispatch dispatch = {
			external_method,
			inputs,
			struct_input_size,
			outputs,
			struct_output_size
		};
#else
		template <IOExternalMethodAction action, uint32_t num_inputs, uint32_t num_outputs, uint32_t input_size, uint32_t output_size>
			struct IOExternalMethodDispatchGenerator
		{
			constexpr operator IOExternalMethodDispatch() const
			{
				return (IOExternalMethodDispatch){ action, num_inputs, input_size, num_outputs, output_size };
			}
		};

		static const IOExternalMethodDispatchGenerator<external_method, inputs, outputs, struct_input_size, struct_output_size> dispatch;
#endif
	};
#else
	template <typename MethodPointerSignature, MethodPointerSignature METHOD> struct userclient_method;
	
	struct EmptyTypeList;
	template <typename T, typename REST>
		struct TypeListItem
	{
		typedef T first;
		typedef REST rest;
	};
	
	template <typename ArgumentTypeList> struct scalar_input_arg_count;
	// termination condition
	template <> struct scalar_input_arg_count<EmptyTypeList>
	{
		static const unsigned count = 0;
	};
	// ignore output scalar
	template<typename REST>
		struct scalar_input_arg_count<TypeListItem<uint64_t*, REST> > {
			static const int count = scalar_input_arg_count<REST>::count;
		};
	// ignore variable sized input struct
	template<typename REST>
		struct scalar_input_arg_count<TypeListItem<const void*, TypeListItem<size_t, REST> > > {
			static const int count = scalar_input_arg_count<REST>::count;
		};
	// ignore variable sized output struct
	template<typename REST>
		struct scalar_input_arg_count<TypeListItem<void*, TypeListItem<size_t, REST > > > {
			static const int count = scalar_input_arg_count<REST>::count;
		};
	// ignore output struct
	template<typename STRUCT_T, typename REST>
	struct scalar_input_arg_count<TypeListItem<STRUCT_T*, REST> > {
			static const int count = scalar_input_arg_count<REST>::count;
	};
	// ignore the async argument triple
	template<typename REST>
	struct scalar_input_arg_count<TypeListItem<mach_port_t, TypeListItem<io_user_reference_t*, TypeListItem<uint32_t, REST> > > > {
		static const int count = scalar_input_arg_count<REST>::count;
	};
	// everything else must be a scalar input
	template<typename T, typename REST>
	struct scalar_input_arg_count<TypeListItem<T, REST> > {
			static const int count = 1 + scalar_input_arg_count<REST>::count;
	};


	template <typename ArgumentTypeList> struct scalar_output_arg_count;
	template <> struct scalar_output_arg_count<EmptyTypeList>
	{
		static const unsigned count = 0;
	};

	template<typename REST>
		struct scalar_output_arg_count<TypeListItem<uint64_t*, REST> > {
			static const int count = 1 + scalar_output_arg_count<REST>::count;
	};
	template<typename T, typename REST>
		struct scalar_output_arg_count<TypeListItem<T, REST> > {
			static const int count = scalar_output_arg_count<REST>::count;
	};
	// ignore the async argument triple
	template<typename REST>
		struct scalar_output_arg_count<TypeListItem<mach_port_t, TypeListItem<io_user_reference_t*, TypeListItem<uint32_t, REST> > > > {
			static const int count = scalar_output_arg_count<REST>::count;
	};

	template<typename REST> struct is_async_method;

	// termination condition for non-async methods
	template<> struct is_async_method<EmptyTypeList> {
		static const bool value = false;
	};
	
	template<typename REST> struct is_async_method<TypeListItem<mach_port_t, TypeListItem<io_user_reference_t*, TypeListItem<uint32_t, REST> > > > {
		static const bool value = true;
	};
	template<typename T, typename REST> struct is_async_method<TypeListItem<T, REST> > {
		static const bool value = is_async_method<REST>::value;
	};


	template<typename ArgumentTypeList> struct struct_input_arg_size;
	template<>
	struct struct_input_arg_size<EmptyTypeList> {
			static const unsigned size = 0;
	};
	template<typename RestTypeList>
	struct struct_input_arg_size<TypeListItem<const void*, TypeListItem<size_t, RestTypeList> > > {
		static_assert(struct_input_arg_size<RestTypeList>::size == 0,"Only one struct input allowed.");
			static const int size = kIOUCVariableStructureSize;
	};
	template<typename T, typename RestTypeList>
		struct struct_input_arg_size<TypeListItem<const T*, RestTypeList> > {
			static_assert(struct_input_arg_size<RestTypeList>::size == 0,"Only one struct input allowed.");
			static const int size = sizeof(T);
			static_assert(size != 0,"zero-sized input structs not allowed");
		};
	template<typename T, typename RestTypeList>
	struct struct_input_arg_size<TypeListItem<T, RestTypeList> > {
			static const int size = struct_input_arg_size<RestTypeList>::size;
	};



	template<typename ArgTypeList> struct struct_output_arg_size;
	template<>
	struct struct_output_arg_size<EmptyTypeList> {
			static const unsigned size = 0;
	};
	template<typename RestArgTypeList>
		struct struct_output_arg_size<TypeListItem<const void*, TypeListItem<size_t, RestArgTypeList> > > {
			static const int size = struct_output_arg_size<RestArgTypeList>::size;
		};
	template<typename T, typename RestArgTypeList>
		struct struct_output_arg_size<TypeListItem<const T*, RestArgTypeList> > {
			static const int size = struct_output_arg_size<RestArgTypeList>::size;
		};
	// ignore the async argument triple
	template<typename RestArgTypeList>
		struct struct_output_arg_size<TypeListItem<mach_port_t, TypeListItem<io_user_reference_t*, TypeListItem<uint32_t, RestArgTypeList> > > > {
			static const int size = struct_output_arg_size<RestArgTypeList>::size;
		};

	template<typename RestArgTypeList>
	struct struct_output_arg_size<TypeListItem<void*, TypeListItem<size_t, RestArgTypeList> > > {
		static_assert(struct_output_arg_size<RestArgTypeList>::size == 0,"Only one struct output allowed.");
			static const int size = kIOUCVariableStructureSize;
	};
	template<typename T, typename RestArgTypeList>
		struct struct_output_arg_size<TypeListItem<T*, RestArgTypeList> > {
			static_assert(struct_output_arg_size<RestArgTypeList>::size == 0,"Only one struct output allowed.");
			static const int size = sizeof(T);
			static_assert(size != 0, "No zero-sized output struct types");
		};
	template<typename RestArgTypeList>
	struct struct_output_arg_size<TypeListItem<uint64_t*, RestArgTypeList> > {
			static const int size = struct_output_arg_size<RestArgTypeList>::size;
	};
	template<typename T, typename RestArgTypeList>
	struct struct_output_arg_size<TypeListItem<T, RestArgTypeList> > {
			static const int size = struct_output_arg_size<RestArgTypeList>::size;
	};

	
	// Recursively invoked type; result will be in the 'seq' member type.
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARG_SEQ, typename REMAIN_ARGLIST>
		struct arg_seq_accum;
	
	// The recurrence relation; current argument has type T and index NEXT_INPUT
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS, typename T, typename REMAIN_ARGLIST>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<T, REMAIN_ARGLIST> >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT + 1, // increment index
			NEXT_OUTPUT,
			TypeListItem<arg_sel_scalar_input<T, NEXT_INPUT>, ARGSELS>, // create a new arg_sel and prepend it to the existing ones
			REMAIN_ARGLIST // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for fixed-sized struct inputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename T, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<const T*, REMAIN_ARGS> >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			TypeListItem<arg_sel_fixed_struct_input_ptr<T>, ARGSELS>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop 'const T*' from parameters
			>::seq seq;
	};

	// The recurrence relation for variable-sized struct inputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<const void*, TypeListItem<size_t, REMAIN_ARGS> > >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			TypeListItem<arg_sel_variable_struct_input_size, TypeListItem<arg_sel_variable_struct_input_ptr, ARGSELS> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for variable-sized struct outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<void*, TypeListItem<size_t, REMAIN_ARGS> > >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			TypeListItem<arg_sel_variable_struct_output_size, TypeListItem<arg_sel_variable_struct_output_ptr, ARGSELS> >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for the async triple
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<mach_port_t, TypeListItem<io_user_reference_t*, TypeListItem<uint32_t, REMAIN_ARGS> > > >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			TypeListItem<arg_sel_async_ref_count, TypeListItem<arg_sel_async_ref, TypeListItem<arg_sel_async_mach_port, ARGSELS> > >, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop the triple from parameters
			>::seq seq;
	};

	// The recurrence relation for scalar outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<uint64_t*, REMAIN_ARGS> >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT + 1, // increment index
			TypeListItem<arg_sel_scalar_output<NEXT_OUTPUT>, ARGSELS>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop 'T' from parameters
			>::seq seq;
	};

	// The recurrence relation for fixed-sized struct outputs
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename T, typename ARGSELS, typename REMAIN_ARGS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, TypeListItem<T*, REMAIN_ARGS> >
	{
		typedef typename arg_seq_accum<
			NEXT_INPUT,
			NEXT_OUTPUT,
			TypeListItem<arg_sel_fixed_struct_output_ptr<T>, ARGSELS>, // create a new arg_sel and append it to the existing ones
			REMAIN_ARGS // Drop 'T' from parameters
			>::seq seq;
	};

	// Termination condition
	template <int NEXT_INPUT, int NEXT_OUTPUT, typename ARGSELS>
		struct arg_seq_accum<NEXT_INPUT, NEXT_OUTPUT, ARGSELS, EmptyTypeList>
	{
		typedef ARGSELS seq;
	};
	
	template <typename TYPELIST, typename REVERSED = EmptyTypeList> struct reverse_typelist;
	template <typename REVERSED> struct reverse_typelist<EmptyTypeList, REVERSED> 
	{
		typedef REVERSED type;
	};
	template <typename T, typename REST, typename REVERSED> struct reverse_typelist<TypeListItem<T, REST>, REVERSED>
	{
		typedef typename reverse_typelist<REST, TypeListItem<T, REVERSED> >::type type;
	};
	
	// This is really just a convenience for starting the accumulator off with the right initial conditions
	template <typename ARGS> struct gen_arg_seq
	{
		// start with an empty arg_sel sequence, index 0, and all parameters remaining
		typedef typename reverse_typelist<typename arg_seq_accum<0, 0, EmptyTypeList, ARGS>::seq>::type type;
	};
	
	template <typename TYPE_LIST, unsigned index> struct TypeListItemIndex;
	
	template <typename T, typename REST_TYPE_LIST> struct TypeListItemIndex<TypeListItem<T, REST_TYPE_LIST>, 0>
	{
		typedef T type;
	};
	template <typename T, typename REST, unsigned index> struct TypeListItemIndex<TypeListItem<T, REST>, index>
	{
		typedef typename TypeListItemIndex<REST, index - 1>::type type;
	};
		
	template <class UCC, IOReturn(UCC::*METHOD)()>
		struct userclient_method<IOReturn(UCC::*)(), METHOD>
	{
		typedef EmptyTypeList argument_types;
		typedef typename gen_arg_seq<argument_types>::type gen_args;
		static const unsigned inputs = scalar_input_arg_count<argument_types>::count;
		static const unsigned outputs = scalar_output_arg_count<argument_types>::count;
		static const uint32_t struct_input_size = struct_input_arg_size<argument_types>::size;
		static const uint32_t struct_output_size = struct_output_arg_size<argument_types>::size;
		static const bool is_async = is_async_method<argument_types>::value;
		
			static IOReturn apply_fn(UCC* target, IOExternalMethodArguments* arguments)
			{
				IOMemoryMap* in_map = nullptr;
				IOMemoryMap* out_map = nullptr;
				IOReturn result = map_struct_arguments(in_map, out_map, arguments);
				if (result != kIOReturnSuccess)
					return result;
				result = (target->*METHOD)();
				OSSafeReleaseNULL(in_map);
				OSSafeReleaseNULL(out_map);
				return result;
			}
		
		static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments)
		{
			UCC* uc = OSDynamicCast(UCC, target);
			if (!uc)
				return kIOReturnBadArgument;
			if (is_async_method<argument_types>::value && (arguments->asyncWakePort == MACH_PORT_NULL || arguments->asyncReference == nullptr || arguments->asyncReferenceCount == 0))
				return kIOReturnBadArgument;
			return apply_fn(uc, arguments);
		}

		
		static IOExternalMethodDispatch get_dispatch()
		{
			return (IOExternalMethodDispatch){ external_method, inputs, 0, outputs, 0 };
		}
	};
	template <class UCC, typename AT1, IOReturn(UCC::*METHOD)(AT1)>
		struct userclient_method<IOReturn(UCC::*)(AT1), METHOD>
	{
		typedef TypeListItem<AT1, EmptyTypeList> argument_types;
		typedef typename gen_arg_seq<argument_types>::type gen_args;
		static const unsigned inputs = scalar_input_arg_count<argument_types>::count;
		static const unsigned outputs = scalar_output_arg_count<argument_types>::count;
		static const uint32_t struct_input_size = struct_input_arg_size<argument_types>::size;
		static const uint32_t struct_output_size = struct_output_arg_size<argument_types>::size;
		static const bool is_async = is_async_method<argument_types>::value;
		
			static IOReturn apply_fn(UCC* target, IOExternalMethodArguments* arguments)
			{
				IOMemoryMap* in_map = nullptr;
				IOMemoryMap* out_map = nullptr;
				IOReturn result = map_struct_arguments(in_map, out_map, arguments);
				if (result != kIOReturnSuccess)
					return result;
				result = (target->*METHOD)(get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments));
				OSSafeReleaseNULL(in_map);
				OSSafeReleaseNULL(out_map);
				return result;
			}
		
		static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments)
		{
			UCC* uc = OSDynamicCast(UCC, target);
			if (!uc)
				return kIOReturnBadArgument;
			if (is_async_method<argument_types>::value && (arguments->asyncWakePort == MACH_PORT_NULL || arguments->asyncReference == nullptr || arguments->asyncReferenceCount == 0))
				return kIOReturnBadArgument;
			return apply_fn(uc, arguments);
		}

		
		static IOExternalMethodDispatch get_dispatch()
		{
			return (IOExternalMethodDispatch){ external_method, inputs, struct_input_size, outputs, struct_output_size };
		}
	};
	
	#define UC_METHOD_PROPERTY_DEFS \
		typedef typename gen_arg_seq<argument_types>::type gen_args; \
		static const unsigned inputs = scalar_input_arg_count<argument_types>::count; \
		static const unsigned outputs = scalar_output_arg_count<argument_types>::count; \
		static const uint32_t struct_input_size = struct_input_arg_size<argument_types>::size; \
		static const uint32_t struct_output_size = struct_output_arg_size<argument_types>::size; \
		static const bool is_async = is_async_method<argument_types>::value;

	#define UC_METHOD_APPLY_DEF(...) \
			static IOReturn apply_fn(UCC* target, IOExternalMethodArguments* arguments) \
			{ \
				IOMemoryMap* in_map = nullptr; \
				IOMemoryMap* out_map = nullptr; \
				IOReturn result = map_struct_arguments(in_map, out_map, arguments); \
				if (result != kIOReturnSuccess) \
					return result; \
				result = (target->*METHOD)(__VA_ARGS__); \
				OSSafeReleaseNULL(in_map); \
				OSSafeReleaseNULL(out_map); \
				return result; \
			} \
		static IOReturn external_method(OSObject* target, void* reference, IOExternalMethodArguments* arguments) \
		{ \
			UCC* uc = OSDynamicCast(UCC, target); \
			if (!uc) \
				return kIOReturnBadArgument; \
			if (is_async_method<argument_types>::value && (arguments->asyncWakePort == MACH_PORT_NULL || arguments->asyncReference == nullptr || arguments->asyncReferenceCount == 0)) \
				return kIOReturnBadArgument; \
			return apply_fn(uc, arguments); \
		} \
		static IOExternalMethodDispatch get_dispatch() \
		{ \
			return (IOExternalMethodDispatch){ external_method, inputs, struct_input_size, outputs, struct_output_size }; \
		}

	template <
			class UCC,
			typename AT1,
			typename AT2,
			IOReturn(UCC::*METHOD)(AT1, AT2)>
		struct userclient_method<IOReturn(UCC::*)(AT1, AT2), METHOD>
	{
		typedef TypeListItem<AT1, TypeListItem<AT2, EmptyTypeList> > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments))
		
	};

	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			IOReturn(UCC::*METHOD)(AT1, AT2, AT3)>
		struct userclient_method<IOReturn(UCC::*)(AT1, AT2, AT3), METHOD>
	{
		typedef TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, EmptyTypeList> > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments)
			)
		
	};

	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			typename AT4,
			IOReturn(UCC::*METHOD)(
				AT1, AT2, AT3, AT4
				)>
		struct userclient_method<IOReturn(UCC::*)(
			AT1, AT2, AT3, AT4
			), METHOD>
	{
		typedef
			TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, TypeListItem<AT4, EmptyTypeList> > > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 3>::type>(arguments)
			)		
	};

	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			typename AT4,
			typename AT5,
			IOReturn(UCC::*METHOD)(
				AT1, AT2, AT3, AT4, AT5
				)>
		struct userclient_method<IOReturn(UCC::*)(
			AT1, AT2, AT3, AT4, AT5
			), METHOD>
	{
		typedef
			TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, TypeListItem<AT4, TypeListItem<AT5, EmptyTypeList> > > > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 3>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 4>::type>(arguments)
			)		
	};



	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			typename AT4,
			typename AT5,
			typename AT6,
			IOReturn(UCC::*METHOD)(
				AT1, AT2, AT3, AT4, AT5, AT6
				)>
		struct userclient_method<IOReturn(UCC::*)(
			AT1, AT2, AT3, AT4, AT5, AT6
			), METHOD>
	{
		typedef
			TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, TypeListItem<AT4, TypeListItem<AT5, TypeListItem<AT6, EmptyTypeList> > > > > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 3>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 4>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 5>::type>(arguments)
			)		
	};


	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			typename AT4,
			typename AT5,
			typename AT6,
			typename AT7,
			IOReturn(UCC::*METHOD)(
				AT1, AT2, AT3, AT4, AT5, AT6, AT7
				)>
		struct userclient_method<IOReturn(UCC::*)(
			AT1, AT2, AT3, AT4, AT5, AT6, AT7
			), METHOD>
	{
		typedef
			TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, TypeListItem<AT4, TypeListItem<AT5, TypeListItem<AT6, TypeListItem<AT7, EmptyTypeList> > > > > > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 3>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 4>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 5>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 6>::type>(arguments)
			)		
	};


	template <
			class UCC,
			typename AT1,
			typename AT2,
			typename AT3,
			typename AT4,
			typename AT5,
			typename AT6,
			typename AT7,
			typename AT8,
			IOReturn(UCC::*METHOD)(
				AT1, AT2, AT3, AT4, AT5, AT6, AT7, AT8
				)>
		struct userclient_method<IOReturn(UCC::*)(
			AT1, AT2, AT3, AT4, AT5, AT6, AT7, AT8
			), METHOD>
	{
		typedef
			TypeListItem<AT1, TypeListItem<AT2, TypeListItem<AT3, TypeListItem<AT4, TypeListItem<AT5, TypeListItem<AT6, TypeListItem<AT7, TypeListItem<AT8, EmptyTypeList> > > > > > > > argument_types;
		UC_METHOD_PROPERTY_DEFS
		UC_METHOD_APPLY_DEF(
			get_element<typename TypeListItemIndex<gen_args, 0>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 1>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 2>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 3>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 4>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 5>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 6>::type>(arguments),
			get_element<typename TypeListItemIndex<gen_args, 7>::type>(arguments)
			)		
	};


#endif
}

const void* dj_iouserclient_map_input_struct(
	IOExternalMethodArguments* args, IOMemoryMap*& out_map, uint32_t& out_size);

