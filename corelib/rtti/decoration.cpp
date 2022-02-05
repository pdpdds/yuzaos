#include <string.h>
#include <typeinfo>
#include <winapi.h>

extern "C" void* _InterlockedCompareExchangePointer(
	void* volatile* Destination,
	void* Exchange,
	void* Comparand
);

extern "C" void _ReadWriteBarrier(void);

template <typename T, typename E, typename C>
T* __crt_interlocked_compare_exchange_pointer(T* const volatile* target, E const exchange,
	C const comparand) noexcept
{
	//UNREFERENCED_PARAMETER(exchange);  // These are required to silence spurious
	//UNREFERENCED_PARAMETER(comparand); // unreferenced formal parameter warnings.

	return reinterpret_cast<T*>(_InterlockedCompareExchangePointer(
		(void**)target, (void*)exchange, (void*)comparand));
}

struct __crt_internal_free_policy
{
	template <typename T>
	void operator()(T const* const p) const noexcept
	{
		free(const_cast<T*>(p));
	}
};


struct __crt_public_free_policy
{
	template <typename T>
	void operator()(T const* const p) const noexcept
	{
		free(const_cast<T*>(p));
	}
};

template <typename T, typename Free = __crt_internal_free_policy>
class __crt_unique_heap_ptr
{
public:

	explicit __crt_unique_heap_ptr(T* const p = nullptr) noexcept
		: _p(p)
	{
	}

	__crt_unique_heap_ptr(__crt_unique_heap_ptr const&) = delete;
	__crt_unique_heap_ptr& operator=(__crt_unique_heap_ptr const&) = delete;

	__crt_unique_heap_ptr(__crt_unique_heap_ptr&& other) noexcept
		: _p(other._p)
	{
		other._p = nullptr;
	}

	~__crt_unique_heap_ptr() noexcept
	{
		release();
	}

	__crt_unique_heap_ptr& operator=(__crt_unique_heap_ptr&& other) noexcept
	{
		release();
		_p = other._p;
		other._p = nullptr;
		return *this;
	}

	T* detach() noexcept
	{
		T* const local_p{ _p };
		_p = nullptr;
		return local_p;
	}

	void attach(T* const p) noexcept
	{
		release();
		_p = p;
	}

	void release() noexcept
	{
		Free()(_p);
		_p = nullptr;
	}

	bool is_valid() const noexcept
	{
		return _p != nullptr;
	}

	explicit operator bool() const noexcept
	{
		return is_valid();
	}

	T* get() const noexcept
	{
		return _p;
	}

	T** get_address_of() noexcept
	{
		return &_p;
	}

	T** release_and_get_address_of() noexcept
	{
		release();
		return &_p;
	}

private:
	T* _p;
};

#ifndef _M_CEE_PURE

#if defined _M_ARM
#define __crt_interlocked_memory_barrier() (__dmb(_ARM_BARRIER_ISH))
#elif defined _M_ARM64
#define __crt_interlocked_memory_barrier() (__dmb(_ARM64_BARRIER_ISH))
#endif

inline __int32 __crt_interlocked_read_32(__int32 const volatile* target) noexcept
{
#if defined _M_IX86 || defined _M_X64
	__int32 const result = *target;
	_ReadWriteBarrier();
	return result;
#elif defined _M_ARM || defined _M_ARM64
	__int32 const result = __iso_volatile_load32(reinterpret_cast<int const volatile*>(target));
	__crt_interlocked_memory_barrier();
	return result;
#else
#error Unsupported architecture
#endif
}

#if defined _WIN64
inline __int64 __crt_interlocked_read_64(__int64 const volatile* target) noexcept
{
#if defined _M_X64
	__int64 const result = *target;
	_ReadWriteBarrier();
	return result;
#elif defined _M_ARM64
	__int64 const result = __iso_volatile_load64(target);
	__crt_interlocked_memory_barrier();
	return result;
#else
#error Unsupported architecture
#endif
}
#endif // _WIN64

template <typename T>
T __crt_interlocked_read(T const volatile* target) noexcept
{
	static_assert(sizeof(T) == sizeof(__int32), "Type being read must be 32 bits in size.");
	return (T)__crt_interlocked_read_32((__int32*)target);
}


template <typename T>
T* __crt_interlocked_read_pointer(T* const volatile* target) noexcept
{
#ifdef _WIN64
	return (T*)__crt_interlocked_read_64((__int64*)target);
#else
	return (T*)__crt_interlocked_read_32((__int32*)target);
#endif
}

#endif // _M_CEE_PURE


typedef void* (__cdecl* malloc_func_t)(size_t);
typedef void(__cdecl* free_func_t)(void*);

#define UNDNAME_32_BIT_DECODE            (0x0800)
#define UNDNAME_TYPE_ONLY	0x2000

extern "C" char* __cdecl __unDName(char* buffer, const char* mangled, int buflen,
	malloc_func_t memget, free_func_t memfree,
	unsigned short int flags);

extern "C" char const* __cdecl __std_type_info_name(
	__std_type_info_data* const data,
	__type_info_node* const root_node
)
{
	// First check to see if we've already cached the undecorated name; if we
	// have, we can just return it:
	{
		char const* const cached_undecorated_name = __crt_interlocked_read_pointer(&data->_UndecoratedName);
		if (cached_undecorated_name)
		{
			return cached_undecorated_name;
		}
	}

	__crt_unique_heap_ptr<char> undecorated_name(__unDName(
		nullptr,
		data->_DecoratedName + 1,
		0,
		[](size_t const n) { return malloc(n); },
		[](void* const p) { return free(p);   },
		UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY));

	if (!undecorated_name)
	{
		return nullptr; // CRT_REFACTOR TODO This is nonconforming
	}

	size_t undecorated_name_length = strlen(undecorated_name.get());
	while (undecorated_name_length != 0 && undecorated_name.get()[undecorated_name_length - 1] == ' ')
	{
		undecorated_name.get()[undecorated_name_length - 1] = '\0';
		--undecorated_name_length;
	}

	size_t const undecorated_name_count = undecorated_name_length + 1;
	size_t const node_size = sizeof(SLIST_ENTRY) + undecorated_name_count;

	__crt_unique_heap_ptr<void> node_block(malloc(node_size));
	if (!node_block)
	{
		return nullptr; // CRT_REFACTOR TODO This is nonconforming
	}

	PSLIST_ENTRY const node_header = static_cast<PSLIST_ENTRY>(node_block.get());
	char* const node_string = reinterpret_cast<char*>(node_header + 1);

	*node_header = SLIST_ENTRY{};
	strcpy(node_string, undecorated_name.get());

	char const* const cached_undecorated_name = __crt_interlocked_compare_exchange_pointer(
		&data->_UndecoratedName,
		node_string,
		nullptr);

	// If the cache already contained an undecorated name pointer, another
	// thread must have cached it while we were computing the undecorated
	// name.  Discard the string we created and return the cached string:
	if (cached_undecorated_name)
	{
		return cached_undecorated_name;
	}

	// Otherwise, we've successfully cached our string; link it into the list
	// and return it:
	node_block.detach();
	InterlockedPushEntrySList(&root_node->_Header, node_header);
	return node_string;
}

extern "C" int __cdecl __std_type_info_compare(
	__std_type_info_data const* const lhs,
	__std_type_info_data const* const rhs
)
{
	if (lhs == rhs)
	{
		return 0;
	}

	return strcmp(lhs->_DecoratedName + 1, rhs->_DecoratedName + 1);
}

extern "C" size_t __cdecl __std_type_info_hash(
	__std_type_info_data const* const data
)
{
	// FNV-1a hash function for the undecorated name

#ifdef _WIN64
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	size_t const fnv_offset_basis = 14695981039346656037ULL;
	size_t const fnv_prime = 1099511628211ULL;
#else
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	size_t const fnv_offset_basis = 2166136261U;
	size_t const fnv_prime = 16777619U;
#endif

	size_t value = fnv_offset_basis;
	for (char const* it = data->_DecoratedName + 1; *it != '\0'; ++it)
	{
		value ^= static_cast<size_t>(static_cast<unsigned char>(*it));
		value *= fnv_prime;
	}

#ifdef _WIN64
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	value ^= value >> 32;
#else
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
#endif

	return value;
}
