#ifndef __STD_EXCEPTION
#define __STD_EXCEPTION
namespace std
{
    class exception
	{
	public:
		exception() noexcept;
		exception(exception const&) noexcept;
		exception& operator=(exception const&) noexcept;
		virtual ~exception() noexcept;
		virtual const char* what() const noexcept;
	};
}
#endif