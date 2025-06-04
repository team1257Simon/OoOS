#include "users.hpp"
#include "ow-crypt.h"
void user_info::compute_csum() { checksum = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
bool user_info::verify_csum() const { return checksum == crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
bool user_info::check_pw(std::string const& pw) const
{
    std::string setting_str(credentials.crypto_setting_str, crypto_setting_len);
    std::string check_str = create_crypto_string(pw, setting_str);
    return !std::strncmp(check_str.c_str(), credentials.password_hash_str, crypto_hash_len);
}