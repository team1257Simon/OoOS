#include <ext/type_erasure.hpp>
namespace std
{
	using namespace ABI_NAMESPACE;
	static bool __is_derived_from(__class_type_info const* __type, __class_type_info const* __base)
	{
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
	namespace ext
	{
		static void* __reflective_cast(type_info const& from, type_info const& to, void* obj)
		{
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
	}
}