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
		 * When using type erasure to manage a polymorphic object, the actual type need only be known to the object's owner.
		 * This differs from the analogous concept in Java, where all objects are owned by the JVM, and the underlying type might never see the light of day at all.
		 * Because C++ lacks true reflection, the end-user must be aware of at least one base of the object at compile time if the object is not polymorphic.
		 * In order to actually use the object, of course, the desired type must be input by the end-user.
		 */
		class type_erasure : public type_index
		{
			template<typename T> struct __can_cast_ptr : false_type {};
			template<std::__detail::__dereferenceable T> struct __can_cast_ptr<T> : bool_constant<is_object_v<std::remove_cvref_t<decltype(*std::declval<T>())>>>{};
			template<satisfies<__can_cast_ptr> T> using __ptr_val_t = std::remove_cvref_t<std::iter_reference_t<T>>;
		public:
			constexpr type_erasure(type_info const& i) : type_index(i) {}
			constexpr type_erasure() noexcept : type_index() {}
			constexpr type_info const& get_type_info() const { return *info; }
			bool is_derived_from(type_info const& that) const noexcept;
			bool derives(type_info const& that) const noexcept;
			void* cast_to(void* obj, type_info const& ti) const noexcept;
			void* cast_from(void* obj, type_erasure const& that) const noexcept;
			void* cast(void* obj) const noexcept;
			template<typename T> requires(!is_pointer_v<T>) void* cast(T& t) const noexcept { return cast_from(addressof(t), typeid(t)); }
			template<satisfies<__can_cast_ptr> T> void* cast(T t) const noexcept { return cast_from(std::to_address(t), typeid(__ptr_val_t<T>)); }
			template<typename T> T* cast_to(void* obj) const noexcept { return static_cast<T*>(cast_to(obj, typeid(T))); }
			template<typename T> T const* cast_to(const void* obj) const noexcept { return static_cast<T const*>(cast_to(const_cast<void*>(obj), typeid(T))); }
			template<typename T> bool matches() const noexcept { return type_erasure(typeid(T)).is_derived_from(*info); }
			template<typename T> bool matches(T const& t) const noexcept { return type_erasure(typeid(t)).is_derived_from(*info); }
			friend constexpr bool operator==(type_erasure const& __this, type_erasure const& __that) noexcept { return __this.info == __that.info; }
			friend constexpr bool operator==(type_erasure const& __this, type_info const& __that) noexcept { return __this.info == std::addressof(__that); }
			friend constexpr bool operator==(type_info const& __this, type_erasure const& __that) noexcept { return std::addressof(__this) == __that.info; }
		};
		template<typename T> type_erasure get_erasure() { return type_erasure(typeid(T)); }
		/**
		 * If ptr is a pointer to an object whose dynamic most-derived type is U, then reflective_cast<T>(ptr) is equivalent to dynamic_cast<T*>(static_cast<U*>(dynamic_cast<void*>(ptr))).
		 * In other words, if ptr is a void pointer decayed from a pointer to B, and B is a base of U, then reflective_cast<T>(ptr) will return the same as dynamic_cast<T*>(dynamic_cast<U*>(static_cast<B*>(ptr))).
		 * Otherwise, it will return a null pointer.
		 * The type U need not be known to the caller, so this can be used to verify the inheritance of an unknown pointer from a given base.
		 * Note: the dynamic most-derived type is the type whose constructor was invoked when the pointee object was constructed.
		 * Because this only works with types that have virtual pointers, the type T must be polymorphic.
		 */
		template<typename T> requires(std::is_polymorphic_v<T>)
		T* reflective_cast(void* ptr) noexcept
		{
			type_erasure erasure	= type_erasure(typeid(T));
			void* result			= erasure.cast(ptr);
			return static_cast<T*>(result);
		}
		/**
		 * Attempts to extract the RTTI from the passed pointer.
		 * If successful, this function returns the typeid of the object's most-derived type.
		 * If passed a null pointer, it returns typeid(nullptr).
		 * On a failure (because the passed pointer is not polymorphic), it returns typeid(void).
		 */
		type_info const& extract_typeid(void*) noexcept;
	}
}
#endif