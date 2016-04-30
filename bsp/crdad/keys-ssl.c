#include <stdint.h>
static BN_ULONG e_0[1] = {
	0x0000000000010001, 
};

#include <stdint.h>
static BN_ULONG n_0[32] = {
	0x63a2705416a0d8e1, 0xdc9fca11c8ba757b, 
	0xb9c06510cbcb35e3, 0x39e3dfebba941433, 
	0x7bbae38a6c1fce9d, 0x205a5a73fefabba7, 
	0x53ea3e5a97839a2e, 0xfec8f5b661dc0170, 
	0xefe311d8d29a1004, 0x8c6a92d0a5156bb8, 
	0x9067cc767a6eb5cc, 0xd103580b0bd5b1ff, 
	0x4a563e848f3a2daf, 0xacd7cadb46b0943e, 
	0x5fabb688ebd1e198, 0x7e70c1d35916f173, 
	0xaaa8acc85d6ca84e, 0x1685c157e20fd4dc, 
	0xf9e9c9c7ad933f64, 0xbe6272edc5f59824, 
	0x585d9a7d53447bd1, 0x011a5b3f5b3bc30d, 
	0xf312b966ffbbf0e9, 0x2203fb37482c131b, 
	0x3e7c157d0dc38eab, 0xb04de1d6b39fcc8d, 
	0x4d9f013707fc0d84, 0xb075a241e13b5ac5, 
	0x0a9a9d488e56e153, 0xf2cff393f97054eb, 
	0x2a2ead68376024f2, 0xd657997188d35dce, 
};

#include <stdint.h>
static BN_ULONG e_1[1] = {
	0x0000000000010001, 
};

#include <stdint.h>
static BN_ULONG n_1[32] = {
	0xa066f4dac4ff951d, 0xe6e0d246d5e1c45f, 
	0xe7fb461684e3c7a1, 0x11151b7af6e26899, 
	0x6c3f93fbc5ee7852, 0x96790b2bd0d8dec8, 
	0xb1722bf4a129207c, 0x3673e797044137b1, 
	0x183277072912661e, 0xd37e005c9a5ed820, 
	0x655b7f257568a1ea, 0xe731f136a29c63c6, 
	0x3036d253eeecac1e, 0x85ef7a7fa5cb80c7, 
	0x2ad91b7345ebba27, 0x715756f6ccd3df7d, 
	0x28900fac36fa6823, 0xf1026fe9469b935f, 
	0x98b8d15621f0531f, 0x180b2895b22dea88, 
	0x8ad9fe76a9fd602d, 0x510cd14519da1044, 
	0x0b09f9681184fbca, 0x578b96161cfd24d5, 
	0x3b1b0817146b61c4, 0x205bd497323d718b, 
	0x2d7e66f41eb31270, 0x389f7c6a52c2032a, 
	0x7c68dd6f3fd9d759, 0xac7ea58371257e90, 
	0xf239d7662c413815, 0xb540e39c28843903, 
};


struct pubkey {
	struct bignum_st e, n;
};

#define KEY(data) {				\
	.d = data,				\
	.top = sizeof(data)/sizeof(data[0]),	\
}

#define KEYS(e,n)	{ KEY(e), KEY(n), }

static struct pubkey keys[] = {
	KEYS(e_0, n_0),
	KEYS(e_1, n_1),
};
