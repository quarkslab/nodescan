#include <ns/buffer.h>
#include <string.h>
#include <iostream>

typedef ns::Buffer<unsigned char, uint32_t> BufferU8;

int main()
{
	int ret = 0;
#define CHECK(b)\
	if (!(b)) {\
		ret = 1;\
		std::cerr << #b << " check failed" << std::endl;\
	}\

	BufferU8 b;
	CHECK(b.begin() == nullptr);
	unsigned char* buf = b.grow_by(10);
	CHECK(b.size() == 10);
	CHECK(b.allocated_size() >= 10);
	CHECK(buf == b.begin());
	CHECK(buf+10 == b.end());

	memset(buf, 'A', 10);

	BufferU8 bc(b);
	CHECK(bc.begin() != nullptr);
	CHECK(bc.begin() != b.begin());
	CHECK(bc.size() == b.size());
	CHECK(memcmp(b.begin(), bc.begin(), 10) == 0);

	buf = b.grow_by(10);
	CHECK(b.size() == 20);
	CHECK(b.allocated_size() >= 20);
	CHECK(bc.size() == 10);
	memset(buf, 'B', 10);
	CHECK(memcmp(b.begin(), "AAAAAAAAAABBBBBBBBBB", 20) == 0);

	BufferU8 bm(std::move(b));
	CHECK(bm.begin() != b.begin());
	CHECK(bm.size() == 20);
	CHECK(memcmp(bm.begin(), "AAAAAAAAAABBBBBBBBBB", 20) == 0);

	bm.pop_by(5);
	CHECK(bm.size() == 15);
	CHECK(memcmp(bm.begin(), "AAAAABBBBBBBBBB", 15) == 0);

	return ret;
}
