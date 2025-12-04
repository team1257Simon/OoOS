#ifndef __SYSFS
#define __SYSFS
/**
 * While sysfs isn't technically a proper filesystem, it is a part of the filesystem and uses many of the same concepts.
 * It's how the various configurations and other system objects (such as user account info) are persisted.
 * A given object will be tagged in the directory file with a name up to 23 characters long.
 * This limit allows directory entries to be precisely 32 bytes in length and still include a checksum.
 * Each object's index node specifies a type code (such as user info, network config, etc) and points to an extent tree.
 * The extent trees are essentially simplified versions of the ext4 extent system.
 * All extent nodes beyond those in the index itself are stored in a separate file, with non-leaf nodes pointing to an ordinal within that file.
 * Leaf nodes, of course, point to blocks in the data file itself.
 * All checksums are calculated as crc32c(~0, object) with the computation stopping just before the checksum field.
 * Eventually, I might expand sysfs to support a sort-of registry that applications and drivers can also use.
 */
#include <fs/fs.hpp>
#include <sys/errno.h>
#include <typeindex>
#include <unordered_map>
// This template declaration is long enough that using a macro saves a significant amount of space.
#define sysfs_htbl_template template<typename KT, typename VT, std::__detail::__key_extract<KT, VT> XT, std::__detail::__hash_ftor<KT> HT, std::__detail::__predicate<KT> ET>
constexpr uint32_t sysfs_magic                  = 0xA11C0DED;
constexpr uint32_t sysfs_extent_magic           = 0xB16F11E5;
constexpr uint32_t sysfs_directory_magic        = 0xCA11ED17;
constexpr uint32_t sysfs_index_magic            = 0xD1617A15;
constexpr uint32_t sysfs_htbl_magic             = 0xED6E108D;
constexpr size_t sysfs_extent_branch_size       = 128UZ;
constexpr size_t sysfs_data_block_size          = 1024UZ;
constexpr size_t sysfs_object_name_size_max     = 22UZ;
enum class sysfs_object_type : uint16_t
{
	NONE,
	GENERAL_CONFIG,
	USER_INFO,
	NET_CONFIG,
	HASH_TABLE,
	STRING_TABLE,
	// ...
};
struct sysfs_extent_entry
{
	uint32_t ordinal;   // x: nth block in this sequence -> (n+x)th block in data entry
	uint32_t length;    // length in blocks if this is a leaf
	size_t start;       // points to a block in the data file if this is a leaf; otherwise points to another branch
};
struct sysfs_data_block
{
	char data[sysfs_data_block_size];
	constexpr operator char*() & noexcept { return data; }
	constexpr operator const char*() const& noexcept { return data; }
};
struct sysfs_extent_branch
{
	constexpr static size_t num_entries = (sysfs_extent_branch_size - (sizeof(uint64_t) + sizeof(uint64_t))) / sizeof(sysfs_extent_entry);
	uint64_t depth;                 // 0 if the branch contains leaves; otherwise, the number of branches to be traversed before reaching leaves.
	sysfs_extent_entry entries[num_entries];
	uint64_t checksum;              // 32-bit checksum, but we had extra space
	constexpr sysfs_extent_entry& operator[](size_t i) & { return entries[i]; }
	constexpr sysfs_extent_entry const& operator[](size_t i) const& { return entries[i]; }
};
struct __pack sysfs_inode
{
	sysfs_object_type type;
	uint16_t root_depth;            // at least 1 — the entry stored here might point to a branch containing leaves, but will never itself be a leaf
	size_t size_bytes;
	sysfs_extent_entry extent_root;
	uint32_t checksum;
};
struct __pack sysfs_dir_entry
{
	uint32_t inode_number;
	sysfs_object_type object_type;  // this is also stored in the inode
	char object_name[sysfs_object_name_size_max];
	uint32_t checksum;
};
struct __pack sysfs_index_file
{
	uint32_t magic = sysfs_index_magic;
	char backup_file_name[16];
	size_t total_inodes;
	uint32_t header_checksum;
	sysfs_inode inodes[];
};
struct __pack sysfs_extents_file
{
	uint32_t magic = sysfs_extent_magic;
	char backup_file_name[16];
	size_t total_branches;
	uint32_t header_checksum;
	sysfs_extent_branch branches[];
};
struct __pack sysfs_directory_file
{
	uint32_t magic = sysfs_directory_magic;
	char backup_file_name[16];
	size_t total_entries;
	uint32_t header_checksum;
	sysfs_dir_entry entries[];
};
struct __pack sysfs_data_file_header
{
	uint32_t magic = sysfs_magic;
	char backup_file_name[16];
	size_t total_size;
	uint32_t header_checksum;
};
struct __pack sysfs_hashtable_entry { size_t object_index; size_t next_in_chain; };
struct sysfs_hashtable_header
{
	uint32_t magic = sysfs_htbl_magic;
	uint32_t values_object_ino;     // inode number for the actual object data; it will be tagged as table_name#DATA where table_name is this table's tag
	size_t num_buckets;             // length of the buckets array; 32 by default
	size_t num_entries;             // the actual table size
	size_t buckets[];
	constexpr sysfs_hashtable_entry* entries() { return addr_t(std::addressof(buckets[num_buckets])); }
};
class sysfs;
struct sysfs_vnode;
class sysfs_extent_tree
{
	constexpr static size_t __start_pos     = sysfs_extent_branch::num_entries / 2;
	constexpr static size_t __start_incr    = div_to_nearest(sysfs_extent_branch::num_entries, 4U);
	sysfs_vnode& __managed_vnode;
	size_t __total_extent;
	size_t __stored_leaf_index;
	sysfs_inode& __inode();
	sysfs_inode const& __inode() const;
	sysfs_extent_branch& __root();
	sysfs_extent_branch const& __root() const;
	sysfs_extent_entry& __root_first();
	sysfs_extent_entry const& __root_first() const;
	sysfs_extent_entry const& __find_from(sysfs_extent_branch const& b, size_t idx, size_t pos = __start_pos, size_t incr = __start_incr) const;
	void __overflow_root();
	sysfs_extent_branch& __next_leaf_branch();
public:
	sysfs_extent_tree(sysfs_vnode& n);
	void push(uint16_t n_blocks);
	/**
	* Gets the entry corresponding to the block in the file at ordinal i.
	* If the index is part of a block of length greater than 1, the entry's ordinal might be less than i.
	* In these cases, the actual block number in the data file is ((i - e.ordinal) + e.start), where e is the returned entry.
	*/
	sysfs_extent_entry const& operator[](size_t i) const;
	/**
	* The total extent of the file, in blocks.
	* Note that this value multiplied by the sector size might be greater than the size in bytes stored in the inode.
	* It will never be less, however.
	*/
	size_t total_extent() const;
};
struct block_range { size_t start; size_t end; };
struct sysfs_vnode : std::ext::dynamic_streambuf<char>
{
	sysfs_inode& inode();
	sysfs_inode const& inode() const;
	sysfs& parent();
	uint32_t inode_number() const;
	void init();                        // performs some setup; the constructor automatically calls this.
	pos_type commit(size_t target_pos); // marks all blocks in the object up to and including the given position as needing to be written to disk.
	bool sync_parent();
	bool expand_to_size(size_t target);
	bool expand_by_size(size_t added);
	void* raw_data();
	const void* raw_data() const;
	sysfs_vnode(sysfs& sysfs_parent, uint32_t inode_num);
protected:
	friend class sysfs_extent_tree;
	using __base = std::ext::dynamic_streambuf<char>;
	sysfs& parent_fs;
	uint32_t ino;
	sysfs_extent_tree extent_tree;
	std::vector<block_range> dirty_blocks;
	virtual void on_modify() override;
	virtual int write_dev() override;
	virtual std::streamsize on_overflow(std::streamsize n) override;
	virtual std::streamsize sector_size() const override;
	virtual std::streamsize xsputn(char const* s, std::streamsize n) override;
	uint32_t actual_block(size_t ordinal) const;
	size_t block_capacity() const;
	void calc_inode_csum();
};
struct sysfs_file_paths
{
	std::string data_file;
	std::string index_file;
	std::string extents_file;
	std::string directory_file;
};
struct sysfs_backup_filenames
{
	char data_backup_file_name[16];
	char index_backup_file_name[16];
	char extents_backup_file_name[16];
	char directory_backup_file_name[16];
};
class sysfs
{
	filesystem& __backend;
	file_vnode& __data_file;
	file_vnode& __index_file;
	file_vnode& __extents_file;
	file_vnode& __directory_file;
	std::unordered_map<std::string, uint32_t> __directory_map;
	std::unordered_map<uint32_t, sysfs_vnode> __opened_nodes;
	sysfs_index_file& __index();
	sysfs_directory_file& __dir();
	sysfs_extents_file& __extents();
	sysfs_data_file_header& __header();
	sysfs_data_block& __block(size_t num);
	size_t __num_blocks() const;
public:
	~sysfs();
	sysfs(filesystem* backend, sysfs_file_paths const& files);
	void init_load();
	void init_blank(sysfs_backup_filenames const& bak);
	void write_data(size_t start_block, const char* data, size_t n);
	void read_data(char* out, size_t start_block, size_t n);
	bool sync();
	sysfs_extent_branch& get_extent_branch(size_t idx);
	sysfs_extent_branch const& get_extent_branch(size_t idx) const;
	sysfs_inode& get_inode(size_t ino);
	sysfs_inode const& get_inode(size_t ino) const;
	sysfs_dir_entry& get_dir_entry(size_t num);
	sysfs_dir_entry const& get_dir_entry(size_t num) const;
	uint32_t add_extent_branch();
	uint32_t add_blocks(uint16_t how_many);
	uint32_t add_inode();
	uint32_t add_directory_entry();
	uint32_t find_node(std::string const& name);
	void dir_add_object(std::string const& name, uint32_t ino);
	sysfs_extent_branch& extend_to_leaf(size_t from_idx, uint32_t ordinal);
	std::pair<sysfs_extent_branch*, size_t> next_available_extent_entry(size_t from_idx);
	uint32_t mknod(std::string const& name, sysfs_object_type type);
	sysfs_vnode& open(uint32_t ino);
};
/**
 * Represents a non-owning reference to an object stored in the sysfs.
 * The handle can be used to read and modify the object.
 * It automatically invokes the commit and sync methods on the object's handle when it goes out of scope.
 * Alternatively, these can be manually called via commit_object.
 * Note that, because these are non-owning references, they are copy-constructible but not assignable.
 * However, if repeatedly copied, they will repeatedly commit the node, potentially with no actual changes, which is somewhat inefficient.
 */
template<typename T>
struct sysfs_object_handle
{
	sysfs_vnode& object_node;
	typedef T value_type;
	typedef T* pointer;
	typedef T const* const_pointer;
	typedef T& reference;
	typedef T const& const_reference;
	sysfs_object_handle(sysfs_vnode& n) : object_node(n) { if(!object_node.expand_to_size(sizeof(T))) throw std::runtime_error("[sysfs] no space in object file"); object_node.pubsync(); }
	sysfs_object_handle(sysfs_vnode& n, T const& t) requires(std::is_trivially_copy_constructible_v<T>) : object_node(n) { if(!object_node.sputn(reinterpret_cast<const char*>(std::addressof(t)), sizeof(T))) throw std::runtime_error("[sysfs] no space in object file"); object_node.pubsync(); }
	sysfs_object_handle(sysfs& parent, uint32_t ino) : sysfs_object_handle(parent.open(ino)) {}
	sysfs_object_handle(sysfs_object_handle const& that) : object_node(that.object_node) {}
	sysfs_object_handle(sysfs_object_handle&& that) : object_node(that.object_node) {}
	reference operator*() & { return *static_cast<pointer>(object_node.raw_data()); }
	const_reference operator*() const& { return *static_cast<const_pointer>(object_node.raw_data()); }
	pointer operator->() & { return static_cast<pointer>(object_node.raw_data()); }
	const_pointer operator->() const& { return static_cast<const_pointer>(object_node.raw_data()); }
	void commit_object() { object_node.commit(sizeof(T)); object_node.pubsync(); }
	~sysfs_object_handle() { commit_object(); object_node.sync_parent(); }
};
/**
 * Tabulated data in sysfs is stored in hashtables.
 * These take up two objects: one containing the table's pointers and the other containing the actual data.
 * The table header contains a number of buckets (32 by default) which in turn point entries in the table.
 * The entries themselves consist of an index into the data and a pointer to the next entry in a chain (in case of hash collisions).
 * A value of zero indicates the end of a chain.
 * The index is a multiple of the size of the stored object type (i.e. as though indexing a C-style array).
 */
struct sysfs_hash_table_base
{
	sysfs_vnode& table_node;
	sysfs_hash_table_base(sysfs_vnode& n);
	sysfs_hash_table_base(sysfs& parent, std::string const& name, sysfs_object_type type, size_t buckets = 32UZ);
	size_t total_table_bytes() const;
	bool add_entry(size_t object_hash, size_t object_index);
	sysfs_hashtable_entry* get_chain_start(size_t object_hash);
	sysfs_hashtable_entry* get_chain_next(sysfs_hashtable_entry* e);
	sysfs_hashtable_header* header();
	sysfs_hashtable_header const* header() const;
};
sysfs_htbl_template struct sysfs_hash_table;
/**
 * Similar to a sysfs_object_handle<VT>, except represents an object stored in a hashtable.
 * The destructor syncs the table's data up to and including the block containing the object.
 * If the handle is released before it goes out of scope, the sync does not occur.
 * This is useful to save time if the handle is accessed as read-only.
 */
sysfs_htbl_template struct sysfs_table_entry_handle
{
	typedef sysfs_hash_table<KT, VT, XT, HT, ET> parent_table_type;
	parent_table_type& parent;
	size_t value_index;
	constexpr void release() noexcept { value_index = 0UZ; }
	constexpr sysfs_table_entry_handle(parent_table_type& p, size_t i) : parent(p), value_index(i) {}
	constexpr sysfs_table_entry_handle(sysfs_table_entry_handle const& that) : parent(that.parent), value_index(that.value_index) {}
	constexpr sysfs_table_entry_handle(sysfs_table_entry_handle&& that) : parent(that.parent), value_index(that.value_index) { that.release(); }
	constexpr sysfs_table_entry_handle& operator++() { value_index++; return *this; }
	void commit_object();
	~sysfs_table_entry_handle() { commit_object(); parent.table_node.sync_parent(); }
	VT* ptr();
	VT const* ptr() const;
	VT* operator->();
	VT const* operator->() const;
	VT& operator*();
	VT const& operator*() const;
	template<typename ... Args> requires(std::constructible_from<VT, Args...>) void emplace(Args&& ... args) { std::construct_at(ptr(), std::forward<Args>(args)...); }
};
/**
 * Implements the logic that is dependent on the stored object type.
 * This structure is still a non-owning reference but, unlike the object handle, does not commit when it goes out of scope.
 * Instead, certain accesses (in particular, insertions) will call the commit and sync functions on the table object node.
 * The value handles returned by all accesses will commit the data object node in the same manner and under the same circumstances as a regular object handle.
 * Note that this structure, while it uses many of the same concepts as an std::unordered_map, is entirely contained within the sysfs data buffers.
 * It does not allocate memory directly, nor can it safely store types whose constructors have side-effects.
 */
sysfs_htbl_template struct sysfs_hash_table : sysfs_hash_table_base
{
	constexpr static size_t nxobj = 0UZ;
	typedef sysfs_table_entry_handle<KT, VT, XT, HT, ET> value_handle;
	sysfs_vnode& object_node;
	sysfs_hash_table(sysfs_vnode& n) : sysfs_hash_table_base(n), object_node(n.parent().open(header()->values_object_ino)) {}
	sysfs_hash_table(sysfs& parent, std::string const& name, sysfs_object_type type, size_t buckets = 32UZ) : sysfs_hash_table_base(parent, name, type, buckets), object_node(parent.open(header()->values_object_ino)) {}
	VT* data() { return static_cast<VT*>(object_node.raw_data()); }
	VT const* data() const { return static_cast<VT const*>(object_node.raw_data()); }
	constexpr static size_t hash_of(KT const& key) { return HT{}(key); }
	constexpr static KT key_of(VT const& value) { return XT{}(value); }
	constexpr static bool key_matches(KT const& key, VT const& value) { return ET{}(key, XT{}(value)); }
	size_t find_value_index(KT const& key);
	size_t size() const { return header()->num_entries; }
	value_handle find(KT const& key);
	value_handle get(KT const& key) { size_t existing = find_value_index(key); if(existing == nxobj) throw std::out_of_range("[sysfs] key not found"); return value_handle(*this, existing); }
	bool contains(KT const& key) { return find_value_index(key) != nxobj; }
	std::pair<value_handle, bool> add(VT const& value);
};
struct sysfs_string_table
{
	sysfs_vnode& node;
	sysfs_string_table(sysfs& parent, uint32_t ino);
	sysfs_string_table(sysfs_vnode& vn);
	bool write(std::string const& str, off_t where);
	off_t write(std::string const& str);
	std::string read(off_t pos) const;
	size_t size() const;
};
sysfs_htbl_template void sysfs_table_entry_handle<KT, VT, XT, HT, ET>::commit_object() { if(value_index) { parent.object_node.commit((value_index - 1) * sizeof(VT)); parent.object_node.pubsync(); } }
sysfs_htbl_template VT* sysfs_table_entry_handle<KT, VT, XT, HT, ET>::ptr() { return std::addressof(parent.data()[value_index - 1]); }
sysfs_htbl_template VT const* sysfs_table_entry_handle<KT, VT, XT, HT, ET>::ptr() const { return std::addressof(parent.data()[value_index - 1]); }
sysfs_htbl_template VT* sysfs_table_entry_handle<KT, VT, XT, HT, ET>::operator->() { return ptr(); }
sysfs_htbl_template VT const* sysfs_table_entry_handle<KT, VT, XT, HT, ET>::operator->() const { return ptr(); }
sysfs_htbl_template VT& sysfs_table_entry_handle<KT, VT, XT, HT, ET>::operator*() { return parent.data()[value_index - 1]; }
sysfs_htbl_template VT const& sysfs_table_entry_handle<KT, VT, XT, HT, ET>::operator*() const { return parent.data()[value_index - 1]; }
sysfs_htbl_template size_t sysfs_hash_table<KT, VT, XT, HT, ET>::find_value_index(KT const& key)
{
	VT* vs = data();
	for(sysfs_hashtable_entry* e = get_chain_start(hash_of(key)); e != nullptr; e = get_chain_next(e))
	{
		VT& v = vs[e->object_index - 1];
		if(key_matches(key, v))
			return e->object_index;
	}
	return nxobj;
}
sysfs_htbl_template typename sysfs_hash_table<KT, VT, XT, HT, ET>::value_handle sysfs_hash_table<KT, VT, XT, HT, ET>::find(KT const& key)
{
	size_t existing = find_value_index(key);
	if(existing == nxobj)
	{
		size_t target_index = size();
		if(__unlikely(!object_node.expand_by_size(sizeof(VT)) || !add_entry(hash_of(key), target_index))) throw std::bad_alloc();
		return value_handle(*this, target_index + 1);
	}
	else return value_handle(*this, existing);
}
sysfs_htbl_template std::pair<typename sysfs_hash_table<KT, VT, XT, HT, ET>::value_handle, bool> sysfs_hash_table<KT, VT, XT, HT, ET>::add(VT const& value)
{
	typedef std::pair<typename sysfs_hash_table<KT, VT, XT, HT, ET>::value_handle, bool> result_type;
	size_t existing = find_value_index(key_of(value));
	if(existing == nxobj)
	{
		size_t target_index = size();
		if(__unlikely(!object_node.expand_by_size(sizeof(VT)) || !add_entry(hash_of(key_of(value)), target_index))) throw std::bad_alloc();
		value_handle result(*this, target_index + 1);
		*result = value;
		return result_type(result, true);
	}
	else return result_type(std::move(value_handle(*this, existing)), false);
}
#undef sysfs_htbl_template
#endif