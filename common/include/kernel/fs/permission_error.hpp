#ifndef __PERM_ERROR
#define __PERM_ERROR
#include <stdexcept>
struct user_info;
class permission_error : public std::runtime_error
{
public:
	explicit permission_error(const char* msg);
	explicit permission_error(std::string const& msg);
	permission_error(permission_error const&) = default;
	permission_error(permission_error&&) = default;
	permission_error& operator=(permission_error const&) = default;
	permission_error& operator=(permission_error&&) = default;
	virtual ~permission_error() noexcept;
};
extern "C" void __throw_permission_error(user_info const& user, uint8_t perm_check_bits) attribute(noreturn);
#endif