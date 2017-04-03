#if defined(ED25519_GCC_64BIT_X86_CHOOSE)

#define HAVE_GE25519_SCALARMULT_BASE_CHOOSE_NIELS

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverlength-strings"
#endif

DONNA_NOINLINE static void
ge25519_scalarmult_base_choose_niels(ge25519_niels *t, const uint8_t table[256][96], uint32_t pos, signed char b) {
	int64_t breg = (int64_t)b;
	uint64_t sign = (uint64_t)breg >> 63;
	uint64_t mask = ~(sign - 1);
	uint64_t u = (breg + mask) ^ mask;

	__asm__ __volatile__ (
		/* ysubx+xaddy+t2d */
		"movq %0, %%rax                  ;\n"
		"movd %%rax, %%xmm14             ;\n"
		"pshufd $0x00, %%xmm14, %%xmm14  ;\n"
		"pxor %%xmm0, %%xmm0             ;\n"
		"pxor %%xmm1, %%xmm1             ;\n"
		"pxor %%xmm2, %%xmm2             ;\n"
		"pxor %%xmm3, %%xmm3             ;\n"
		"pxor %%xmm4, %%xmm4             ;\n"
		"pxor %%xmm5, %%xmm5             ;\n"

		/* 0 */
		"movq $0, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movq $1, %%rax                  ;\n"
		"movd %%rax, %%xmm6              ;\n"
		"pxor %%xmm7, %%xmm7             ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm6, %%xmm2              ;\n"
		"por %%xmm7, %%xmm3              ;\n"

		/* 1 */
		"movq $1, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 0(%1), %%xmm6            ;\n"
		"movdqa 16(%1), %%xmm7           ;\n"
		"movdqa 32(%1), %%xmm8           ;\n"
		"movdqa 48(%1), %%xmm9           ;\n"
		"movdqa 64(%1), %%xmm10          ;\n"
		"movdqa 80(%1), %%xmm11          ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 2 */
		"movq $2, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 96(%1), %%xmm6           ;\n"
		"movdqa 112(%1), %%xmm7          ;\n"
		"movdqa 128(%1), %%xmm8          ;\n"
		"movdqa 144(%1), %%xmm9          ;\n"
		"movdqa 160(%1), %%xmm10         ;\n"
		"movdqa 176(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 3 */
		"movq $3, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 192(%1), %%xmm6          ;\n"
		"movdqa 208(%1), %%xmm7          ;\n"
		"movdqa 224(%1), %%xmm8          ;\n"
		"movdqa 240(%1), %%xmm9          ;\n"
		"movdqa 256(%1), %%xmm10         ;\n"
		"movdqa 272(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 4 */
		"movq $4, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 288(%1), %%xmm6          ;\n"
		"movdqa 304(%1), %%xmm7          ;\n"
		"movdqa 320(%1), %%xmm8          ;\n"
		"movdqa 336(%1), %%xmm9          ;\n"
		"movdqa 352(%1), %%xmm10         ;\n"
		"movdqa 368(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 5 */
		"movq $5, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 384(%1), %%xmm6          ;\n"
		"movdqa 400(%1), %%xmm7          ;\n"
		"movdqa 416(%1), %%xmm8          ;\n"
		"movdqa 432(%1), %%xmm9          ;\n"
		"movdqa 448(%1), %%xmm10         ;\n"
		"movdqa 464(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 6 */
		"movq $6, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 480(%1), %%xmm6          ;\n"
		"movdqa 496(%1), %%xmm7          ;\n"
		"movdqa 512(%1), %%xmm8          ;\n"
		"movdqa 528(%1), %%xmm9          ;\n"
		"movdqa 544(%1), %%xmm10         ;\n"
		"movdqa 560(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 7 */
		"movq $7, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 576(%1), %%xmm6          ;\n"
		"movdqa 592(%1), %%xmm7          ;\n"
		"movdqa 608(%1), %%xmm8          ;\n"
		"movdqa 624(%1), %%xmm9          ;\n"
		"movdqa 640(%1), %%xmm10         ;\n"
		"movdqa 656(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* 8 */
		"movq $8, %%rax                  ;\n"
		"movd %%rax, %%xmm15             ;\n"
		"pshufd $0x00, %%xmm15, %%xmm15  ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa 672(%1), %%xmm6          ;\n"
		"movdqa 688(%1), %%xmm7          ;\n"
		"movdqa 704(%1), %%xmm8          ;\n"
		"movdqa 720(%1), %%xmm9          ;\n"
		"movdqa 736(%1), %%xmm10         ;\n"
		"movdqa 752(%1), %%xmm11         ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pand %%xmm15, %%xmm8            ;\n"
		"pand %%xmm15, %%xmm9            ;\n"
		"pand %%xmm15, %%xmm10           ;\n"
		"pand %%xmm15, %%xmm11           ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm8, %%xmm2              ;\n"
		"por %%xmm9, %%xmm3              ;\n"
		"por %%xmm10, %%xmm4             ;\n"
		"por %%xmm11, %%xmm5             ;\n"

		/* conditionally swap ysubx and xaddy */
		"movq %3, %%rax                  ;\n"
		"xorq $1, %%rax                  ;\n"
		"movd %%rax, %%xmm14             ;\n"
		"pxor %%xmm15, %%xmm15           ;\n"
		"pshufd $0x00, %%xmm14, %%xmm14  ;\n"
		"pxor %%xmm0, %%xmm2             ;\n"
		"pxor %%xmm1, %%xmm3             ;\n"
		"pcmpeqd %%xmm14, %%xmm15        ;\n"
		"movdqa %%xmm2, %%xmm6           ;\n"
		"movdqa %%xmm3, %%xmm7           ;\n"
		"pand %%xmm15, %%xmm6            ;\n"
		"pand %%xmm15, %%xmm7            ;\n"
		"pxor %%xmm6, %%xmm0             ;\n"
		"pxor %%xmm7, %%xmm1             ;\n"
		"pxor %%xmm0, %%xmm2             ;\n"
		"pxor %%xmm1, %%xmm3             ;\n"

		/* sspidere ysubx */
		"movq $0x7ffffffffffff, %%rax    ;\n"
		"movd %%xmm0, %%rcx              ;\n"
		"movd %%xmm0, %%r8               ;\n"
		"movd %%xmm1, %%rsi              ;\n"
		"pshufd $0xee, %%xmm0, %%xmm0    ;\n"
		"pshufd $0xee, %%xmm1, %%xmm1    ;\n"
		"movd %%xmm0, %%rdx              ;\n"
		"movd %%xmm1, %%rdi              ;\n"
		"shrdq $51, %%rdx, %%r8          ;\n"
		"shrdq $38, %%rsi, %%rdx         ;\n"
		"shrdq $25, %%rdi, %%rsi         ;\n"
		"shrq $12, %%rdi                 ;\n"
		"andq %%rax, %%rcx               ;\n"
		"andq %%rax, %%r8                ;\n"
		"andq %%rax, %%rdx               ;\n"
		"andq %%rax, %%rsi               ;\n"
		"andq %%rax, %%rdi               ;\n"
		"movq %%rcx, 0(%2)               ;\n"
		"movq %%r8, 8(%2)                ;\n"
		"movq %%rdx, 16(%2)              ;\n"
		"movq %%rsi, 24(%2)              ;\n"
		"movq %%rdi, 32(%2)              ;\n"

		/* sspidere xaddy */
		"movq $0x7ffffffffffff, %%rax    ;\n"
		"movd %%xmm2, %%rcx              ;\n"
		"movd %%xmm2, %%r8               ;\n"
		"movd %%xmm3, %%rsi              ;\n"
		"pshufd $0xee, %%xmm2, %%xmm2    ;\n"
		"pshufd $0xee, %%xmm3, %%xmm3    ;\n"
		"movd %%xmm2, %%rdx              ;\n"
		"movd %%xmm3, %%rdi              ;\n"
		"shrdq $51, %%rdx, %%r8          ;\n"
		"shrdq $38, %%rsi, %%rdx         ;\n"
		"shrdq $25, %%rdi, %%rsi         ;\n"
		"shrq $12, %%rdi                 ;\n"
		"andq %%rax, %%rcx               ;\n"
		"andq %%rax, %%r8                ;\n"
		"andq %%rax, %%rdx               ;\n"
		"andq %%rax, %%rsi               ;\n"
		"andq %%rax, %%rdi               ;\n"
		"movq %%rcx, 40(%2)              ;\n"
		"movq %%r8, 48(%2)               ;\n"
		"movq %%rdx, 56(%2)              ;\n"
		"movq %%rsi, 64(%2)              ;\n"
		"movq %%rdi, 72(%2)              ;\n"

		/* extract t2d */
		"movq $0x7ffffffffffff, %%rax    ;\n"
		"movd %%xmm4, %%rcx              ;\n"
		"movd %%xmm4, %%r8               ;\n"
		"movd %%xmm5, %%rsi              ;\n"
		"pshufd $0xee, %%xmm4, %%xmm4    ;\n"
		"pshufd $0xee, %%xmm5, %%xmm5    ;\n"
		"movd %%xmm4, %%rdx              ;\n"
		"movd %%xmm5, %%rdi              ;\n"
		"shrdq $51, %%rdx, %%r8          ;\n"
		"shrdq $38, %%rsi, %%rdx         ;\n"
		"shrdq $25, %%rdi, %%rsi         ;\n"
		"shrq $12, %%rdi                 ;\n"
		"andq %%rax, %%rcx               ;\n"
		"andq %%rax, %%r8                ;\n"
		"andq %%rax, %%rdx               ;\n"
		"andq %%rax, %%rsi               ;\n"
		"andq %%rax, %%rdi               ;\n"

		/* conditionally negate t2d */
		"movq %3, %%rax                  ;\n"
		"movq $0xfffffffffffda, %%r9     ;\n"
		"movq $0xffffffffffffe, %%r10    ;\n"
		"movq %%r10, %%r11               ;\n"
		"movq %%r10, %%r12               ;\n"
		"movq %%r10, %%r13               ;\n"
		"subq %%rcx, %%r9                ;\n"
		"subq %%r8, %%r10                ;\n"
		"subq %%rdx, %%r11               ;\n"
		"subq %%rsi, %%r12               ;\n"
		"subq %%rdi, %%r13               ;\n"
		"cmpq $1, %%rax                  ;\n"
		"cmove %%r9, %%rcx               ;\n"
		"cmove %%r10, %%r8               ;\n"
		"cmove %%r11, %%rdx              ;\n"
		"cmove %%r12, %%rsi              ;\n"
		"cmove %%r13, %%rdi              ;\n"

		/* sspidere t2d */
		"movq %%rcx, 80(%2)              ;\n"
		"movq %%r8, 88(%2)               ;\n"
		"movq %%rdx, 96(%2)              ;\n"
		"movq %%rsi, 104(%2)             ;\n"
		"movq %%rdi, 112(%2)             ;\n"
		:
		: "m"(u), "r"(&table[pos * 8]), "r"(t), "m"(sign) /* %0 = u, %1 = table, %2 = t, %3 = sign */
		:
			"%rax", "%rcx", "%rdx", "%rdi", "%rsi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", 
			"%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm14", "%xmm14",
			"cc", "memory"
	);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif /* defined(ED25519_GCC_64BIT_X86_CHOOSE) */

