#include <fs/sysfs.hpp>
static sysfs_vnode& create_sysfs_hashtable(sysfs& parent, std::string const& name, sysfs_object_type type, size_t buckets);
sysfs_hash_table_base::sysfs_hash_table_base(sysfs_vnode& n) : table_node(n) {}
sysfs_hash_table_base::sysfs_hash_table_base(sysfs& parent, std::string const& name, sysfs_object_type type, size_t buckets) : table_node(create_sysfs_hashtable(parent, name, type, buckets)) {}
sysfs_hashtable_header* sysfs_hash_table_base::header() { return static_cast<sysfs_hashtable_header*>(table_node.raw_data()); }
sysfs_hashtable_header const *sysfs_hash_table_base::header() const { return static_cast<sysfs_hashtable_header const*>(table_node.raw_data()); }
static sysfs_vnode& create_sysfs_hashtable(sysfs& parent, std::string const& name, sysfs_object_type type, size_t buckets)
{
	uint32_t table_node	= parent.mknod(name, sysfs_object_type::HASH_TABLE);
	uint32_t data_node	= parent.mknod(name + "#DATA", type);
	if(!table_node) throw std::runtime_error("[FS/SYSFS/HTAB] failed to create hashtable object node");
	if(!data_node) throw std::runtime_error("[FS/SYSFS/HTAB] failed to create hashtable data object node");
	size_t target_size	= sizeof(sysfs_hashtable_header) + buckets * sizeof(size_t) + sizeof(sysfs_hashtable_entry);
	sysfs_vnode& result	= parent.open(table_node);
	if(!result.expand_to_size(target_size)) throw std::bad_alloc();
	new(result.raw_data()) sysfs_hashtable_header
	{
		.values_object_ino	{ data_node },
		.num_buckets		{ buckets },
		.num_entries		{ 0UZ }
	};
	result.commit(target_size);
	parent.sync();
	return result;
}
size_t sysfs_hash_table_base::total_table_bytes() const
{
	sysfs_hashtable_header const* hdr	= header();
	size_t bucket_size					= hdr->num_buckets * sizeof(size_t);
	return bucket_size + hdr->num_entries * sizeof(sysfs_hashtable_entry);
}
bool sysfs_hash_table_base::add_entry(size_t object_hash, size_t object_index)
{
	table_node.pubseekpos(total_table_bytes() + sizeof(sysfs_hashtable_header));
	sysfs_hashtable_entry entry(object_index + 1, 0UZ);
	if(__unlikely(!table_node.sputn(reinterpret_cast<char*>(std::addressof(entry)), sizeof(sysfs_hashtable_entry)))) return false;
	sysfs_hashtable_header* hdr	= header();
	size_t bucket				= object_hash % hdr->num_buckets;
	size_t entry_idx			= hdr->num_entries;
	size_t commit_pos			= sizeof(sysfs_hashtable_header) + hdr->num_buckets * sizeof(size_t);
	size_t prev_idx				= hdr->buckets[bucket];
	if(prev_idx)
	{
		sysfs_hashtable_entry* current_entries		= hdr->entries();
		while(current_entries[prev_idx - 1].next_in_chain)
			prev_idx								= current_entries[prev_idx].next_in_chain;
		current_entries[prev_idx - 1].next_in_chain	= entry_idx + 1;
		commit_pos									+= sizeof(sysfs_hashtable_entry) * (prev_idx - 1);
	}
	else hdr->buckets[bucket]						= entry_idx + 1;
	hdr->num_entries++;
	if(sysfs_vnode::pos_type result					= table_node.commit(commit_pos); __unlikely(static_cast<std::streamoff>(result) == -1Z)) return false;
	table_node.pubsync();
	return table_node.sync_parent();
}
sysfs_hashtable_entry* sysfs_hash_table_base::get_chain_start(size_t object_hash)
{
	sysfs_hashtable_header* hdr	= header();
	if(size_t entry_idx			= hdr->buckets[object_hash % hdr->num_buckets]) return std::addressof(hdr->entries()[entry_idx - 1]);
	return nullptr;
}
sysfs_hashtable_entry* sysfs_hash_table_base::get_chain_next(sysfs_hashtable_entry* e)
{
	if(!e->next_in_chain) return nullptr;
	sysfs_hashtable_header* hdr	= header();
	size_t next_idx				= e->next_in_chain;
	if(next_idx > hdr->num_entries)
		throw std::out_of_range("[FS/SYSFS/HTAB] hashtable chain points out of range; the table might be corrupted");
	return std::addressof(hdr->entries()[next_idx - 1]);
}