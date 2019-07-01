#include "ago_internal.h"
#include "mips_internal.h"

vx_uint32 dataConvertU1ToU8_4bytes[16] = { 0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
									       0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
									       0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
									       0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF };

int HafCpu_And_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16u8 pixels1, pixels2, pixels11, pixels22;

	int alignedWidth = dstWidth & ~31;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16i8 *) pSrcImage1;
		pLocalSrc2_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);
			pixels11 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels22 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1 = __builtin_msa_and_v(pixels1, pixels2);
			pixels11 = __builtin_msa_and_v(pixels11, pixels22);

			__builtin_msa_st_b((v16i8) pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels11, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++ & *pLocalSrc2++;
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++ & *pLocalSrc2++;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_And_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2;
	vx_int32 *pLocalDst;
	vx_uint8 pixels1, pixels2;

	for (int height = 0; height < (int) dstHeight; height++)
	{
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int32 *) pDstImage;

		for (int width = 0; width < (int) dstWidth; width += 8)
		{
			pixels1 = *pLocalSrc1++;
			pixels2 = *pLocalSrc2++;
			pixels1 = pixels1 & pixels2;

			// U1 to U8
			*pLocalDst++ = dataConvertU1ToU8_4bytes[pixels1 & 0xF];
			pixels1 >>= 4;
			*pLocalDst++ = dataConvertU1ToU8_4bytes[pixels1 & 0xF];
		}
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

/* The function assumes that the widths are a multiple of 8 pixels*/
int HafCpu_And_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
	vx_int16 pixels1, pixels2;

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;

	for (int height = 0; height < (int) dstHeight; height++)
	{
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = *pLocalSrc1++;
			pixels2 = *pLocalSrc2++;
			pixels1 = pixels1 & pixels2;
			*pLocalDst++ = pixels1;
		}

		if (postfixWidth)
		{
			*((vx_uint8 *) pLocalDst) = *((vx_uint8 *) pLocalSrc1) & *((vx_uint8 *) pLocalSrc2);
		}
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Not_U8_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	vx_uint8 *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc_msa, *pLocalDst_msa;

	int alignedWidth = (int) (dstWidth & ~63);
	int postfixWidth = dstWidth - alignedWidth;
#endif
	int height = (int) dstHeight;
	while (height > 0)
	{
#if ENABLE_MSA
		pLocalSrc_msa = (v16i8 *) pSrcImage;
		pLocalDst_msa = (v16i8 *) pDstImage;
		int width = alignedWidth >> 6;

		while (width > 0)
		{
			v16i8 pixels0 = __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			v16i8 pixels1 = __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			v16i8 pixels2 = __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			v16i8 pixels3 = __builtin_msa_ld_b(pLocalSrc_msa++, 0);

			pixels0 = (v16i8) __builtin_msa_xori_b((v16u8) pixels0, 255);
			pixels1 = (v16i8) __builtin_msa_xori_b((v16u8) pixels1, 255);
			pixels2 = (v16i8) __builtin_msa_xori_b((v16u8) pixels2, 255);
			pixels3 = (v16i8) __builtin_msa_xori_b((v16u8) pixels3, 255);

			__builtin_msa_st_b((v16i8) pixels0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels2, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) pLocalDst_msa++, 0);

			width--;
		}

		width = postfixWidth;
		pLocalSrc = (vx_uint8 *) pLocalSrc_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		while (width > 0)
		{
			*pLocalDst++ = ~(*pLocalSrc++);
			width--;
		}
#else
		pLocalSrc = (vx_uint8 *) pSrcImage;
		pLocalDst = (vx_uint8 *) pDstImage;
		int width = dstWidth;

		while (width > 0)
		{
			*pLocalDst++ = ~(*pLocalSrc++);
			width--;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Or_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
    v16i8 pixels1, pixels2, pixels11, pixels22;

	int alignedWidth = dstWidth & ~31;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16i8 *) pSrcImage1;
		pLocalSrc2_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_b(pLocalSrc2_msa++, 0);
			pixels11 = __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels22 = __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1 = (v16i8) __builtin_msa_or_v((v16u8) pixels1, (v16u8) pixels2);
			pixels11 = (v16i8) __builtin_msa_or_v((v16u8) pixels11, (v16u8) pixels22);

			__builtin_msa_st_b(pixels1, pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels11, pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++ | *pLocalSrc2++;
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
        pLocalSrc2 = (vx_uint8 *) pSrcImage2;
        pLocalDst = (vx_uint8 *) pDstImage;

        for (int width = 0; width < dstWidth; width++)
        {
            *pLocalDst++ = *pLocalSrc1++ | *pLocalSrc2++;
        }
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Xor_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16i8 pixels1, pixels2, pixels11, pixels22;

	int alignedWidth = dstWidth & ~31;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16i8 *) pSrcImage1;
		pLocalSrc2_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_b(pLocalSrc2_msa++, 0);
			pixels11 = __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels22 = __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1 = (v16i8) __builtin_msa_xor_v((v16u8) pixels1, (v16u8) pixels2);
			pixels11 = (v16i8) __builtin_msa_xor_v((v16u8) pixels11, (v16u8) pixels22);

			__builtin_msa_st_b(pixels1, pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels11, pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc1++ ^ *pLocalSrc2++;
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
        pLocalSrc2 = (vx_uint8 *) pSrcImage2;
        pLocalDst = (vx_uint8 *) pDstImage;

        for (int width = 0; width < dstWidth; width++)
        {
            *pLocalDst++ = *pLocalSrc1++ ^ *pLocalSrc2++;
        }
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}
