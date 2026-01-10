#include "xhci.hpp"
namespace ooos
{
	static inline size_t seg_bytes(xhci_config_type const& cfg) { return sizeof(xhci_generic_trb) * get_element<1>(cfg); }
	static inline xhci_generic_trb* alloc_seg(abstract_module_base& mod, xhci_config_type const& cfg) { return addr_t(mod.allocate_dma(seg_bytes(cfg), get_element<3>(cfg))); }
	xhci_trb_ring_segment::xhci_trb_ring_segment(abstract_module_base& mod, xhci_config_type const& cfg) : __base(alloc_seg(mod, cfg), get_element<1>(cfg)), __delete(mod) {}
	xhci_trb_ring_segment::~xhci_trb_ring_segment() { if(__base::data()) __delete(__base::data(), __base::size_bytes()); }
}