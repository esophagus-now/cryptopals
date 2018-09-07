#ifndef AES_HPP
#define AES_HPP 1

#include "buffer.hpp"

bytebuf keyschedule(bytebuf key);

bytebuf decrypt128(const bytebuf &ctxt, bytebuf &key);

bytebuf decrypt(bytebuf &enc, bytebuf &key);

#endif
