#if defined(ED25519_GCC_32BIT_SSE_CHOOSE)

#define HAVE_GE25519_SCALARMULT_BASE_CHOOSE_NIELS

DONNA_NOINLINE static void
ge25519_scalarmult_base_choose_niels(ge25519_niels *t, const uint8_t table[256][96], uint32_t pos, signed char b) {
	int32_t breg = (int32_t)b;
	uint32_t sign = (uint32_t)breg >> 31;
	uint32_t mask = ~(sign - 1);
	uint32_t u = (breg + mask) ^ mask;

	__asm__ __volatile__ (
		/* ysubx+xaddy */
		"movl %0, %%eax                  ;\n"
		"movd %%eax, %%xmm6              ;\n"
		"pshufd $0x00, %%xmm6, %%xmm6    ;\n"
		"pxor %%xmm0, %%xmm0             ;\n"
		"pxor %%xmm1, %%xmm1             ;\n"
		"pxor %%xmm2, %%xmm2             ;\n"
		"pxor %%xmm3, %%xmm3             ;\n"

		/* 0 */
		"movl $0, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movl $1, %%ecx                  ;\n"
		"movd %%ecx, %%xmm4              ;\n"
		"pxor %%xmm5, %%xmm5             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 1 */
		"movl $1, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 0(%1), %%xmm4            ;\n"
		"movdqa 16(%1), %%xmm5           ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 32(%1), %%xmm4           ;\n"
		"movdqa 48(%1), %%xmm5           ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 2 */
		"movl $2, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 96(%1), %%xmm4           ;\n"
		"movdqa 112(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 128(%1), %%xmm4          ;\n"
		"movdqa 144(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 3 */
		"movl $3, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 192(%1), %%xmm4          ;\n"
		"movdqa 208(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 224(%1), %%xmm4          ;\n"
		"movdqa 240(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 4 */
		"movl $4, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 288(%1), %%xmm4          ;\n"
		"movdqa 304(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 320(%1), %%xmm4          ;\n"
		"movdqa 336(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 5 */
		"movl $5, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 384(%1), %%xmm4          ;\n"
		"movdqa 400(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 416(%1), %%xmm4          ;\n"
		"movdqa 432(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 6 */
		"movl $6, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 480(%1), %%xmm4          ;\n"
		"movdqa 496(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 512(%1), %%xmm4          ;\n"
		"movdqa 528(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 7 */
		"movl $7, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 576(%1), %%xmm4          ;\n"
		"movdqa 592(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 608(%1), %%xmm4          ;\n"
		"movdqa 624(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* 8 */
		"movl $8, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 672(%1), %%xmm4          ;\n"
		"movdqa 688(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm0              ;\n"
		"por %%xmm5, %%xmm1              ;\n"
		"movdqa 704(%1), %%xmm4          ;\n"
		"movdqa 720(%1), %%xmm5          ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"por %%xmm4, %%xmm2              ;\n"
		"por %%xmm5, %%xmm3              ;\n"

		/* conditional swap based on sign */
		"movl %3, %%ecx                  ;\n"
		"movl %2, %%eax                  ;\n"
		"xorl $1, %%ecx                  ;\n"
		"movd %%ecx, %%xmm6              ;\n"
		"pxor %%xmm7, %%xmm7             ;\n"
		"pshufd $0x00, %%xmm6, %%xmm6    ;\n"
		"pxor %%xmm0, %%xmm2             ;\n"
		"pxor %%xmm1, %%xmm3             ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa %%xmm2, %%xmm4           ;\n"
		"movdqa %%xmm3, %%xmm5           ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"pand %%xmm7, %%xmm5             ;\n"
		"pxor %%xmm4, %%xmm0             ;\n"
		"pxor %%xmm5, %%xmm1             ;\n"
		"pxor %%xmm0, %%xmm2             ;\n"
		"pxor %%xmm1, %%xmm3             ;\n"

		/* sspidere ysubx */
		"movd %%xmm0, %%ecx              ;\n"
		"movl %%ecx, %%edx               ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 0(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $26, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 4(%%eax)            ;\n"
		"movd %%xmm0, %%edx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $19, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 8(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"shrdl $13, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 12(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrl $6, %%ecx                  ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 16(%%eax)           ;\n"
		"movl %%edx, %%ecx               ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 20(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $25, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 24(%%eax)           ;\n"
		"movd %%xmm1, %%ecx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $19, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 28(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"shrdl $12, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 32(%%eax)           ;\n"
		"shrl $6, %%edx                  ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"xorl %%ecx, %%ecx               ;\n"
		"movl %%edx, 36(%%eax)           ;\n"
		"movl %%ecx, 40(%%eax)           ;\n"
		"movl %%ecx, 44(%%eax)           ;\n"

		/* sspidere xaddy */
		"addl $48, %%eax                 ;\n"
		"movdqa %%xmm2, %%xmm0           ;\n"
		"movdqa %%xmm3, %%xmm1           ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"movl %%ecx, %%edx               ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 0(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $26, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 4(%%eax)            ;\n"
		"movd %%xmm0, %%edx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $19, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 8(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"shrdl $13, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 12(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrl $6, %%ecx                  ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 16(%%eax)           ;\n"
		"movl %%edx, %%ecx               ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 20(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $25, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 24(%%eax)           ;\n"
		"movd %%xmm1, %%ecx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $19, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 28(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"shrdl $12, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 32(%%eax)           ;\n"
		"shrl $6, %%edx                  ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"xorl %%ecx, %%ecx               ;\n"
		"movl %%edx, 36(%%eax)           ;\n"
		"movl %%ecx, 40(%%eax)           ;\n"
		"movl %%ecx, 44(%%eax)           ;\n"

		/* t2d */
		"movl %0, %%eax                  ;\n"
		"movd %%eax, %%xmm6              ;\n"
		"pshufd $0x00, %%xmm6, %%xmm6    ;\n"
		"pxor %%xmm0, %%xmm0             ;\n"
		"pxor %%xmm1, %%xmm1             ;\n"

		/* 0 */
		"movl $0, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"pxor %%xmm0, %%xmm0             ;\n"
		"pxor %%xmm1, %%xmm1             ;\n"

		/* 1 */
		"movl $1, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 64(%1), %%xmm3           ;\n"
		"movdqa 80(%1), %%xmm4           ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 2 */
		"movl $2, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 160(%1), %%xmm3          ;\n"
		"movdqa 176(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 3 */
		"movl $3, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 256(%1), %%xmm3          ;\n"
		"movdqa 272(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 4 */
		"movl $4, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 352(%1), %%xmm3          ;\n"
		"movdqa 368(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 5 */
		"movl $5, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 448(%1), %%xmm3          ;\n"
		"movdqa 464(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 6 */
		"movl $6, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 544(%1), %%xmm3          ;\n"
		"movdqa 560(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 7 */
		"movl $7, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 640(%1), %%xmm3          ;\n"
		"movdqa 656(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* 8 */
		"movl $8, %%eax                  ;\n"
		"movd %%eax, %%xmm7              ;\n"
		"pshufd $0x00, %%xmm7, %%xmm7    ;\n"
		"pcmpeqd %%xmm6, %%xmm7          ;\n"
		"movdqa 736(%1), %%xmm3          ;\n"
		"movdqa 752(%1), %%xmm4          ;\n"
		"pand %%xmm7, %%xmm3             ;\n"
		"pand %%xmm7, %%xmm4             ;\n"
		"por %%xmm3, %%xmm0              ;\n"
		"por %%xmm4, %%xmm1              ;\n"

		/* sspidere t2d */
		"movl %2, %%eax                  ;\n"
		"addl $96, %%eax                 ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"movl %%ecx, %%edx               ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 0(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $26, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 4(%%eax)            ;\n"
		"movd %%xmm0, %%edx              ;\n"
		"pshufd $0x39, %%xmm0, %%xmm0    ;\n"
		"shrdl $19, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 8(%%eax)            ;\n"
		"movd %%xmm0, %%ecx              ;\n"
		"shrdl $13, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 12(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrl $6, %%ecx                  ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 16(%%eax)           ;\n"
		"movl %%edx, %%ecx               ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 20(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $25, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 24(%%eax)           ;\n"
		"movd %%xmm1, %%ecx              ;\n"
		"pshufd $0x39, %%xmm1, %%xmm1    ;\n"
		"shrdl $19, %%ecx, %%edx         ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"movl %%edx, 28(%%eax)           ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"movd %%xmm1, %%edx              ;\n"
		"shrdl $12, %%edx, %%ecx         ;\n"
		"andl $0x3ffffff, %%ecx          ;\n"
		"movl %%ecx, 32(%%eax)           ;\n"
		"shrl $6, %%edx                  ;\n"
		"andl $0x1ffffff, %%edx          ;\n"
		"xorl %%ecx, %%ecx               ;\n"
		"movl %%edx, 36(%%eax)           ;\n"
		"movl %%ecx, 40(%%eax)           ;\n"
		"movl %%ecx, 44(%%eax)           ;\n"
		"movdqa 0(%%eax), %%xmm0         ;\n"
		"movdqa 16(%%eax), %%xmm1        ;\n"
		"movdqa 32(%%eax), %%xmm2        ;\n"

		/* conditionally negate t2d */

		/* set up 2p in to 3/4 */
		"movl $0x7ffffda, %%ecx          ;\n"
		"movl $0x3fffffe, %%edx          ;\n"
		"movd %%ecx, %%xmm3              ;\n"
		"movd %%edx, %%xmm5              ;\n"
		"movl $0x7fffffe, %%ecx          ;\n"
		"movd %%ecx, %%xmm4              ;\n"
		"punpckldq %%xmm5, %%xmm3        ;\n"
		"punpckldq %%xmm5, %%xmm4        ;\n"
		"punpcklqdq %%xmm4, %%xmm3       ;\n"
		"movdqa %%xmm4, %%xmm5           ;\n"
		"punpcklqdq %%xmm4, %%xmm4       ;\n"

		/* subtract and conditionally move */
		"movl %3, %%ecx                  ;\n"
		"sub $1, %%ecx                   ;\n"
		"movd %%ecx, %%xmm6              ;\n"
		"pshufd $0x00, %%xmm6, %%xmm6    ;\n"
		"movdqa %%xmm6, %%xmm7           ;\n"
		"psubd %%xmm0, %%xmm3            ;\n"
		"psubd %%xmm1, %%xmm4            ;\n"
		"psubd %%xmm2, %%xmm5            ;\n"
		"pand %%xmm6, %%xmm0             ;\n"
		"pand %%xmm6, %%xmm1             ;\n"
		"pand %%xmm6, %%xmm2             ;\n"
		"pandn %%xmm3, %%xmm6            ;\n"
		"movdqa %%xmm7, %%xmm3           ;\n"
		"pandn %%xmm4, %%xmm7            ;\n"
		"pandn %%xmm5, %%xmm3            ;\n"
		"por %%xmm6, %%xmm0              ;\n"
		"por %%xmm7, %%xmm1              ;\n"
		"por %%xmm3, %%xmm2              ;\n"

		/* sspidere */
		"movdqa %%xmm0, 0(%%eax)         ;\n"
		"movdqa %%xmm1, 16(%%eax)        ;\n"
		"movdqa %%xmm2, 32(%%eax)        ;\n"
		:
		: "m"(u), "r"(&table[pos * 8]), "m"(t), "m"(sign) /* %0 = u, %1 = table, %2 = t, %3 = sign */
		: "%eax", "%ecx", "%edx", "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "cc", "memory"
	);
}

#endif /* defined(ED25519_GCC_32BIT_SSE_CHOOSE) */

