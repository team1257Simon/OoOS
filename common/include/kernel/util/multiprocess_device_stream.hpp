#ifndef __MULTI_DEV_STREAM
#define __MULTI_DEV_STREAM
#include <ext/simplex_stream.hpp>
#include <fs/dev_stream.hpp>
#include <util/circular_queue.hpp>
#include <errno.h>
#include <stdexcept>
#include <unordered_map>
#include <vector>
namespace ooos
{
	template<std::char_type CT>
	class multiprocess_device_stream : private std::unordered_map<pid_t, std::ext::simplex_stream<circular_queue<CT>, std::vector<CT>>>, public dev_stream<CT>
	{
		typedef std::unordered_map<pid_t, std::ext::simplex_stream<circular_queue<CT>, std::vector<CT>>> __base;
		uint32_t __dev_id;
		using typename __base::iterator;
		using typename __base::const_iterator;
	public:
		typedef std::ext::simplex_stream<circular_queue<CT>, std::vector<CT>> stream_type;
		using typename dev_stream<CT>::size_type;
		using typename dev_stream<CT>::difference_type;
		using typename dev_stream<CT>::pointer;
		using typename dev_stream<CT>::const_pointer;
	protected:
		stream_type* active_stream;
		stream_type& add_or_get(pid_t key)
		{
			iterator i	= __base::find(key);
			if(i == __base::end())
				i		= __base::emplace(std::piecewise_construct, std::tuple<pid_t>(key), std::tuple<>()).first;
			return i->second;
		}
		template<typename FT>
		inline auto on_process(FT&& fn) -> std::invoke_result_t<FT, stream_type*>
		{
			typedef std::invoke_result_t<FT, stream_type*> result_t;
			pid_t key	= active_pid();
			if constexpr(std::is_void_v<result_t>) fn(std::addressof(add_or_get(key)));
			else return fn(std::addressof(add_or_get(key)));
		}
		inline size_type opt_process_avail() const
		{
			pid_t key			= active_pid();
			const_iterator i	= __base::find(key);
			if(i != __base::end()) return i->second.in_avail();
			return 0UZ;
		}
		inline size_type opt_process_tellg() const
		{
			pid_t key			= active_pid();
			const_iterator i	= __base::find(key);
			if(i != __base::end()) return i->second.tellg();
			return 0UZ;
		}
		static inline int stream_sync(stream_type* s) noexcept
		{
			try { s->flush(); }
			catch(...) { return -ENOMEM; }
			return 0; 
		}
		void activate(pid_t key) { active_stream = std::addressof(add_or_get(key)); }
		void push_to_active(CT c) { activate(active_pid()); active_stream->push(c); }
	public:
		multiprocess_device_stream(uint32_t id) : __base(64UZ), dev_stream<CT>(), __dev_id(id), active_stream(nullptr) {}
		multiprocess_device_stream(multiprocess_device_stream&&) = default;
		multiprocess_device_stream& operator=(multiprocess_device_stream&&) = default;
		virtual ~multiprocess_device_stream() {}
		virtual void on_open() override { add_or_get(active_pid()); }
		virtual int sync() override { return on_process(stream_sync); }
		virtual size_type read(pointer dest, size_type n) override { return on_process([dest, n](stream_type* s) -> size_type { return s->read(dest, n); }); }
		virtual size_type write(size_type n, const_pointer src) override { return active_stream ? active_stream->write(src, src + n) : 0UZ; }
		virtual size_type seek(int direction, difference_type where, uint8_t) override { return on_process([direction, where](stream_type* s) -> size_type { return s->seek(direction, where); }); }
		virtual size_type seek(size_type where, uint8_t) override { return on_process([where](stream_type* s) -> size_type { return s->seek(where); }); }
		virtual size_type avail() const override { return opt_process_avail(); }
		virtual size_type tellg() const override { return opt_process_tellg(); }
		virtual uint32_t get_device_id() const noexcept override { return __dev_id; }
	};
	
}
#endif