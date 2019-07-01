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

		vx_size prefixBytes = intptr_t(pDstBuf) & 15;
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
#if 0 //ENABLE_MSA
	__m128i val = _mm_set1_epi16((short)value);
	__m128i * buf = (__m128i *) pDstBuf;
	__m128i * buf_end = buf + (count >> 3);
	for (; buf != buf_end; buf++)
		_mm_store_si128(buf, val);
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
#if 0 //ENABLE_MSA
	vx_uint32 valArray[4] = {value, value, value, value};
	v4u32 val = (v4u32) __builtin_msa_ld_w((void *) &valArray, 0);
	v4u32 *buf = (v4u32*) pDstBuf;
	v4u32 *buf_end = buf + (count >> 2);
	for (; buf != buf_end; buf++)
		__builtin_msa_st_w((v4i32) val, (void*) buf, 0);
#else
	vx_uint8 *buf = (vx_uint8 *) pDstBuf;
	vx_uint8 *buf_end = buf + count;
	for (; buf != buf_end; buf++)
		*buf = value;
#endif
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
	int postfixWidth = (int)dstWidth - alignedWidth;

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
			*pLocalDst0++ = *pSrcImage++;
			*pLocalDst1++ = *pSrcImage++;
			*pLocalDst2++ = *pSrcImage++;
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
			*pLocalDst0++ = *pSrcImage++;
			*pLocalDst1++ = *pSrcImage++;
			*pLocalDst2++ = *pSrcImage++;
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
	int alignedWidth = dstWidth & ~15;
	int alignedTest = alignedWidth >> 4;
	int postfixWidth = (int) dstWidth - alignedWidth;

#if ENABLE_MSA
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
	unsigned char *pLocalSrc, *pLocalDst;
	int height = (int) dstHeight;

#if ENABLE_MSA
	v16u8 r0, r1;
	v16u8 *pLocalSrc_msa, *pLocalDst_msa;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 31;					// 32 pixels processed at a time in MSA loop
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;

		for (int x = 0; x < prefixWidth; x++)
			*pLocalDst++ = *pLocalSrc++;

		int width = alignedWidth >> 5;									// 32 pixels copied at a time
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
		int width = (int) dstWidth;
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
