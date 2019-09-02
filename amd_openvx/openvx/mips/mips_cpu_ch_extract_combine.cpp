#include "ago_internal.h"
#include "mips_internal.h"

DECL_ALIGN(16) unsigned char dataChannelExtract[16 * 29] ATTR_ALIGN(16) = {
	  0,   2,   4,	 6,   8,  10,  12,  14, 255, 255, 255, 255, 255, 255, 255, 255,		// Lower 8 bytes pos0 for U8_U16
	255, 255, 255, 255, 255, 255, 255, 255,   0,   2,   4,	 6,   8,  10,  12,  14,		// Upper 8 bytes pos0 for U8_U16
	  1,   3,   5,	 7,   9,  11,  13,  15, 255, 255, 255, 255, 255, 255, 255, 255,		// Lower 8 bytes pos1 for U8_U16
	255, 255, 255, 255, 255, 255, 255, 255,   1,   3,   5,	 7,   9,  11,  13,  15,		// Upper 8 bytes pos1 for U8_U16
	  0,   3,   6,	 9,  12,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Lower 6 bytes pos0 for U8_U24
	255, 255, 255, 255, 255, 255,	2,   5,   8,  11,  14, 255, 255, 255, 255, 255,		// Mid	 5 bytes pos0 for U8_U24
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	 1,   4,   7,  10,  13,		// Upper 5 bytes pos0 for U8_U24
	  1,   4,   7,	10,  13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Lower 5 bytes pos1 for U8_U24
	255, 255, 255, 255, 255,   0,	3,   6,   9,  12,  15, 255, 255, 255, 255, 255,		// Mid	 6 bytes pos1 for U8_U24
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	 2,   5,   8,  11,  14,		// Upper 5 bytes pos1 for U8_U24
	  2,   5,   8,	11,  14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Lower 5 bytes pos2 for U8_U24
	255, 255, 255, 255, 255,   1,	4,   7,  10,  13, 255, 255, 255, 255, 255, 255,		// Mid	 5 bytes pos2 for U8_U24
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,	 3,   6,   9,  12,  15,		// Upper 6 bytes pos2 for U8_U24
	  0,   4,   8,	12, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Low	 4 bytes pos0 for U8_U32
	255, 255, 255, 255,   0,   4,	8,  12, 255, 255, 255, 255, 255, 255, 255, 255,		// Next  4 bytes pos0 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255,   0,   4,   8,	12, 255, 255, 255, 255,		// Next  4 bytes pos0 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,   4,	8,  12,		// Upper 4 bytes pos0 for U8_U32
	  1,   5,   9,	13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Low	 4 bytes pos1 for U8_U32
	255, 255, 255, 255,   1,   5,	9,  13, 255, 255, 255, 255, 255, 255, 255, 255,		// Next  4 bytes pos1 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255,   1,   5,   9,	13, 255, 255, 255, 255,		// Next  4 bytes pos1 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   1,   5,	9,  13,		// Upper 4 bytes pos1 for U8_U32
	  2,   6,  10,	14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Low	 4 bytes pos2 for U8_U32
	255, 255, 255, 255,   2,   6,  10,  14, 255, 255, 255, 255, 255, 255, 255, 255,		// Next  4 bytes pos2 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255,   2,   6,  10,	14, 255, 255, 255, 255,		// Next  4 bytes pos2 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   2,   6,  10,  14,		// Upper 4 bytes pos2 for U8_U32
	  3,   7,  11,	15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// Low	 4 bytes pos3 for U8_U32
	255, 255, 255, 255,   3,   7,  11,  15, 255, 255, 255, 255, 255, 255, 255, 255,		// Next  4 bytes pos3 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255,   3,   7,  11,	15, 255, 255, 255, 255,		// Next  4 bytes pos3 for U8_U32
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   3,   7,  11,  15		// Upper 4 bytes pos3 for U8_U32
};

DECL_ALIGN(16) unsigned char dataChannelCombine[16 * 15] ATTR_ALIGN(16) = {
	  0, 255, 255,	 1, 255, 255,	2, 255, 255,   3, 255, 255,   4, 255, 255,   5,		// R into first  16 bytes for RGB
	255, 255,   6, 255, 255,   7, 255, 255,   8, 255, 255,	 9, 255, 255,  10, 255,		// R into second 16 bytes for RGB
	255,  11, 255, 255,  12, 255, 255,  13, 255, 255,  14, 255, 255,  15, 255, 255,		// R into third  16 bytes for RGB
	255,   0, 255, 255,   1, 255, 255,   2, 255, 255,   3, 255, 255,   4, 255, 255,		// G into first  16 bytes for RGB
	  5, 255, 255,	 6, 255, 255,	7, 255, 255,   8, 255, 255,   9, 255, 255,  10,		// G into second 16 bytes for RGB
	255, 255,  11, 255, 255,  12, 255, 255,  13, 255, 255,	14, 255, 255,  15, 255,		// G into third  16 bytes for RGB
	255, 255,   0, 255, 255,   1, 255, 255,   2, 255, 255,	 3, 255, 255,	4, 255,		// B into first  16 bytes for RGB
	255,   5, 255, 255,   6, 255, 255,   7, 255, 255,   8, 255, 255,   9, 255, 255,		// B into second 16 bytes for RGB
	 10, 255, 255,	11, 255, 255,  12, 255, 255,  13, 255, 255,  14, 255, 255,  15,		// B into third  16 bytes for RGB
	255,   0, 255,	 1, 255,   2, 255,   3, 255,   4, 255,	 5, 255,   6, 255,   7,		// Y into UYVY
	  0, 255, 255, 255,   1, 255, 255, 255,   2, 255, 255, 255,   3, 255, 255, 255,		// U into UYVY
	255, 255,   0, 255, 255, 255,	1, 255, 255, 255,   2, 255, 255, 255,	3, 255,		// V into UYVY
	  0, 255,   1, 255,   2, 255,	3, 255,   4, 255,   5, 255,   6, 255,	7, 255,		// Y into YUYV
	255,   0, 255, 255, 255,   1, 255, 255, 255,   2, 255, 255, 255,   3, 255, 255,		// U into YUYV
	255, 255, 255,	 0, 255, 255, 255,   1, 255, 255, 255,	 2, 255, 255, 255,   3,		// V into YUYV
};

/*
Benchmarks shown that memcpy is faster then MSA implementation for most usecases.
MSA implementation could be used and tuned to suit specific usecase,
so we are providing it as well.
*/
int HafCpu_BinaryCopy_U8_U8
	(
		vx_size		size,
		vx_uint8	  * pDstBuf,
		vx_uint8	  * pSrcBuf
	)
{
	memcpy(pDstBuf, pSrcBuf, size);
	return AGO_SUCCESS;
}
/*
int HafCpu_BinaryCopy_U8_U8
	(
		vx_size		size,
		vx_uint8	  * pDstBuf,
		vx_uint8	  * pSrcBuf
	)
{
	if ((intptr_t(pSrcBuf) & 15) | (intptr_t(pDstBuf) & 15))
	{
		memcpy(pDstBuf, pSrcBuf, size);
	}
	else
	{
#if ENABLE_MSA
		v16u8 *src = (v16u8 *) pSrcBuf;
		v16u8 *dst = (v16u8 *) pDstBuf;
		v16u8 r0, r1, r2, r3;

		vx_size sizeAligned = size & ~63;

		for (unsigned int i = 0; i < sizeAligned; i += 64)
		{
			r0 = (v16u8) __builtin_msa_ld_b((void *) src++, 0);
			r1 = (v16u8) __builtin_msa_ld_b((void *) src++, 0);
			r2 = (v16u8) __builtin_msa_ld_b((void *) src++, 0);
			r3 = (v16u8) __builtin_msa_ld_b((void *) src++, 0);
			__builtin_msa_st_b((v16i8) r0, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) r1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) r2, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) r3, (void *) dst++, 0);
		}
		for (vx_size i = sizeAligned; i < size; i++)
		{
			pDstBuf[i] = pSrcBuf[i];
		}
#else
		memcpy(pDstBuf, pSrcBuf, size);
#endif
	}
	return AGO_SUCCESS;
}
*/

int HafCpu_ChannelCombine_U16_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage0,
		vx_uint32     srcImage0StrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes
	)
{
	int height = (int) dstHeight;
	unsigned char *pLocalSrc0, *pLocalSrc1, *pLocalDst;

#if ENABLE_MSA
	int width;
	v16u8 r0, r1, resultL, resultH;
	v16u8 *pLocalSrc0_msa, *pLocalSrc1_msa, *pLocalDst_msa;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 32 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 31;
#endif

	while (height)
	{
		pLocalSrc0 = (unsigned char *) pSrcImage0;
		pLocalSrc1 = (unsigned char *) pSrcImage1;
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
		}

		// 16 byte pairs copied into dst at once
		width = (int) (dstWidth >> 4);
		pLocalSrc0_msa = (v16u8 *) pLocalSrc0;
		pLocalSrc1_msa = (v16u8 *) pLocalSrc1;
		pLocalDst_msa = (v16u8 *) pLocalDst;

		while (width)
		{
			r0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc0_msa++, 0);
			r1 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			resultL = (v16u8) __builtin_msa_ilvr_b((v16i8) r1, (v16i8) r0);
			resultH = (v16u8) __builtin_msa_ilvl_b((v16i8) r1, (v16i8) r0);
			__builtin_msa_st_b((v16i8) resultL, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) resultH, (void *) pLocalDst_msa++, 0);
			width--;
		}

		pLocalSrc0 = (unsigned char *) pLocalSrc0_msa;
		pLocalSrc1 = (unsigned char *) pLocalSrc1_msa;
		pLocalDst = (unsigned char *) pLocalDst_msa;

		for (int x = 0; x < postfixWidth; x++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
		}
#else	// C
		for (int x = 0; x < dstWidth; x++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
		}
#endif
		pSrcImage0 += srcImage0StrideInBytes;
		pSrcImage1 += srcImage1StrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelCombine_U24_U8U8U8_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage0,
		vx_uint32     srcImage0StrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16u8 *tbl = (v16u8 *) dataChannelCombine;
	v16u8 r, g, b, result1, result2, result3;
	v16i8 maskR1 = __builtin_msa_ld_b((void *) tbl, 0);
	v16i8 maskR2 = __builtin_msa_ld_b((void *) (tbl + 1), 0);
	v16i8 maskR3 = __builtin_msa_ld_b((void *) (tbl + 2), 0);
	v16i8 maskG1 = __builtin_msa_ld_b((void *) (tbl + 3), 0);
	v16i8 maskG2 = __builtin_msa_ld_b((void *) (tbl + 4), 0);
	v16i8 maskG3 = __builtin_msa_ld_b((void *) (tbl + 5), 0);
	v16i8 maskB1 = __builtin_msa_ld_b((void *) (tbl + 6), 0);
	v16i8 maskB2 = __builtin_msa_ld_b((void *) (tbl + 7), 0);
	v16i8 maskB3 = __builtin_msa_ld_b((void *) (tbl + 8), 0);
#endif

	int height = (int) dstHeight;
	while (height)
	{
		vx_uint8 * pLocalSrc0 = pSrcImage0;
		vx_uint8 * pLocalSrc1 = pSrcImage1;
		vx_uint8 * pLocalSrc2 = pSrcImage2;
		vx_uint8 * pLocalDst = pDstImage;

	#if ENABLE_MSA
		int width = (int) (dstWidth >> 4);
		while (width)
		{

			r = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc0, 0);
			g = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc1, 0);
			b = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc2, 0);

			result1 = (v16u8) __builtin_msa_vshf_b(maskR1, (v16i8) r, (v16i8) r);					// Extract and place R in first 16 bytes
			result2 = (v16u8) __builtin_msa_vshf_b(maskG1, (v16i8) g, (v16i8) g);					// Extract and place G in first 16 bytes
			result3 = (v16u8) __builtin_msa_vshf_b(maskB1, (v16i8) b, (v16i8) b);					// Extract and place B in first 16 bytes
			result1 = __builtin_msa_or_v(result1, result2);
			result1 = __builtin_msa_or_v(result1, result3);

			result2 = (v16u8) __builtin_msa_vshf_b(maskR2, (v16i8) r, (v16i8) r);					// Extract and place R in second 16 bytes
			result3 = (v16u8) __builtin_msa_vshf_b(maskG2, (v16i8) g, (v16i8) g);					// Extract and place G in second 16 bytes
			result2 = __builtin_msa_or_v(result2, result3);
			result3 = (v16u8) __builtin_msa_vshf_b(maskB2, (v16i8) b, (v16i8) b);					// Extract and place B in second 16 bytes
			result2 = __builtin_msa_or_v(result2, result3);

			result3 = (v16u8) __builtin_msa_vshf_b(maskR3, (v16i8) r, (v16i8) r);					// Extract and place R in third 16 bytes
			r = (v16u8) __builtin_msa_vshf_b(maskG3, (v16i8) g, (v16i8) g);						// Extract and place G in third 16 bytes
			g = (v16u8) __builtin_msa_vshf_b(maskB3, (v16i8) b, (v16i8) b);						// Extract and place B in third 16 bytes
			result3 = __builtin_msa_or_v(result3, r);
			result3 = __builtin_msa_or_v(result3, g);

			__builtin_msa_st_b((v16i8) result1, (void *) pLocalDst, 0);
			__builtin_msa_st_b((v16i8) result2, (void *) (pLocalDst + 16), 0);
			__builtin_msa_st_b((v16i8) result3, (void *) (pLocalDst + 32), 0);

			width--;
			pLocalSrc0 += 16;
			pLocalSrc1 += 16;
			pLocalSrc2 += 16;
			pLocalDst += 48;
		}

		for (width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
			*pLocalDst++ = *pLocalSrc2++;
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
			*pLocalDst++ = *pLocalSrc2++;
		}
#endif
		pSrcImage0 += srcImage0StrideInBytes;
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelCombine_U32_U8U8U8_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage0,
		vx_uint32     srcImage0StrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~31;
	int postfixWidth = (int) dstWidth - alignedWidth;
	v16i8 Y0, Y1, U, V, temp;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc0 = pSrcImage0;
		vx_uint8 * pLocalSrc1 = pSrcImage1;
		vx_uint8 * pLocalSrc2 = pSrcImage2;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 5); width++)
		{
			Y0 = __builtin_msa_ld_b((void *) pLocalSrc0, 0);
			Y1 = __builtin_msa_ld_b((void *) (pLocalSrc0 + 16), 0);
			U = __builtin_msa_ld_b((void *) pLocalSrc1, 0);
			V = __builtin_msa_ld_b((void *) pLocalSrc2, 0);

			temp = __builtin_msa_pckev_b(Y1, Y0);
			Y0 = __builtin_msa_pckod_b(Y1, Y0);

			Y1 = __builtin_msa_ilvr_b(V, U);
			U = __builtin_msa_ilvl_b(V, U);
			V = __builtin_msa_ilvr_b(Y0, temp);
			temp = __builtin_msa_ilvl_b(Y0, temp);
			Y0 = __builtin_msa_ilvr_b(V, Y1);
			Y1 = __builtin_msa_ilvl_b(V, Y1);

			__builtin_msa_st_b( Y0, (void *) pLocalDst, 0);
			__builtin_msa_st_b( Y1, (void *) (pLocalDst + 16), 0);

			Y0 = __builtin_msa_ilvr_b(temp, U);
			U = __builtin_msa_ilvl_b(temp, U);

			__builtin_msa_st_b( Y0, (void *) (pLocalDst + 32), 0);
			__builtin_msa_st_b( U, (void *) (pLocalDst + 48), 0);

			pLocalSrc0 += 32;
			pLocalSrc1 += 16;
			pLocalSrc2 += 16;
			pLocalDst += 64;
		}
		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++;			// U
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc2++;			// V
			*pLocalDst++ = *pLocalSrc0++;			// Y
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++;			// U
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc2++;			// V
			*pLocalDst++ = *pLocalSrc0++;			// Y
		}
#endif
		pSrcImage0 += srcImage0StrideInBytes;
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelCombine_U32_U8U8U8_YUYV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage0,
		vx_uint32     srcImage0StrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~31;
	int postfixWidth = (int) dstWidth - alignedWidth;
	v16i8 Y0, Y1, U, V, temp;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc0 = pSrcImage0;
		vx_uint8 * pLocalSrc1 = pSrcImage1;
		vx_uint8 * pLocalSrc2 = pSrcImage2;
		vx_uint8 * pLocalDst = pDstImage;
#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 5); width++)
		{
			Y0 = __builtin_msa_ld_b((void *) pLocalSrc0, 0);
			Y1 = __builtin_msa_ld_b((void *) (pLocalSrc0 + 16), 0);
			V = __builtin_msa_ld_b((void *) pLocalSrc1, 0);
			U = __builtin_msa_ld_b((void *) pLocalSrc2, 0);

			temp = __builtin_msa_pckev_b(Y1, Y0);
			Y0 = __builtin_msa_pckod_b(Y1, Y0);

			Y1 = __builtin_msa_ilvr_b(Y0, temp);
			temp = __builtin_msa_ilvl_b(Y0, temp);

			Y0 = __builtin_msa_ilvr_b(U, V);
			V = __builtin_msa_ilvl_b(U, V);

			U = __builtin_msa_ilvr_b(Y0, Y1);
			Y1 = __builtin_msa_ilvl_b(Y0, Y1);

			__builtin_msa_st_b( U, (void *) pLocalDst, 0);
			__builtin_msa_st_b( Y1, (void *) (pLocalDst + 16), 0);

			Y0 = __builtin_msa_ilvr_b(V, temp);
			temp = __builtin_msa_ilvl_b(V, temp);

			__builtin_msa_st_b( Y0, (void *) (pLocalDst + 32), 0);
			__builtin_msa_st_b( temp, (void *) (pLocalDst + 48), 0);

			pLocalSrc0 += 32;
			pLocalSrc1 += 16;
			pLocalSrc2 += 16;
			pLocalDst += 64;
		}
		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc1++;			// U
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc2++;			// V
		}
#else
		for (int width = 0; width < dstWidth-8; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc1++;			// U
			*pLocalDst++ = *pLocalSrc0++;			// Y
			*pLocalDst++ = *pLocalSrc2++;			// V
		}
#endif
		pSrcImage0 += srcImage0StrideInBytes;
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelCombine_U32_U8U8U8U8_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage0,
		vx_uint32     srcImage0StrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_uint8    * pSrcImage3,
		vx_uint32     srcImage3StrideInBytes
	)
{
	int width;
	int height = (int) dstHeight;

#if ENABLE_MSA
	v16u8 r, g, b, x, pixels0, pixels1, pixels2;

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;
#endif

	while (height)
	{
		vx_uint8 * pLocalSrc0 = pSrcImage0;
		vx_uint8 * pLocalSrc1 = pSrcImage1;
		vx_uint8 * pLocalSrc2 = pSrcImage2;
		vx_uint8 * pLocalSrc3 = pSrcImage3;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		// Inner loop processess 16 pixels at a time
		width = (int) (dstWidth >> 4);
		while (width)
		{
			r = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc0, 0);
			g = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc1, 0);
			b = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc2, 0);
			x = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc3, 0);

			// r0 g0 r1 g1 r2 g2 r3 g3 r4 g4 r5 g5 r6 g6 r7 g7
			pixels0 = (v16u8) __builtin_msa_ilvr_b((v16i8) g, (v16i8) r);
			// b0 x0 b1 x1 b2 x2 b3 x3 b4 x4 b5 x5 b6 x6 b7 x7
			pixels1 = (v16u8) __builtin_msa_ilvr_b((v16i8) x, (v16i8) b);
			// r0 g0 b0 x0 r1 g1 b1 x1 r2 g2 b2 x2 r3 g3 b3 x3
			pixels2 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) pixels0);
			__builtin_msa_st_b((v16i8) pixels2, (void *) pLocalDst, 0);
			pLocalDst += 16;
			// r4 g4 b4 x4 r5 g5 b5 x5 r6 g6 b6 x6 r7 g7 b7 x7
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) pixels0);
			__builtin_msa_st_b((v16i8) pixels2, (void *) pLocalDst, 0);
			pLocalDst += 16;

			// r8 g8 r9 g9 r10 g10 r11 g11 r12 g12 r13 g13 r14 g14 r15 g15
			pixels0 = (v16u8) __builtin_msa_ilvl_b((v16i8) g, (v16i8) r);
			// b8 x8 b9 x9 b10 x10 b11 x11 b12 x12 b13 x13 b14 x14 b15 x15
			pixels1 = (v16u8) __builtin_msa_ilvl_b((v16i8) x, (v16i8) b);
			// r8 g8 b8 x8 r9 g9 b9 x9 r10 g10 b10 x10 r11 g11 b11 x11
			pixels2 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) pixels0);
			__builtin_msa_st_b((v16i8) pixels2, (void *) pLocalDst, 0);
			pLocalDst += 16;
			// r12 g12 b12 x12 r13 g13 b13 x13 r14 g14 b14 x14 r15 g15 b15 x15
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) pixels0);
			__builtin_msa_st_b((v16i8) pixels2, (void *) pLocalDst, 0);
			pLocalDst += 16;

			width--;
			pLocalSrc0 += 16;
			pLocalSrc1 += 16;
			pLocalSrc2 += 16;
			pLocalSrc3 += 16;
		}

		for (width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
			*pLocalDst++ = *pLocalSrc2++;
			*pLocalDst++ = *pLocalSrc3++;
		}
#else	// C
		for (width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc0++;
			*pLocalDst++ = *pLocalSrc1++;
			*pLocalDst++ = *pLocalSrc2++;
			*pLocalDst++ = *pLocalSrc3++;
		}
#endif
		pSrcImage0 += srcImage0StrideInBytes;
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pSrcImage3 += srcImage3StrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8U8U8_U24
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage0,
		vx_uint8    * pDstImage1,
		vx_uint8    * pDstImage2,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	v16u8 *tbl = (v16u8 *) dataChannelExtract;
	v16u8 pixels0, pixels1, pixels2, pixels_R, pixels_G;

	v16i8 mask_r0 = __builtin_msa_ld_b((void *) (tbl + 4), 0);
	v16i8 mask_r1 = __builtin_msa_ld_b((void *) (tbl + 5), 0);
	v16i8 mask_r2 = __builtin_msa_ld_b((void *) (tbl + 6), 0);

	v16i8 mask_g0 = __builtin_msa_ld_b((void *) (tbl + 7), 0);
	v16i8 mask_g1 = __builtin_msa_ld_b((void *) (tbl + 8), 0);
	v16i8 mask_g2 = __builtin_msa_ld_b((void *) (tbl + 9), 0);

	v16i8 mask_b0 = __builtin_msa_ld_b((void *) (tbl + 10), 0);
	v16i8 mask_b1 = __builtin_msa_ld_b((void *) (tbl + 11), 0);
	v16i8 mask_b2 = __builtin_msa_ld_b((void *) (tbl + 12), 0);

	intptr_t prealignBytes = (intptr_t(pDstImage0) & intptr_t(pDstImage1) & intptr_t(pDstImage2)) & 15;

	int prefixWidth = (int) ((prealignBytes == 0) ? 0 : (16 - prealignBytes));
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	unsigned char *pLocalSrc, *pLocalDst0, *pLocalDst1, *pLocalDst2;

	int height = (int) dstHeight;
	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst0 = (unsigned char *) pDstImage0;
		pLocalDst1 = (unsigned char *) pDstImage1;
		pLocalDst2 = (unsigned char *) pDstImage2;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++)
		{
			*pLocalDst0++ = *pLocalSrc++;
			*pLocalDst1++ = *pLocalSrc++;
			*pLocalDst2++ = *pLocalSrc++;
		}

		// 16 bytes at a time
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			pixels1 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			pixels2 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);

			pixels_R = (v16u8) __builtin_msa_vshf_b(mask_r0, (v16i8) pixels0, (v16i8) pixels0);
			pixels_R = __builtin_msa_or_v(pixels_R, (v16u8) __builtin_msa_vshf_b(mask_r1, (v16i8) pixels1, (v16i8) pixels1));
			pixels_R = __builtin_msa_or_v(pixels_R, (v16u8) __builtin_msa_vshf_b(mask_r2, (v16i8) pixels2, (v16i8) pixels2));
			__builtin_msa_st_b((v16i8) pixels_R, (void *) pLocalDst0, 0);

			pixels_G = (v16u8) __builtin_msa_vshf_b(mask_g0, (v16i8) pixels0, (v16i8) pixels0);
			pixels_G = __builtin_msa_or_v(pixels_G, (v16u8) __builtin_msa_vshf_b(mask_g1, (v16i8) pixels1, (v16i8) pixels1));
			pixels_G = __builtin_msa_or_v(pixels_G, (v16u8) __builtin_msa_vshf_b(mask_g2, (v16i8) pixels2, (v16i8) pixels2));
			__builtin_msa_st_b((v16i8) pixels_G, (void *) pLocalDst1, 0);

			pixels0 = (v16u8) __builtin_msa_vshf_b(mask_b0, (v16i8) pixels0, (v16i8) pixels0);
			pixels0 = __builtin_msa_or_v(pixels0, (v16u8) __builtin_msa_vshf_b(mask_b1, (v16i8) pixels1, (v16i8) pixels1));
			pixels0 = __builtin_msa_or_v(pixels0, (v16u8) __builtin_msa_vshf_b(mask_b2, (v16i8) pixels2, (v16i8) pixels2));
			__builtin_msa_st_b((v16i8) pixels0, (void *) pLocalDst2, 0);

			pLocalSrc += 48;
			pLocalDst0 += 16;
			pLocalDst1 += 16;
			pLocalDst2 += 16;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++)
		{
			*pLocalDst0++ = *pLocalSrc++;
			*pLocalDst1++ = *pLocalSrc++;
			*pLocalDst2++ = *pLocalSrc++;
		}
#else
		for (int x = 0; x < dstWidth; x++)
		{
			*pLocalDst0++ = *pLocalSrc++;
			*pLocalDst1++ = *pLocalSrc++;
			*pLocalDst2++ = *pLocalSrc++;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage0 += dstImageStrideInBytes;
		pDstImage1 += dstImageStrideInBytes;
		pDstImage2 += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U16_Pos0
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int alignedTest = alignedWidth >> 4;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16u8 *tbl;
	v16u8 r0, r1;
	v16u8 mask1, mask2;

	if(alignedTest > 0)
	{
		tbl = (v16u8 *) dataChannelExtract;
		mask1 = (v16u8) __builtin_msa_ld_b((void *) (tbl), 0);
		mask2 = (v16u8) __builtin_msa_ld_b((void *) (tbl + 1), 0);
	}
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 *pLocalSrc = pSrcImage;
		vx_uint8 *pLocalDst = pDstImage;
#if ENABLE_MSA
		if(alignedTest > 0)
		{
			for (int width = 0; width < alignedTest; width++)
			{
				r0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
				r1 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
				r0 = (v16u8) __builtin_msa_vshf_b((v16i8) mask1, (v16i8) r0, (v16i8) r0);
				r1 = (v16u8) __builtin_msa_vshf_b((v16i8) mask2, (v16i8) r1, (v16i8) r1);
				r0 = (v16u8) __builtin_msa_or_v(r0, r1);
				__builtin_msa_st_b((v16i8) r0, (void *) pLocalDst, 0);

				pLocalSrc += 32;
				pLocalDst += 16;
			}
		}
		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc++;
			pLocalSrc++;
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc++;
			pLocalSrc++;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U16_Pos1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int alignedTest = alignedWidth >> 4;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 *pLocalSrc = pSrcImage;
		vx_uint8 *pLocalDst = pDstImage;
#if ENABLE_MSA
			for (int width = 0; width < alignedTest; width++)
			{
				r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
				r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
				r0 = __builtin_msa_pckod_b(r1, r0);
				__builtin_msa_st_b( r0, (void *) pLocalDst, 0);

				pLocalSrc += 32;
				pLocalDst += 16;
			}
		for (int width = 0; width < postfixWidth; width++)
		{
			pLocalSrc++;
			*pLocalDst++ = *pLocalSrc++;
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			pLocalSrc++;
			*pLocalDst++ = *pLocalSrc++;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U24_Pos0
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int width;
	int height = (int) dstHeight;
	unsigned char * pLocalSrc, *pLocalDst;

#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 4), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 5), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 6), 0);
#endif

	while (height)
	{
		pLocalSrc = pSrcImage;
		pLocalDst = pDstImage;
#if ENABLE_MSA
		width = alignedWidth >> 4;
		while (width)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);
			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			width--;
			pLocalSrc += 48;
			pLocalDst += 16;
		}
		width = postfixWidth;
		while (width)
		{
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 3;
			width--;
		}
#else	// C
		width = dstWidth;
		while (width)
		{
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 3;
			width--;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U24_Pos1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int width;
	int height = (int) dstHeight;

#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 7), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 8), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 9), 0);
#endif

	while (height)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		width = alignedWidth >> 4;
		while (width)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);

			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			width--;
			pLocalSrc += 48;
			pLocalDst += 16;
		}

		width = postfixWidth;
		while (width)
		{
			*pLocalDst++ = *++pLocalSrc;
			pLocalSrc += 2;
			width--;
		}
#else	// C
		width = dstWidth;
		while (width)
		{
			*pLocalDst++ = *++pLocalSrc;
			pLocalSrc += 2;
			width--;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U24_Pos2
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int width;
	int height = (int) dstHeight;

#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 10), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 11), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 12), 0);
#endif

	while (height)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		width = alignedWidth >> 4;
		while (width)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);

			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			width--;
			pLocalSrc += 48;
			pLocalDst += 16;
		}

		width = postfixWidth;
		while (width)
		{
			pLocalSrc += 2;
			*pLocalDst++ = *pLocalSrc++;
			width--;
		}
#else	// C
		width = dstWidth;
		while (width)
		{
			pLocalSrc += 2;
			*pLocalDst++ = *pLocalSrc++;
			width--;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U32_Pos0
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2, r3;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 13), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 14), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 15), 0);
	v16i8 mask4 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 16), 0);
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 4); width++)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);
			r3 = __builtin_msa_ld_b((void *) (pLocalSrc + 48), 0);

			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);
			r3 = __builtin_msa_vshf_b(mask4, r3, r3);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r3);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			pLocalSrc += 64;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 4;
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 4;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U32_Pos1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2, r3;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 17), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 18), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 19), 0);
	v16i8 mask4 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 20), 0);
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 4); width++)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);
			r3 = __builtin_msa_ld_b((void *) (pLocalSrc + 48), 0);

			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);
			r3 = __builtin_msa_vshf_b(mask4, r3, r3);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r3);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			pLocalSrc += 64;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *++pLocalSrc;
			pLocalSrc += 3;
		}
#else // C
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *++pLocalSrc;
			pLocalSrc += 3;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelExtract_U8_U32_Pos2
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 r0, r1, r2, r3;
	v16i8 mask1 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 21), 0);
	v16i8 mask2 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 22), 0);
	v16i8 mask3 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 23), 0);
	v16i8 mask4 = __builtin_msa_ld_b((void *) (((v16i8 *) &dataChannelExtract) + 24), 0);
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;
#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 4); width++)
		{
			r0 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			r1 = __builtin_msa_ld_b((void *) (pLocalSrc + 16), 0);
			r2 = __builtin_msa_ld_b((void *) (pLocalSrc + 32), 0);
			r3 = __builtin_msa_ld_b((void *) (pLocalSrc + 48), 0);

			r0 = __builtin_msa_vshf_b(mask1, r0, r0);
			r1 = __builtin_msa_vshf_b(mask2, r1, r1);
			r2 = __builtin_msa_vshf_b(mask3, r2, r2);
			r3 = __builtin_msa_vshf_b(mask4, r3, r3);

			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r1);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r2);
			r0 = (v16i8) __builtin_msa_or_v((v16u8) r0, (v16u8) r3);

			__builtin_msa_st_b(r0, (void *) pLocalDst, 0);

			pLocalSrc += 64;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			pLocalSrc += 2;
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 2;
		}
#else	// C
		for (int width = 0; width < dstWidth; width++)
		{
			pLocalSrc += 2;
			*pLocalDst++ = *pLocalSrc;
			pLocalSrc += 2;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ChannelCopy_U8_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int width;
	int height = (int) dstHeight;

#if ENABLE_MSA
	v16u8 r0, r1;
	unsigned char *pLocalSrc, *pLocalDst;
	v16u8 *pLocalSrc_msa, *pLocalDst_msa;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 32 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 31;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;

		for (int x = 0; x < prefixWidth; x++)
			*pLocalDst++ = *pLocalSrc++;

		// 32 pixels copied at a time
		width = alignedWidth >> 5;
		pLocalSrc_msa = (v16u8 *) pLocalSrc;
		pLocalDst_msa = (v16u8 *) pLocalDst;
		while (width)
		{
			r0 = (v16u8) __builtin_msa_ld_b((v16u8 *) pLocalSrc_msa++, 0);
			r1 = (v16u8) __builtin_msa_ld_b((v16u8 *) pLocalSrc_msa++, 0);

			__builtin_msa_st_b((v16i8) r0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) r1, (void *) pLocalDst_msa++, 0);

			width--;
		}

		pLocalSrc = (unsigned char *) pLocalSrc_msa;
		pLocalDst = (unsigned char *) pLocalDst_msa;
		for (int x = 0; x < postfixWidth; x++)
			*pLocalDst++ = *pLocalSrc++;

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
#else
	while (height)
	{
		unsigned char *pLocalSrc = (unsigned char *) pSrcImage;
		unsigned char *pLocalDst = (unsigned char *) pDstImage;
		width = (int) dstWidth;
		while (width)
		{
			*pLocalDst++ = *pLocalSrc++;
			width--;
		}

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
#endif
	return AGO_SUCCESS;
}

int HafCpu_MemSet_U32
	(
		vx_size       count,
		vx_uint32   * pDstBuf,
		vx_uint32     value
	)
{
#if ENABLE_MSA
	vx_uint32 valArray[4] = {value, value, value, value};
	v4u32 val = (v4u32) __builtin_msa_ld_w((void *) &valArray, 0);

	v4u32 *buf = (v4u32 *) pDstBuf;
	v4u32 *buf_end = buf + (count >> 2);
	for (; buf != buf_end; buf++)
		__builtin_msa_st_w((v4i32) val, (void *) buf, 0);
#else
	vx_uint32 *buf = (vx_uint32 *) pDstBuf;
	vx_uint32 *buf_end = buf + count;
	for (; buf != buf_end; buf++)
		*buf = value;
#endif
	return AGO_SUCCESS;
}

int HafCpu_MemSet_U16
	(
		vx_size       count,
		vx_uint16   * pDstBuf,
		vx_uint16     value
	)
{
#if ENABLE_MSA
	v8u16 val = (v8u16) __builtin_msa_fill_h(value);

	v8u16 *buf = (v8u16 *) pDstBuf;
	v8u16 *buf_end = buf + (count >> 3);

	for (; buf != buf_end; buf++)
		__builtin_msa_st_h((v8i16) val, (void*) buf, 0);
#else
	vx_uint16 *buf = (vx_uint16 *) pDstBuf;
	vx_uint16 *buf_end = buf + count;

	for (; buf != buf_end; buf++)
		*buf = value;
#endif
	return AGO_SUCCESS;
}

int HafCpu_MemSet_U8
	(
		vx_size      count,
		vx_uint8   * pDstBuf,
		vx_uint8     value
	)
{
	// For C implementation of this function, compiler will use memset.S,
	// and benchmarks shown that, on 64b platform, this is faster then MSA code.
#if ENABLE_MSA && __mips == 32
	v16u8 val = (v16u8) __builtin_msa_fill_b(value);

	v16u8 *buf = (v16u8 *) pDstBuf;
	v16u8 *buf_end = buf + (count >> 4);

	for (; buf != buf_end; buf++)
		__builtin_msa_st_b((v16i8) val, (void*) buf, 0);
#else
	vx_uint8 *buf = (vx_uint8 *) pDstBuf;
	vx_uint8 *buf_end = buf + count;

	for (; buf != buf_end; buf++)
		*buf = value;
#endif
	return AGO_SUCCESS;
}