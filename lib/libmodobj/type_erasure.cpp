#include <ext/type_erasure.hpp>
using namespace ABI_NAMESPACE;
using std::addressof;
static bool __is_primitive(std::type_info const& t) noexcept { return dynamic_cast<__fundamental_type_info const*>(addressof(t)); }
struct vtable_header
{
	// Offset of the leaf (full/primary) object.
	ptrdiff_t					leaf_offset;
	// Type of the object.
	const __class_type_info*	type;
};
static std::type_info const* __extract_type(void* obj) noexcept
{
	// null pointers have a special type, which has its own typeid
	if(__unlikely(!obj)) return std::addressof(typeid(nullptr));
	// treat obj as void**
	addr_t vpointer							= addr_t(obj).deref<addr_t>();
	// if obj does not point to something that is itself a pointer (such as a vtable pointer), we can't dereference this or we'll fault
	if(__unlikely(!vpointer.is_canonical())) return nullptr;
	// the object could be a non-polymorphic object that has a pointer as its first member; to be able to tell the difference, we must check if the pointee is actually a vtable, using some dynamic_cast magic
	vtable_header const& target_header		= vpointer.minus(sizeof(vtable_header)).deref<vtable_header const>();
	// similarly to before, we need to avoid passing an invalid pointer to dynamic_cast
	if(__unlikely(!addr_t(target_header.type).is_canonical())) return nullptr;
	// this is declared as volatile to indicate that we aren't sure it actually points at a valid type_info; otherwise, the dynamic_cast will be optimized out
	std::type_info const* volatile type_v	= target_header.type;
	// any polymorphic type will have a class_type_info pointer here; this dynamic cast will return a null pointer if that is not the case...
	__class_type_info const* volatile ctype	= dynamic_cast<__class_type_info const*>(type_v);
	// at which point we can't extract the type anymore
	if(!ctype) return nullptr;
	// if the type we have is valid, and the leaf offset is zero, we've found the type
	else if(!target_header.leaf_offset) return ctype;
	// otherwise, there's a more-derived class to find; we already know the polymorphic pointers are valid, so we don't need to check again
	else return addr_t(obj).plus(target_header.leaf_offset).deref<addr_t>().minus(sizeof(vtable_header)).deref<vtable_header const>().type;
}
static bool __is_derived_from(__class_type_info const* __type, __class_type_info const* __base) noexcept
{
	if(__type == __base || __type->__type_name == __base->__type_name) return true;
	if(__type && __base)
	{
		if(__si_class_type_info const* sti = dynamic_cast<__si_class_type_info const*>(__type))
			return __is_derived_from(sti->__base_type, __base);
		else if(__vmi_class_type_info const* vti = dynamic_cast<__vmi_class_type_info const*>(__type))
		{
			for(unsigned int i = 0U; i < vti->__base_count; i++)
				if(__is_derived_from(vti->__base_info[i].__base_type, __base))
					return true;
		}
	}
	return false;
}
static void* __reflective_cast(std::type_info const& from, std::type_info const& to, void* obj) noexcept
{
	if(__unlikely(!obj)) return nullptr;
	if(from == to || (__is_primitive(from) && __is_primitive(to))) return obj;
	__class_type_info const* cfrom	= dynamic_cast<__class_type_info const*>(addressof(from));
	__class_type_info const* cto	= dynamic_cast<__class_type_info const*>(addressof(to));
	if(cfrom && cto)
	{
		void* cobj					= obj;
		if(cfrom->__do_upcast(cto, addressof(cobj))) return cobj;
		return nullptr;
	}
	__pointer_type_info const* pfrom	= dynamic_cast<__pointer_type_info const*>(addressof(from));
	__pointer_type_info const* pto		= dynamic_cast<__pointer_type_info const*>(addressof(to));
	if(pfrom && pto) return __reflective_cast(*pfrom->__pointee, *pto->__pointee, *static_cast<void**>(obj));
	return nullptr;
}
namespace std
{
	namespace ext
	{
		void* type_erasure::cast_to(void* obj, type_info const& ti) const noexcept { return __reflective_cast(*info, ti, obj); }
		void* type_erasure::cast_from(void* obj, type_erasure const& that) const noexcept { return that.cast_to(obj, *info); }
		bool type_erasure::is_derived_from(type_info const& that) const noexcept
		{
			if(info == addressof(that)) return true;
			__class_type_info const* cthis	= dynamic_cast<__class_type_info const*>(info);
			__class_type_info const* cthat	= dynamic_cast<__class_type_info const*>(addressof(that));
			if(cthis && cthat) return __is_derived_from(cthis, cthat);
			return false;
		}
		bool type_erasure::derives(type_info const& that) const noexcept
		{
			if(info == addressof(that)) return true;
			__class_type_info const* cthis	= dynamic_cast<__class_type_info const*>(info);
			__class_type_info const* cthat	= dynamic_cast<__class_type_info const*>(addressof(that));
			if(cthis && cthat) return __is_derived_from(cthat, cthis);
			return false;
		}
		void* type_erasure::cast(void* obj) const noexcept
		{
			if(__unlikely(!obj)) return nullptr;
			type_info const* inferred_type	= __extract_type(obj);
			if(__unlikely(!inferred_type)) return nullptr;
			addr_t obj_addr(obj);
			return __reflective_cast(*inferred_type, *info, obj_addr.plus(obj_addr.deref<addr_t>().minus(sizeof(vtable_header)).deref<vtable_header const>().leaf_offset));
		}
		type_info const& extract_typeid(void* obj) noexcept
		{
			type_info const* poly	= __extract_type(obj);
			if(poly) return *poly;
			return typeid(void);
		}
	}
}