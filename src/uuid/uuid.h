#pragma once

#include <openssl/rand.h>

#include <string>

namespace Duktype
{
struct UUID
{
public:
	static std::string v4();

private:
	static int uuid_v4_gen(char* buffer);
};
}