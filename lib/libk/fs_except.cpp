#include <fs/permission_error.hpp>
#include <array>
#include <users.hpp>
static std::array<const char*, 8> perm_strings
{
	"",
	"(execute)",
	"(read)",
	"(read, execute)",
	"(write)",
	"(write, execute)",
	"(read, write)",
	"(read, write, execute)"
};
permission_error::permission_error(const char* msg) : runtime_error(msg) {}
permission_error::permission_error(std::string const& msg) : runtime_error(msg) {}
permission_error::~permission_error() = default;
extern "C"
{
	void __throw_permission_error(user_info const& user, uint8_t perm_check_bits)
	{
		size_t name_len	= std::strnlen(user.credentials.user_login_name, username_max_len);
		std::string err_msg	= "[FS] user " + std::string(user.credentials.user_login_name, name_len) + " lacks one or more permissions from " + std::string(perm_strings[perm_check_bits]);
		throw permission_error(err_msg);
	}
}