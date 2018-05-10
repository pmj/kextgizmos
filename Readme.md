# Kextgizmos

This is a collection (library?) of functions, data structures and macros which
we have created, and which have repeatedly come in useful while developing
macOS/OS X kernel extensions (kexts).

These gizmos are dual-licensed under the permissive
[zLib](https://opensource.org/licenses/zLib) and
[MIT](https://opensource.org/licenses/MIT) Free Software
licenses. Please comply with the terms of one or the other when including the
code in derived works.

To use, just copy the relevant files into your kext project and add them to the
build system you're using. There are no dependencies beyond `Kernel.framework`.
Make sure to run `kextlibs` to check whether your kext's `OSBundleLibraries`
might need updating.

## The Gizmos

### `ioreturn_strings`

This defines a single function,

    const char* djt_ioreturn_string(IOReturn r);

which returns the string representation of an IOKit return code, which you can
log in place of the gobbledegook numeric value.

This gizmo is just as useful when used from user space code!

 * [`ioreturn_strings.h`](./ioreturn_strings.h)
 * [`ioreturn_strings.cpp`](./ioreturn_strings.cpp)

### `osobject_retaincount.h`

Use the `DJ_OSOBJECT_RETAINCOUNT()` macro within an `OSObject`-derived class
definition to get retain/release stack traces on that class. Helpful for tracking
down leaks.

 * [`osobject_retaincount.h`](./osobject_retaincount.h)

### `iopcidevice_helpers`

This is a collection of helper functions for working with `IOPCIDevice` objects.
It takes care of various boilerplate such as enumerating interrupts, including
MSI/MSI-X, walking the capabilities in the device configuration range (the
builtin `IOPCIDevice` methods fail silently on certain configuration layouts),
working with BARs, and logging the whole config area.

 * [`iopcidevice_helpers.hpp`](./iopcidevice_helpers.hpp)
 * [`iopcidevice_helpers.cpp`](./iopcidevice_helpers.cpp)

### `profiling`

Some very basic time measurement helpers for generating min/max/mean/stddev of
measured code runtimes in nanoseconds with little overhead (no locks) and a
drop-in `IOUserClient` external method for extracting the data from user space.

You must `#define` the `DJ_PROFILE_ENABLE` macro to switch this on (e.g. only
for certain builds) 

 * [`profiling.h`](./profiling.h)
 * [`profiling.cpp`](./profiling.cpp)

### `osdictionary_util`

`OSDictionary` objects can be awkward to deal with due to all the retaining,
releasing, casting, etc. The `log_dictionary_contents()` function helps you
debug problems by pretty-printing the contents of a dictionary.

## See also

 * [genccont, the Generic C container library](https://github.com/pmj/genccont/) - Another library which is useful for developing macOS kexts, but can also be used elsewhere.
