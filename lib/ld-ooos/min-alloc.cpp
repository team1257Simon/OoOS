/**
 * Minimalist memory allocation functions, optimized for size. This optimization allows all function calls to be relative,
 * meaning (since the kernel implements the bulk of dynamic linker logic) we can attach ld-ooos.so to any process at an arbitrary address.
 * Note that the memory allocated by this code will essentially be part of the spawning process' data segment.
 * Any calls to sbrk by user code will allocate memory after whatever this allocates (specifically for the link_map structures)
*/
#include "ld-ooos.hpp"
constexpr void* operator new(size_t, void* ptr) noexcept { return ptr; }
constexpr void* operator new[](size_t, void* ptr) noexcept { return ptr; }
constexpr void operator delete(void*, void*) noexcept {}
constexpr void operator delete[](void*, void*) noexcept {}
constexpr unsigned min_exponent = 8U;
constexpr unsigned max_block_index = 32U;
constexpr unsigned alloc_magic = 0xC001C0DE;
constexpr size_t min_block_size = 1UL << min_exponent;
constexpr size_t max_block_size = 1UL << max_block_index;
constexpr static size_t page_size = 4096UL;
constexpr size_t st_bits = __CHAR_BIT__ * sizeof(size_t);
constexpr static size_t __max(size_t a, size_t b) noexcept { return a > b ? a : b; }
constexpr static size_t __pages(size_t sz) noexcept { return (sz / page_size) + (sz % page_size ? 1UL : 0UL); }
constexpr static void* __alignup(void* ptr, size_t al) { return al > 1 ? (static_cast<char*>(ptr) + (al - (__builtin_bit_cast(uintptr_t, ptr) % al))) : ptr; }
typedef struct spinlock_t { volatile bool : 8; } *mutex_t;
static spinlock_t __mutex;
struct [[gnu::may_alias]] block_tag
{
    unsigned int magic      { alloc_magic };
    int index;
    size_t block_size;
    size_t held_size;
    block_tag* left_split   { nullptr };
    block_tag* right_split  { nullptr };
    block_tag* previous     { nullptr };
    block_tag* next         { nullptr };
    size_t align_bytes      { 0 };
    constexpr block_tag() = default;
    constexpr block_tag(size_t size, size_t held, int idx, block_tag* left, block_tag* right, block_tag* prev = nullptr, block_tag* nxt = nullptr, size_t align = 0) noexcept :
        index           { idx },
        block_size      { size },
        held_size       { held },
        left_split      { left },
        right_split     { right },
        previous        { prev },
        next            { nxt },
        align_bytes     { align }
                        {}
    constexpr block_tag(size_t size, size_t held, int idx = -1, size_t align = 0) noexcept : 
        index       { idx }, 
        block_size  { size }, 
        held_size   { held }, 
        align_bytes { align }
                    {}
    constexpr size_t allocated_size() const noexcept { return block_size - sizeof(block_tag); }
    constexpr size_t available_size() const noexcept { return allocated_size() - (held_size + align_bytes); }
    constexpr void* start() const noexcept { return const_cast<char*>(static_cast<const char*>(static_cast<const void*>(this)) + sizeof(block_tag)); }
    constexpr void* actual_start() const noexcept { return static_cast<char*>(start()) + align_bytes; }
	constexpr void* actual_end() const noexcept { return static_cast<char*>(actual_start()) + held_size; }
    constexpr size_t get_align(size_t al) const noexcept { if(!al) return 0; char* st = static_cast<char*>(start()); return static_cast<size_t>(static_cast<char*>(__alignup(st, al)) - st); }
    constexpr block_tag* set_align(size_t al) noexcept { align_bytes = get_align(al); return this; }
    __hidden block_tag* split() noexcept { block_tag* that = new(actual_end()) block_tag(available_size(), 0, -1, this, right_split); if(that->right_split) that->right_split->left_split = that; right_split = that; this->block_size -= that->block_size; return that; }
    __hidden void insert_at(int idx);
    __hidden void remove();
    __hidden bool absorb_right();
    __hidden block_tag* melt_left();
};
static block_tag* available_blocks[max_block_index - min_exponent]{};
static void* __min_sbrk(ptrdiff_t amt) { void* result; asm volatile("syscall" : "=a"(result) : "0"(6), "D"(amt) : "%r11", "%rcx", "memory"); if(long test = reinterpret_cast<long>(result); test < 0 && test > -4096) { errno = static_cast<int>(test); return nullptr; } return result; }
constexpr static uint32_t calculate_block_index(size_t size) { if(size < min_block_size) return 0; if(size > max_block_size) return max_block_index; return (st_bits - __builtin_clzl(size)) - min_exponent; }
static void alloc_lock() { while(__atomic_test_and_set(&__mutex, __ATOMIC_SEQ_CST)) { asm volatile("pause" ::: "memory"); } }
static void alloc_unlock() { __atomic_clear(&__mutex, __ATOMIC_SEQ_CST); }
static void* allocate_pages(size_t pages) { return __min_sbrk(static_cast<ptrdiff_t>(page_size * pages)); }
static block_tag* locate_tag(void* ptr, size_t align)
{
    char* cptr = static_cast<char*>(ptr) - sizeof(block_tag);
    for(size_t i = 0; i < __max(align, 8) && i < reinterpret_cast<uintptr_t>(cptr); i++) { if(block_tag* tag = reinterpret_cast<block_tag*>(cptr - i); tag->magic == alloc_magic) return tag; }
    return nullptr;
}
static void replace_tag(block_tag* tag)
{
    if(tag)
    {
        alloc_lock();
        while(tag->left_split && tag->left_split->index >= 0) tag = tag->melt_left();
        while(tag->absorb_right());
        tag->held_size = 0;
        tag->align_bytes = 0;
        tag->insert_at(calculate_block_index(tag->available_size()));
        alloc_unlock();
    }
}
static block_tag* create_tag(size_t size, size_t align)
{
    size_t actual = __max(size + sizeof(block_tag), align) + align;
    void* allocated = allocate_pages(__pages(actual));
    if(!allocated) return nullptr;
    block_tag* tag = (new(allocated) block_tag(__pages(actual) * page_size, size))->set_align(align);
    if(tag->available_size() >= min_block_size + sizeof(block_tag)) tag->split()->insert_at(-1);
    return tag;
}
static block_tag* find_tag(size_t size, size_t al)
{
    unsigned idx = calculate_block_index(size);
    for(block_tag* tag = available_blocks[idx]; tag != nullptr; tag = tag->next)
    {
        if(tag->available_size() >= size + tag->get_align(al))
        {
            tag->remove();
            tag->held_size = size;
            tag->set_align(al);
            if(tag->available_size() >= min_block_size + sizeof(block_tag)) tag->split()->insert_at(-1);
            return tag;
        }
    }
    return nullptr;
}
__hidden void deallocate(void* ptr, size_t al) 
{
    if(!ptr) return;
    block_tag* tag = locate_tag(ptr, al);
    if(tag) replace_tag(tag);
}
__hidden void* allocate(size_t count, size_t al)
{
    if(!count) return nullptr;
    alloc_lock();
    block_tag* tag = find_tag(count, al);
    if(!tag) tag = create_tag(count, al);
    alloc_unlock();
    return tag ? tag->actual_start() : nullptr;
}
__hidden void block_tag::insert_at(int idx)
{
    index = idx < 0 ? calculate_block_index(block_size) : idx;
    if(available_blocks[index]) { next = available_blocks[index]; available_blocks[index]->previous = this; }
    available_blocks[index] = this;
}
__hidden void block_tag::remove()
{
    if(available_blocks[index] == this) available_blocks[index] = next;
    if(previous) previous->next = next;
    if(next) next->previous = previous;
    next = nullptr;
    previous = nullptr;
    index = -1;
}
__hidden bool block_tag::absorb_right()
{
    block_tag* that = right_split;
    if(that && that->index >= 0)
    {
        that->remove();
        this->block_size += that->block_size;
        this->right_split = that->right_split;
        if(that->right_split) that->right_split->left_split = this;
        return true;
    }
    return false;
}
__hidden block_tag* block_tag::melt_left()
{
    block_tag* that = left_split;
    if(that && that->index >= 0)
    {
        that->block_size += this->block_size;
        that->right_split = this->right_split;
        if(this->right_split) this->right_split->left_split = that;
        this->remove();
        return that;
    }
    return this;
}