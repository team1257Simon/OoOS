#include <ext/type_erasure.hpp>
using namespace ABI_NAMESPACE;
using std::addressof;
struct vtable_header
{
	ptrdiff_t					leaf_offset;	/** Offset of the leaf (full/primary) object. */
	const __class_type_info*	type;			/** Type of the object. */
};
static std::type_info const* __extract_type(void* obj)
{
	if(__unlikely(!obj)) return std::addressof(typeid(nullptr));
	// treat obj as void**
	addr_t vpointer							= addr_t(obj).deref<addr_t>();
	// if obj points to something other than a virtual pointer, we can't dereference this or we'll fault
	if(__unlikely(!vpointer.is_canonical())) return nullptr;
	vtable_header const& target_header		= vpointer.minus(sizeof(vtable_header)).deref<vtable_header const>();
	// avoid passing an invalid pointer to dynamic_cast
	if(__unlikely(!addr_t(target_header.type).is_canonical())) return nullptr;
	std::type_info const* volatile type_v	= target_header.type;
	__class_type_info const* volatile ctype	= dynamic_cast<__class_type_info const*>(type_v);
	if(__unlikely(!ctype)) return nullptr;
	// if the type we have is valid, and the leaf offset is zero, we've found the type
	if(!target_header.leaf_offset) return ctype;
	addr_t leaf_vpointer					= addr_t(obj).plus(target_header.leaf_offset).deref<addr_t>();
	vtable_header const& leaf_header		= leaf_vpointer.minus(sizeof(vtable_header)).deref<vtable_header const>();
	return leaf_header.type;
}
static void* __extract_leaf(void* obj)
{
	// this will be called after the function above, meaning we've already checked this pointer
	vtable_header const& target_header		= addr_t(obj).deref<addr_t>().minus(sizeof(vtable_header)).deref<vtable_header const>();
	return addr_t(obj).plus(target_header.leaf_offset);
}
static bool __is_derived_from(__class_type_info const* __type, __class_type_info const* __base)
{
	if(__type == __base) return true;
	if(__type && __base)
	{
		if(__si_class_type_info const* sti = dynamic_cast<__si_class_type_info const*>(__type))
		{
			if(sti->__base_type == __base)
				return true;
			return __is_derived_from(sti->__base_type, __base);
		}
		else if(__vmi_class_type_info const* vti = dynamic_cast<__vmi_class_type_info const*>(__type))
		{
			for(unsigned int i = 0; i < vti->__base_count; i++)
				if(vti->__base_info[i].__base_type == __base)
					return true;
				else if(__is_derived_from(vti->__base_info[i].__base_type, __base))
					return true;
		}
	}
	return false;
}
static void* __reflective_cast(std::type_info const& from, std::type_info const& to, void* obj)
{
	if(__unlikely(!obj)) return nullptr;
	if(from == to || (dynamic_cast<__fundamental_type_info const*>(addressof(from)) && dynamic_cast<__fundamental_type_info const*>(addressof(to)))) return obj;
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
		type_erasure::type_erasure(type_info const& i) : type_index(i) {}
		void* type_erasure::cast_to(void* obj, type_info const& ti) const { return __reflective_cast(*info, ti, obj); }
		void* type_erasure::cast_from(void* obj, type_erasure const& that) const { return that.cast_to(obj, *info); }
		bool type_erasure::is_derived_from(type_info const& that)
		{
			__class_type_info const* cthis	= dynamic_cast<__class_type_info const*>(info);
			__class_type_info const* cthat	= dynamic_cast<__class_type_info const*>(addressof(that));
			if(cthis && cthat) return __is_derived_from(cthis, cthat);
			return false;
		}
		bool type_erasure::derives(type_info const& that)
		{
			__class_type_info const* cthis	= dynamic_cast<__class_type_info const*>(info);
			__class_type_info const* cthat	= dynamic_cast<__class_type_info const*>(addressof(that));
			if(cthis && cthat) return __is_derived_from(cthat, cthis);
			return false;
		}
		void* type_erasure::cast_inferred(void* obj) const
		{
			if(__unlikely(!obj)) return nullptr;
			type_info const* inferred_type	= __extract_type(obj);
			if(__unlikely(!inferred_type)) return nullptr;
			return __reflective_cast(*inferred_type, *info, __extract_leaf(obj));
		}
		type_info const& extract_typeid(void* obj)
		{
			type_info const* poly	= __extract_type(obj);
			if(poly) return *poly;
			return typeid(void);
		}
	}
}