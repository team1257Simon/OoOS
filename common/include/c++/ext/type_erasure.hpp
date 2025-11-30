#ifndef __TYPE_ERASURE
#define __TYPE_ERASURE
#include <typeindex>
namespace std
{
	extension namespace ext
	{
		/**
		 * std::ext::type_erasure
		 * An extension of the basic C++ type_info / type_index structures that allows passing generic polymorphic pointers through type-unaware intermediates.
		 * When using type erasure to manage a polymorphic object, the actual type need only be known to the object's owner and, potentially, to its end-users.
		 * This differs from the analogous concept in Java, where all objects are owned by the JVM, and the underlying type might never see the light of day at all.
		 * Because C++ lacks true reflection, the end-user must also be aware of at least one (potentially virtual) base of the object at compile time.
		 * A combination of pointer and type erasure can essentially be used to implement a dynamic_cast equivalent on a void pointer.
		 */
		class type_erasure : public type_index
		{
		public:
			type_erasure(type_info const& i);
			constexpr type_erasure() noexcept : type_index() {}
			constexpr type_info const& get_type_info() const { return *info; }
			bool is_derived_from(type_info const& that);
			void* cast_to(void* obj, type_info const& ti) const;
			void* cast_from(void* obj, type_erasure const& that) const;
			template<typename T> T* cast_to(void* obj) { return static_cast<T*>(cast_to(obj, typeid(T))); }
			template<typename T> T const* cast_to(const void* obj) { return static_cast<T const*>(cast_to(const_cast<void*>(obj), typeid(T))); }
			template<typename T> void* cast(T& t);
			template<object T> requires (!is_array_v<T>) void* cast(T& t) { return cast_from(addressof(t), typeid(t)); }
			template<typename T> requires (!object<T> || is_array_v<T>) void* cast(T& t) { if(typeid(T) == *info) { return addressof(t); } return nullptr; }
			template<typename T> bool matches(T const& t) { if(*info == typeid(t)) return true; return type_erasure(typeid(t)).is_derived_from(*info); }
			friend constexpr bool operator==(type_erasure const& __this, type_erasure const& __that) { return __this.info == __that.info; }
		};
		template<typename T> type_erasure get_erasure() { return type_erasure(typeid(T)); }
	}
}
#endif