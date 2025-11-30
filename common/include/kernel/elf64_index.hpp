#ifndef __ELF64_INDEX
#define __ELF64_INDEX
#include <elf64.h>
#include <string>
#include <vector>
#include <unordered_map>
struct elf64_string_hash { constexpr uint32_t operator()(const char* data) const noexcept { uint32_t h = 0U; for(size_t i = 0UZ; data[i]; i++) { h = (h << 4) + static_cast<uint8_t>(data[i]); if (uint32_t g = (h & 0xF0000000U)) { h ^= g >> 24; h &= ~g; } } return h;  }; };
struct gnu_string_hash { constexpr uint32_t operator()(const char* data) const noexcept { uint32_t h = 5381U; for(size_t i = 0UZ; data[i]; i++) h += static_cast<uint8_t>(data[i]) + (h << 5); return h; } };
struct elf64_string_table
{
	size_t total_size;
	addr_t data;
	constexpr const char* operator[](size_t n) const { return data.plus(n); }
	constexpr elf64_string_table() = default;
	constexpr elf64_string_table(size_t size, addr_t data_ptr) : total_size(size), data(data_ptr) {}
	constexpr elf64_string_table(elf64_string_table&& that) : total_size(that.total_size), data(std::move(that.data)) { that.data = nullptr; that.total_size = 0; }
};
struct elf64_sym_table
{
	size_t total_size;
	size_t entry_size;
	addr_t data;
	typedef struct __symtab_iterator
	{
		addr_t pos;
		size_t entsz;
		constexpr elf64_sym const* operator->() const noexcept { return pos; }
		constexpr elf64_sym const& operator*() const noexcept { return pos.deref<elf64_sym const>(); }
		constexpr elf64_sym const& operator[](ptrdiff_t n) const noexcept { return pos.plus(n * entsz).deref<elf64_sym const>(); }
		constexpr __symtab_iterator& operator++() noexcept { pos += static_cast<ptrdiff_t>(entsz); return *this; }
		constexpr __symtab_iterator operator++(int) noexcept { __symtab_iterator that(*this); ++(*this); return that; }
		constexpr __symtab_iterator& operator--() noexcept { pos -= static_cast<ptrdiff_t>(entsz); return *this; }
		constexpr __symtab_iterator operator--(int) noexcept { __symtab_iterator that(*this); --(*this); return that; }
		constexpr __symtab_iterator operator+(ptrdiff_t n) const noexcept { return __symtab_iterator(pos.plus(n * entsz), entsz); }
		constexpr __symtab_iterator operator-(ptrdiff_t n) const noexcept { return __symtab_iterator(pos.minus(n * entsz), entsz); }
		friend constexpr std::strong_ordering operator<=>(__symtab_iterator const& __this, __symtab_iterator const& that) noexcept { return __this.entsz <=> that.entsz; }
		friend constexpr bool operator==(__symtab_iterator const& __this, __symtab_iterator const& that) noexcept { return __this.pos == that.pos && __this.entsz == that.entsz; }
	} iterator, const_iterator;
	constexpr iterator begin() noexcept { return iterator(data, entry_size); }
	constexpr const_iterator begin() const noexcept { return const_iterator(data, entry_size); }
	constexpr const_iterator cbegin() const noexcept { return const_iterator(data, entry_size); }
	constexpr iterator end() noexcept { return iterator(data.plus(total_size), entry_size); }
	constexpr const_iterator end() const noexcept { return const_iterator(data.plus(total_size), entry_size); }
	constexpr const_iterator cend() const noexcept { return const_iterator(data.plus(total_size), entry_size); }
	constexpr elf64_sym const* operator+(size_t n) const { return data.plus(entry_size * n); }
	constexpr elf64_sym const& operator[](size_t n) const { return *operator+(n); }
	constexpr size_t entries() const { return total_size / entry_size; }
	constexpr elf64_sym_table() = default;
	constexpr elf64_sym_table(size_t size, size_t entsz, addr_t data_ptr) : total_size(size), entry_size(entsz), data(data_ptr) {}
	constexpr elf64_sym_table(elf64_sym_table&& that) : total_size(that.total_size), entry_size(that.entry_size), data(std::move(that.data)) { that.data = nullptr; that.entry_size = 0; that.total_size = 0; }
};
struct elf64_sym_version
{
	bool base_or_weak;	// weak flag for requisites, or base flag for definitions
	uint32_t name_hash;
	std::string name;
	friend constexpr bool operator==(elf64_sym_version const& __this, elf64_sym_version const& __that) noexcept
	{
		if(__this.name_hash != __that.name_hash)
			return false;
		return __this.name == __that.name;
	}
	friend constexpr std::strong_ordering operator<=>(elf64_sym_version const& __this, elf64_sym_version const& __that) noexcept
	{
		if(__this.name_hash == __that.name_hash)
			return __this.name <=> __that.name;
		return __this.name_hash <=> __that.name_hash;
	}
};
struct elf64_version_index
{
	size_t verdef_num;
	std::vector<elf64_sym_version> definitions;
	void build(addr_t verdef_section, elf64_string_table const& strtab);
	bool check(elf64_sym_version const& ver) const;
	constexpr operator bool() const noexcept { return verdef_num > 0UZ; }
};
struct elf64_requisites_index
{
	size_t verneed_num;
	std::vector<elf64_sym_version> requisites;
	std::unordered_map<std::string, std::vector<uint16_t>> by_object;
	void build(addr_t verneed_section, elf64_string_table const& strtab);
	constexpr operator bool() const noexcept { return verneed_num > 0UZ; }
};
struct elf64_gnu_htbl
{
	struct hdr { uint32_t nbucket; uint32_t symndx; uint32_t maskwords; uint32_t shift2; } header;
	std::vector<uint64_t> bloom_filter_words;
	std::vector<uint32_t> buckets;
	std::vector<uint32_t> hash_value_array;
};
struct elf64_dynsym_index
{
	elf64_string_table& strtab;
	elf64_sym_table& symtab;
	elf64_gnu_htbl htbl;
	std::vector<uint16_t> versym;
	elf64_version_index verdef;
	elf64_requisites_index verneed;
	elf64_sym const* operator[](std::string const& str) const;
	constexpr operator bool() const noexcept { return htbl.bloom_filter_words.size() && htbl.buckets.size() && htbl.hash_value_array.size(); }
};
#endif