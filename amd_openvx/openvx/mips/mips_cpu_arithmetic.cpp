#include "ago_internal.h"
#include "mips_internal.h"

int HafCpu_AbsDiff_U8_U8U8
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
	v16u8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16u8 pixels1H, pixels1L, pixels2H, pixels2L;
	v8u16 pixelsHu, pixelsLu;

	v16i8 zeromask = (v16i8) __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16u8 *) pSrcImage1;
		pLocalSrc2_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v16u8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2L = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1H = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels1L);
			pixels2H = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels2L);
			pixels1L = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels1L);
			pixels2L = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels2L);

			pixels1H = (v16u8) __builtin_msa_subv_h((v8i16) pixels1H, (v8i16) pixels2H);
			pixels1L = (v16u8) __builtin_msa_subv_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixelsHu = (v8u16) __builtin_msa_add_a_h((v8i16) pixels1H, (v8i16) zeromask);
			pixelsLu = (v8u16) __builtin_msa_add_a_h((v8i16) pixels1L, (v8i16) zeromask);

			pixelsLu = __builtin_msa_sat_u_h(pixelsLu, 15);
			pixelsHu = __builtin_msa_sat_u_h(pixelsHu, 15);
			pixels1L = (v16u8) __builtin_msa_pckev_b((v16i8) pixelsHu, (v16i8) pixelsLu);

			__builtin_msa_st_b((v16i8) pixels1L, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = (vx_uint8) abs((vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++));
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = (vx_uint8) abs((vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++));
		}

#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_AbsDiff_S16_S16S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v8i16 pixels1H, pixels1L, pixels2H, pixels2L;
	v4i32 pixels1Hw, pixels1Lw;
	v8i16 signmask;

	v4i32 zeromask = __builtin_msa_ldi_w(0);
	int alignedWidth = dstWidth & ~7;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v8i16 *) pSrcImage1;
		pLocalSrc2_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 8)
		{
			pixels1L = __builtin_msa_ld_h(pLocalSrc1_msa++, 0);
			pixels2L = __builtin_msa_ld_h(pLocalSrc2_msa++, 0);

			signmask = __builtin_msa_clti_s_h(pixels1L, 0);
			pixels1H = __builtin_msa_ilvl_h(signmask, pixels1L);
			pixels1L = __builtin_msa_ilvr_h(signmask, pixels1L);

			signmask = __builtin_msa_clti_s_h(pixels2L, 0);
			pixels2H = __builtin_msa_ilvl_h(signmask, pixels2L);
			pixels2L = __builtin_msa_ilvr_h(signmask, pixels2L);

			pixels1Hw = __builtin_msa_subv_w((v4i32) pixels1H, (v4i32) pixels2H);
			pixels1Lw = __builtin_msa_subv_w((v4i32) pixels1L, (v4i32) pixels2L);
			pixels1Hw = __builtin_msa_add_a_w(pixels1Hw, zeromask);
			pixels1Lw = __builtin_msa_add_a_w(pixels1Lw, zeromask);

			pixels1Hw = __builtin_msa_sat_s_w(pixels1Hw, 15);
			pixels1Lw = __builtin_msa_sat_s_w(pixels1Lw, 15);
			pixels1L = (v8i16) __builtin_msa_pckev_h((v8i16) pixels1Hw, (v8i16) pixels1Lw);

			__builtin_msa_st_h((v8i16) pixels1L, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_int16 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_int16 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) abs((vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++));
			*pLocalDst++ =	(vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#else
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) abs((vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++));
			*pLocalDst++ =	(vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_S16_S16S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v8i16 pixels1, pixels2, pixels3, pixels4;
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v8i16 *) pSrcImage1;
		pLocalSrc2_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels3 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);
			pixels4 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);
			pixels1 = __builtin_msa_adds_s_h(pixels1, pixels3);
			pixels2 = __builtin_msa_adds_s_h(pixels2, pixels4);
			__builtin_msa_st_h(pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels2, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_int16 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_int16 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) + (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}

#else	// C
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) + (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_S16_S16S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v8i16 pixels1, pixels2, pixels3, pixels4;

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v8i16 *) pSrcImage1;
		pLocalSrc2_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = __builtin_msa_ld_h(pLocalSrc1_msa++, 0 );
			pixels2 = __builtin_msa_ld_h(pLocalSrc1_msa++, 0 );
			pixels3 = __builtin_msa_ld_h(pLocalSrc2_msa++, 0 );
			pixels4 = __builtin_msa_ld_h(pLocalSrc2_msa++, 0 );

			pixels1 = __builtin_msa_addv_h(pixels1, pixels3 );
			pixels2 = __builtin_msa_addv_h(pixels2, pixels4 );

			__builtin_msa_st_h(pixels1, (void *) pLocalDst_msa++, 0 );
			__builtin_msa_st_h(pixels2, (void *) pLocalDst_msa++, 0 );
		}

		pLocalSrc1 = (vx_int16 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_int16 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) + (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) temp;
		}
#else
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) + (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) temp;
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc16_msa, *pLocalSrc8_msa, *pLocalDst_msa;
	v8i16 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc16_msa = (v8i16 *) pSrcImage1;
		pLocalSrc8_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_h((void *) pLocalSrc16_msa++, 0);
			pixels1H = __builtin_msa_ld_h((void *) pLocalSrc16_msa++, 0);
			pixels2L = __builtin_msa_ld_h((void *) pLocalSrc8_msa++, 0);
			pixels2H = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels2L);
			pixels2L = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels2L);

			pixels1L = __builtin_msa_adds_s_h( pixels1L, pixels2L);
			pixels1H = __builtin_msa_adds_s_h( pixels1H, pixels2H);
			__builtin_msa_st_h(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels1H, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc16++) + (vx_int32) (*pLocalSrc8++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#else	// C
		pLocalSrc16 = (vx_int16 *) pSrcImage1;
		pLocalSrc8 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc16++) + (vx_int32) (*pLocalSrc8++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_S16_S16U8_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc16_msa, *pLocalDst_msa;
	v16u8 *pLocalSrc8_msa;
	v8i16 pixels1H, pixels1L;
	v8u16 pixels2H, pixels2L;

	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc16_msa = (v8i16 *) pSrcImage1;
		pLocalSrc8_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_h(pLocalSrc16_msa++, 0);
			pixels1H = __builtin_msa_ld_h(pLocalSrc16_msa++, 0);
			pixels2L = (v8u16) __builtin_msa_ld_b(pLocalSrc8_msa++, 0);

			pixels2H = (v8u16) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels2L);
			pixels2L = (v8u16) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels2L);

			pixels1L = __builtin_msa_addv_h(pixels1L, (v8i16) pixels2L);
			pixels1H = __builtin_msa_addv_h(pixels1H, (v8i16) pixels2H);

			__builtin_msa_st_h(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels1H, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc16++ + (vx_int16) (*pLocalSrc8++);
		}
#else
		pLocalSrc16 = (vx_int16 *) pSrcImage1;
		pLocalSrc8 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc16++ + (vx_int16) (*pLocalSrc8++);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_U8_U8U8_Sat
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
	v16i8 pixels1, pixels2, pixels3, pixels4;
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16i8 *) pSrcImage1;
		pLocalSrc2_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;
		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels3 = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);
			pixels4 = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);
			pixels1 = (v16i8) __builtin_msa_adds_u_b((v16u8) pixels1, (v16u8) pixels3);
			pixels2 = (v16i8) __builtin_msa_adds_u_b((v16u8) pixels2, (v16u8) pixels4);
			__builtin_msa_st_b(pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels2, (void *) pLocalDst_msa++, 0);
		}
		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;
		for (int width = 0; width < postfixWidth; width++)
		{
			int temp = (int) (*pLocalSrc1++) + (int) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) min(temp, UINT8_MAX);
		}
#else	// C
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;
		for (int width = 0; width < dstWidth; width++)
		{
			int temp = *pLocalSrc1++ + *pLocalSrc2++;
			*pLocalDst++ = (vx_uint8) min(temp, UINT8_MAX);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_U8_U8U8_Wrap
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
	v16i8 pixels1, pixels2, pixels3, pixels4;
	int alignedWidth = dstWidth & ~15;
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
			pixels1 = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels3 = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);
			pixels4 = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);
			pixels1 = __builtin_msa_addv_b(pixels1, pixels3);
			pixels2 = __builtin_msa_addv_b(pixels2, pixels4);
			__builtin_msa_st_b(pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels2, (void *) pLocalDst_msa++, 0);
		}
		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;
		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int16 temp = (vx_int16) (*pLocalSrc1++) + (vx_int16) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) temp;
		}
#else	// C
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;
		for (int width = 0; width < dstWidth; width++)
		{
			vx_int16 temp = (vx_int16) (*pLocalSrc1++) + (vx_int16) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) temp;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Add_S16_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2;
	vx_int16 *pLocalDst;
#if ENABLE_MSA
	v16u8 *pLocalSrc1_msa, *pLocalSrc2_msa;
	v8i16 *pLocalDst_msa;
	v8i16 pixels1H, pixels1L, pixels2H, pixels2L;

	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16u8 *) pSrcImage1;
		pLocalSrc2_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_h(pLocalSrc1_msa++, 0);
			pixels2L = __builtin_msa_ld_h(pLocalSrc2_msa++, 0);

			pixels1H = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels1L);
			pixels1L = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels1L);
			pixels2H = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) pixels2L);
			pixels2L = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) pixels2L);

			pixels1L = __builtin_msa_addv_h(pixels1L, pixels2L);
			pixels1H = __builtin_msa_addv_h(pixels1H, pixels2H);

			__builtin_msa_st_h(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels1H, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = (int) (*pLocalSrc1++) + (int) (*pLocalSrc2++);
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = (int) (*pLocalSrc1++) + (int) (*pLocalSrc2++);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_FastAtan2_Canny
	(
		vx_int16	Gx,
		vx_int16	Gy
	)
{
	unsigned int ret;
	vx_uint16 ax, ay;
	ax = abs(Gx), ay = abs(Gy); // todo:: check if math.h function is faster
	float d1 = (float) ax * 0.4142135623730950488016887242097f;
	float d2 = (float) ax * 2.4142135623730950488016887242097f;
	ret = (Gx * Gy) < 0 ? 3 : 1;
	if (ay <= d1)
		ret = 0;
	if (ay >= d2)
		ret = 2;
	return ret;
}

float HafCpu_FastAtan2_deg
	(
		vx_int16      Gx,
		vx_int16      Gy
	)
{
	vx_uint16 ax, ay;
	ax = std::abs(Gx), ay = std::abs(Gy);
	float a, c, c2;
	if (ax >= ay)
	{
		c = (float) ay / ((float) ax + (float) DBL_EPSILON);
		c2 = c*c;
		a = (((atan2_p7 * c2 + atan2_p5) * c2 + atan2_p3) * c2 + atan2_p1) * c;
	}
	else
	{
		c = (float) ax / ((float) ay + (float) DBL_EPSILON);
		c2 = c*c;
		a = 90.f - (((atan2_p7 * c2 + atan2_p5) * c2 + atan2_p3) * c2 + atan2_p1) * c;
	}
	if (Gx < 0)
		a = 180.f - a;
	if (Gy < 0)
		a = 360.f - a;
	return a;
}

#define NUM_BINS 256
int HafCpu_Histogram_DATA_U8
	(
		vx_uint32     dstHist[],
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	unsigned int *pdst = dstHist;
	memset(pdst, 0x0, NUM_BINS * sizeof(unsigned int));
	for (unsigned int y = 0; y < srcHeight; y++)
	{
		unsigned int * src = (unsigned int *) (pSrcImage + y * srcImageStrideInBytes);
		unsigned int * srclast = src + (srcWidth >> 2);
		while (src < srclast)
		{
			v16u8 pixels = (v16u8) __builtin_msa_ld_b((void *) src, 0);

			pdst[pixels[0]]++;
			pdst[pixels[1]]++;
			pdst[pixels[2]]++;
			pdst[pixels[3]]++;
			pdst[pixels[4]]++;
			pdst[pixels[5]]++;
			pdst[pixels[6]]++;
			pdst[pixels[7]]++;
			pdst[pixels[8]]++;
			pdst[pixels[9]]++;
			pdst[pixels[10]]++;
			pdst[pixels[11]]++;
			pdst[pixels[12]]++;
			pdst[pixels[13]]++;
			pdst[pixels[14]]++;
			pdst[pixels[15]]++;

			src += 4;
		}
	}
#else	// C
	unsigned int *pdst = dstHist;
	memset(pdst, 0x0, NUM_BINS * sizeof(unsigned int));
	for (unsigned int y = 0; y < srcHeight; y++)
	{
		unsigned int * src = (unsigned int *) (pSrcImage + y * srcImageStrideInBytes);
		unsigned int * srclast = src + (srcWidth >> 2);
		while (src < srclast)
		{
			// do for 16 pixels..
			unsigned int pixel4;
			pixel4 = *src++;
			pdst[(pixel4 & 0xFF)]++;
			pdst[(pixel4 >> 8) & 0xFF]++;
			pdst[(pixel4 >> 16) & 0xFF]++;
			pdst[(pixel4 >> 24) & 0xFF]++;

			pixel4 = *src++;
			pdst[(pixel4 & 0xFF)]++;
			pdst[(pixel4 >> 8) & 0xFF]++;
			pdst[(pixel4 >> 16) & 0xFF]++;
			pdst[(pixel4 >> 24) & 0xFF]++;

			pixel4 = *src++;
			pdst[(pixel4 & 0xFF)]++;
			pdst[(pixel4 >> 8) & 0xFF]++;
			pdst[(pixel4 >> 16) & 0xFF]++;
			pdst[(pixel4 >> 24) & 0xFF]++;

			pixel4 = *src++;
			pdst[(pixel4 & 0xFF)]++;
			pdst[(pixel4 >> 8) & 0xFF]++;
			pdst[(pixel4 >> 16) & 0xFF]++;
			pdst[(pixel4 >> 24) & 0xFF]++;
		}
	}
#endif
	return AGO_SUCCESS;
}

int HafCpu_Magnitude_S16_S16S16
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pMagImage,
		vx_uint32     magImageStrideInBytes,
		vx_int16    * pGxImage,
		vx_uint32     gxImageStrideInBytes,
		vx_int16    * pGyImage,
		vx_uint32     gyImageStrideInBytes
	)
{
	short *pLocalGx, *pLocalGy, *pLocalDst;
#if ENABLE_MSA
	// check for 16 byte aligned
	int prefixWidth = intptr_t(pMagImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	prefixWidth >>= 1;
	// Check for multiple of 8
	int postfixWidth = ((int) dstWidth - prefixWidth) & 7;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v8i16 pixelsGxH, pixelsGxL, pixelsGyH, pixelsGyL;
	v2f64 pixels0, pixels1, pixels2, pixels3, temp;
	v4i32 zero = __builtin_msa_ldi_w(0);
	v4i32 temp_32;
#endif
	for (unsigned int height = 0; height < dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalGx = (short *) pGxImage;
		pLocalGy = (short *) pGyImage;
		pLocalDst = (short *) pMagImage;

		for (int x = 0; x < prefixWidth; x++, pLocalGx++, pLocalGy++)
		{
			float temp = (float) (*pLocalGx * *pLocalGx) + (float) (*pLocalGy * *pLocalGy);
			temp = sqrtf(temp);
			*pLocalDst++ = max(min((vx_int32) temp, INT16_MAX), INT16_MIN);
		}

		for (int width = 0; width < (alignedWidth >> 3); width++)
		{
			pixelsGxH = __builtin_msa_ld_h((void *) pLocalGx, 0);
			pixelsGyH = __builtin_msa_ld_h((void *) pLocalGy, 0);

			// Convert lower 4 words to dwords
			v8i16 signmask = __builtin_msa_clti_s_h(pixelsGxH, 0);
			pixelsGxL = __builtin_msa_ilvr_h(signmask, pixelsGxH);
			// Convert lower 4 words to dwords
			signmask = __builtin_msa_clti_s_h(pixelsGyH, 0);
			pixelsGyL = __builtin_msa_ilvr_h(signmask, pixelsGyH);

			pixelsGxH = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGxH);
			pixelsGyH = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGyH);

			// Convert upper 4 words to dwords
			signmask = __builtin_msa_clti_s_h(pixelsGxH, 0);
			pixelsGxH = __builtin_msa_ilvr_h(signmask, pixelsGxH);
			// Convert upper 4 words to dwords
			signmask = __builtin_msa_clti_s_h(pixelsGyH, 0);
			pixelsGyH = __builtin_msa_ilvr_h(signmask, pixelsGyH);

			// square
			pixelsGxL = (v8i16) __builtin_msa_mulv_w((v4i32) pixelsGxL, (v4i32) pixelsGxL);
			pixelsGxH = (v8i16) __builtin_msa_mulv_w((v4i32) pixelsGxH, (v4i32) pixelsGxH);
			pixelsGyL = (v8i16) __builtin_msa_mulv_w((v4i32) pixelsGyL, (v4i32) pixelsGyL);
			pixelsGyH = (v8i16) __builtin_msa_mulv_w((v4i32) pixelsGyH, (v4i32) pixelsGyH);

			// Convert to double precision values
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGxL);

			pixels0 = __builtin_msa_ffint_s_d((v2i64) temp_32);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGyL);
			temp = __builtin_msa_ffint_s_d((v2i64) temp_32);

			// Lower two values a^2 + b^2
			pixels0 = __builtin_msa_fadd_d(pixels0, temp);
			pixelsGxL = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGxL);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGxL);
			pixels1 = __builtin_msa_ffint_s_d((v2i64) temp_32);

			pixelsGyL = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGyL);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGyL);
			temp = __builtin_msa_ffint_s_d((v2i64) temp_32);

			// Next two values a^2 + b^2
			pixels1 = __builtin_msa_fadd_d(pixels1, temp);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGxH);
			pixels2 = __builtin_msa_ffint_s_d((v2i64) temp_32);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGyH);
			temp = __builtin_msa_ffint_s_d((v2i64) temp_32);

			// Next two values a^2 + b^2
			pixels2 = __builtin_msa_fadd_d(pixels2, temp);
			pixelsGxH = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGxH);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGxH);
			pixels3 = __builtin_msa_ffint_s_d((v2i64) temp_32);

			pixelsGyH = (v8i16) __builtin_msa_pckod_d((v2i64) zero, (v2i64) pixelsGyH);
			temp_32 = __builtin_msa_ilvr_w(zero, (v4i32) pixelsGyH);
			temp = __builtin_msa_ffint_s_d((v2i64) temp_32);

			// Upper two values a^2 + b^2
			pixels3 = __builtin_msa_fadd_d(pixels3, temp);

			// square root
			pixels0 = __builtin_msa_fsqrt_d(pixels0);
			pixels1 = __builtin_msa_fsqrt_d(pixels1);
			pixels2 = __builtin_msa_fsqrt_d(pixels2);
			pixels3 = __builtin_msa_fsqrt_d(pixels3);

			// Convert double to lower 2 dwords
			pixelsGxL = (v8i16) __builtin_msa_ftint_s_d(pixels0);
			// Convert double to next 2 dwords
			pixelsGyL = (v8i16) __builtin_msa_ftint_s_d(pixels1);
			pixelsGxH = (v8i16) __builtin_msa_ftint_s_d(pixels2);
			pixelsGyH = (v8i16) __builtin_msa_ftint_s_d(pixels3);

			pixelsGxL = (v8i16) __builtin_msa_pckev_w((v4i32) pixelsGyL, (v4i32) pixelsGxL);
			pixelsGxH = (v8i16) __builtin_msa_pckev_w((v4i32) pixelsGyH, (v4i32) pixelsGxH);

			v4i32 temp0_u = __builtin_msa_sat_s_w((v4i32) pixelsGxL, 15);
			v4i32 temp1_u = __builtin_msa_sat_s_w((v4i32) pixelsGxH, 15);
			pixelsGxL = __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			__builtin_msa_st_h(pixelsGxL, pLocalDst, 0);

			pLocalGx += 8;
			pLocalGy += 8;
			pLocalDst += 8;
		}
		for (int x = 0; x < postfixWidth; x++, pLocalGx++, pLocalGy++)
		{
			float temp = (float) (*pLocalGx * *pLocalGx) + (float) (*pLocalGy * *pLocalGy);
			temp = sqrtf(temp);
			*pLocalDst++ = max(min((vx_int32) temp, INT16_MAX), INT16_MIN);
		}
#else // C
		pLocalGx = (short *) pGxImage;
		pLocalGy = (short *) pGyImage;
		pLocalDst = (short *) pMagImage;

		for (int x = 0; x < dstWidth; x++, pLocalGx++, pLocalGy++)
		{
			float temp = (float) (*pLocalGx * *pLocalGx) + (float) (*pLocalGy * *pLocalGy);
			temp = sqrtf(temp);
			*pLocalDst++ = max(min((vx_int32) temp, INT16_MAX), INT16_MIN);
		}
#endif
		pGxImage += (gxImageStrideInBytes >> 1);
		pGyImage += (gyImageStrideInBytes >> 1);
		pMagImage += (magImageStrideInBytes >> 1);
	}

	return AGO_SUCCESS;
}

int HafCpu_MeanStdDevMerge_DATA_DATA
	(
		vx_float32  * mean,
		vx_float32  * stddev,
		vx_uint32	  totalSampleCount,
		vx_uint32     numPartitions,
		vx_float32    partSum[],
		vx_float32    partSumOfSquared[]
	)
{
	vx_float32 lmean = 0, lstd = 0;

	for (unsigned int i = 0; i < numPartitions; i++)
	{
		lmean += partSum[i];
		lstd += partSumOfSquared[i];
	}

	lmean /= totalSampleCount;
	lstd = sqrtf((lstd / totalSampleCount) - (lmean * lmean));

	*mean = lmean;
	*stddev = lstd;

	return AGO_SUCCESS;
}

int HafCpu_MeanStdDev_DATA_U8
	(
		vx_float32  * pSum,
		vx_float32  * pSumOfSquared,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	unsigned char * pLocalSrc;
#if ENABLE_MSA
	v16i8 pixels, pixels_16, pixels_32, pixels_64;
	v16i8 zeromask, sum, sum_squared;
	zeromask = sum = sum_squared = __builtin_msa_ldi_b(0);

	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - prefixWidth - postfixWidth;
	unsigned int prefixSum = 0, postfixSum = 0;
	unsigned long long prefixSumSquared = 0, postfixSumSquared = 0;
#else //	C
	unsigned int sum = 0;
	unsigned long long sum_squared = 0;
#endif
	int height = (int) srcHeight;
	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			prefixSum += (unsigned int) *pLocalSrc;
			prefixSumSquared += (unsigned long long)*pLocalSrc * (unsigned long long)*pLocalSrc;
		}
		// 16 pixels processed at a time
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels = __builtin_msa_ld_b((void *) pLocalSrc, 0);

			// 15, 14, 13, 12, 11, 10, 9, 8
			pixels_16 = __builtin_msa_ilvl_b(zeromask, pixels);
			// 15, 14, 13, 12
			pixels_32 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) pixels_16);
			// Pixels 15, 14, 13, 12
			sum = (v16i8)__builtin_msa_addv_w((v4i32) sum, (v4i32) pixels_32);
			// 15, 14
			pixels_64 = (v16i8) __builtin_msa_ilvl_w((v4i32) zeromask, (v4i32) pixels_32);
			// 13, 12
			pixels_32 = (v16i8) __builtin_msa_ilvr_w((v4i32) zeromask, (v4i32) pixels_32);

			// square
			pixels_64 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_64, (v4i32) pixels_64);
			pixels_32 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_32, (v4i32) pixels_32);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_64);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_32);

			// Pixels 11, 10, 9, 8
			pixels_32 = (v16i8) __builtin_msa_ilvr_h((v8i16) zeromask, (v8i16) pixels_16);
			sum = (v16i8)__builtin_msa_addv_w((v4i32) sum, (v4i32) pixels_32);
			// 11, 10
			pixels_64 = (v16i8) __builtin_msa_ilvl_w((v4i32) zeromask, (v4i32) pixels_32);
			// 9, 8
			pixels_32 = (v16i8) __builtin_msa_ilvr_w((v4i32) zeromask, (v4i32) pixels_32);

			// square
			pixels_64 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_64, (v4i32) pixels_64);
			pixels_32 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_32,(v4i32) pixels_32);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_64);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_32);

			// 7, 6, 5, 4, 3, 2, 1, 0
			pixels_16 = __builtin_msa_ilvr_b(zeromask, pixels);
			// 7, 6, 5, 4
			pixels_32 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) pixels_16);
			// Pixels 7, 6, 5, 4
			sum = (v16i8)__builtin_msa_addv_w((v4i32) sum, (v4i32) pixels_32);
			// 7, 6
			pixels_64 = (v16i8) __builtin_msa_ilvl_w((v4i32) zeromask, (v4i32) pixels_32);
			// 5, 4
			pixels_32 = (v16i8) __builtin_msa_ilvr_w((v4i32) zeromask, (v4i32) pixels_32);

			// square
			pixels_64 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_64, (v4i32) pixels_64);
			pixels_32 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_32, (v4i32) pixels_32);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_64);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_32);

			// Pixels 3, 2, 1, 0
			pixels_32 = (v16i8) __builtin_msa_ilvr_h((v8i16) zeromask, (v8i16) pixels_16);
			sum = (v16i8)__builtin_msa_addv_w((v4i32) sum, (v4i32) pixels_32);
			// 3, 2
			pixels_64 = (v16i8) __builtin_msa_ilvl_w((v4i32) zeromask, (v4i32) pixels_32);
			// 1, 0
			pixels_32 = (v16i8) __builtin_msa_ilvr_w((v4i32) zeromask, (v4i32) pixels_32);

			// square
			pixels_64 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_64, (v4i32) pixels_64);
			pixels_32 = (v16i8) __builtin_msa_mulv_w((v4i32) pixels_32, (v4i32) pixels_32);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_64);
			sum_squared = (v16i8) __builtin_msa_addv_d((v2i64) sum_squared, (v2i64) pixels_32);

			pLocalSrc += 16;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			postfixSum += (unsigned int)*pLocalSrc;
			postfixSumSquared += (unsigned long long)*pLocalSrc * (unsigned long long)*pLocalSrc;
		}
#else	// C
		int width = (int) (srcWidth);
		while (width)
		{
			sum += (unsigned int) *pLocalSrc;
			sum_squared += (unsigned long long) *pLocalSrc * (unsigned long long) *pLocalSrc;

			pLocalSrc += 1;
			width--;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		height--;
	}
#if ENABLE_MSA
	sum = (v16i8) __builtin_msa_hadd_s_d((v4i32) sum, (v4i32) sum);
	*pSum = (vx_float32) (__builtin_msa_copy_s_d((v2i64) sum,0) + __builtin_msa_copy_s_d((v2i64) sum, 1) + prefixSum + postfixSum);
	*pSumOfSquared = (vx_float32) (__builtin_msa_copy_s_d((v2i64) sum_squared, 0) + __builtin_msa_copy_s_d((v2i64) sum_squared, 1) + prefixSumSquared + postfixSumSquared);
#else	// C
	*pSum = sum;
	*pSumOfSquared = sum_squared;
#endif
	return AGO_SUCCESS;
}

int HafCpu_MinMaxMerge_DATA_DATA
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     numDataPartitions,
		vx_int32      srcMinValue[],
		vx_int32      srcMaxValue[]
	)
{
	vx_int32 minVal, maxVal;

	minVal = srcMinValue[0];
	maxVal = srcMaxValue[0];

	for (int i = 1; i < (int) numDataPartitions; i++)
	{
		minVal = min(minVal, srcMinValue[i]);
		maxVal = min(minVal, srcMaxValue[i]);
	}

	*pDstMinValue = minVal;
	*pDstMaxValue = maxVal;

	return AGO_SUCCESS;
}

int HafCpu_MinMax_DATA_U8
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	v16u8 *pLocalSrc_msa;
	v16u8 pixels;
	v16u8 maxVal_msa = (v16u8)__builtin_msa_ldi_b(0);
	v16u8 minVal_msa = (v16u8)__builtin_msa_fill_b((char) 0xFF);
#endif
	unsigned char maxVal = 0, minVal = 255;
	unsigned char * pLocalSrc;
	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - prefixWidth - postfixWidth;

	int height = (int) srcHeight;
	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}
		pLocalSrc_msa = (v16u8 *) pLocalSrc;

		// 16 pixels processed at a time
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels = (v16u8)__builtin_msa_ld_b(pLocalSrc_msa++, 0);
			maxVal_msa = __builtin_msa_max_u_b(maxVal_msa, pixels);
			minVal_msa = __builtin_msa_min_u_b(minVal_msa, pixels);

			width--;
		}
		pLocalSrc = (unsigned char *) pLocalSrc_msa;
		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}
		for (int i = 0; i < 16; i++)
		{
			maxVal = max(maxVal, *((unsigned char*) &maxVal_msa[i]));
			minVal = min(minVal, *((unsigned char*) &minVal_msa[i]));
		}
#else
		for (int x = 0; x < srcWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		height--;
	}
	*pDstMinValue = (vx_int32) minVal;
	*pDstMaxValue = (vx_int32) maxVal;

	return AGO_SUCCESS;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_MinMax_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	)
{
	// Compute the global minima and maxima
	vx_int32 globalMin, globalMax;
	HafCpu_MinMaxMerge_DATA_DATA(&globalMin, &globalMax, numDataPartitions, srcMinValue, srcMaxValue);

	*pDstMinValue = globalMin;
	*pDstMaxValue = globalMax;

	// Search for the min and the max values in the source image
#if ENABLE_MSA
	v16u8 minVal = (v16u8) __builtin_msa_fill_b((unsigned char) globalMin);
	v16u8 maxVal = (v16u8) __builtin_msa_fill_b((unsigned char) globalMax);
	v16u8 pixels;

	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - postfixWidth;

	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16u8 vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
#endif

	int minCount = 0, maxCount = 0;
	unsigned char * pLocalSrc;

	bool minListNotFull = (minCount < (int) capacityOfMinLocList);
	bool maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
	vx_coordinates2d_t loc;

	for (int height = 0; height < (int) srcHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 0;
		while (width < prefixWidth)
		{
			if (*pLocalSrc == globalMin)
			{
				if (minListNotFull)
				{
					loc.x = width;
					loc.y = height;
					minLocList[minCount] = loc;
				}
				minCount++;
				minListNotFull = (minCount < (int) capacityOfMinLocList);
			}
			if (*pLocalSrc == globalMax)
			{
				if (maxListNotFull)
				{
					loc.x = width;
					loc.y = height;
					maxLocList[maxCount] = loc;
				}
				maxCount++;
				maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
			}
			width++;
			pLocalSrc++;
		}
		while (width < alignedWidth)
		{
			int minMask, maxMask;

			pixels = (v16u8) __builtin_msa_ld_b(pLocalSrc, 0);
			v16u8 temp = (v16u8) __builtin_msa_ceq_b((v16i8) pixels, (v16i8) minVal);

			temp = __builtin_msa_and_v(temp, vmask);
			temp = (v16u8) __builtin_msa_hadd_u_h(temp, temp);
			temp = (v16u8) __builtin_msa_hadd_u_w((v8u16) temp,(v8u16) temp);
			temp = (v16u8) __builtin_msa_hadd_u_d((v4u32) temp,(v4u32) temp);
			minMask = __builtin_msa_copy_u_d((v2i64) temp, 1);
			minMask = (vx_uint16) minMask << 8;
			minMask = minMask | __builtin_msa_copy_u_d((v2i64) temp, 0);

			temp = (v16u8) __builtin_msa_ceq_b((v16i8) pixels, (v16i8) maxVal);

			temp = __builtin_msa_and_v(temp, vmask);
			temp = (v16u8) __builtin_msa_hadd_u_h(temp, temp);
			temp = (v16u8) __builtin_msa_hadd_u_w((v8u16) temp,(v8u16) temp);
			temp = (v16u8) __builtin_msa_hadd_u_d((v4u32) temp,(v4u32) temp);
			maxMask = __builtin_msa_copy_u_d((v2i64) temp, 1);
			maxMask = (vx_uint16) maxMask << 8;
			maxMask = maxMask | __builtin_msa_copy_u_d((v2i64) temp, 0);

			if (minMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (minMask & 1)
					{
						if (minListNotFull)
						{
							loc.y = height;
							loc.x = width + i;
							minLocList[minCount] = loc;
						}
						minCount++;
						minListNotFull = (minCount < (int) capacityOfMinLocList);
					}
					minMask >>= 1;
				}
			}
			if (maxMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (maxMask & 1)
					{
						if (maxListNotFull)
						{
							loc.y = height;
							loc.x = width + i;
							maxLocList[maxCount] = loc;
						}
						maxCount++;
						maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
					}
					maxMask >>= 1;
				}
			}
			width += 16;
			pLocalSrc += 16;
		}

		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
			{
				if (minListNotFull)
				{
					loc.x = width;
					loc.y = height;
					minLocList[minCount] = loc;
				}
				minCount++;
				minListNotFull = (minCount < (int) capacityOfMinLocList);
			}
			if (*pLocalSrc == globalMax)
			{
				if (maxListNotFull)
				{
					loc.x = width;
					loc.y = height;
					maxLocList[maxCount] = loc;
				}
				maxCount++;
				maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
			}
			width++;
			pLocalSrc++;
		}
#else // C
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 0;

		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
			{
				if (minListNotFull)
				{
					loc.x = width;
					loc.y = height;
					minLocList[minCount] = loc;
				}
				minCount++;
				minListNotFull = (minCount < (int) capacityOfMinLocList);
			}
			if (*pLocalSrc == globalMax)
			{
				if (maxListNotFull)
				{
					loc.x = width;
					loc.y = height;
					maxLocList[maxCount] = loc;
				}
				maxCount++;
				maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
			}
			width++;
			pLocalSrc++;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
	}
	*pMinLocCount = (vx_int32) minCount;
	*pMaxLocCount = (vx_int32) maxCount;

	return AGO_SUCCESS;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_None_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	)
{
	// Compute the global minima and maxima
	vx_int32 globalMin, globalMax;
	HafCpu_MinMaxMerge_DATA_DATA(&globalMin, &globalMax, numDataPartitions, srcMinValue, srcMaxValue);

	*pDstMinValue = globalMin;
	*pDstMaxValue = globalMax;

#if ENABLE_MSA
	// Search for the min and the max values in the source image
	v16u8 minVal = (v16u8) __builtin_msa_fill_b((unsigned char) globalMin);
	v16u8 maxVal = (v16u8) __builtin_msa_fill_b((unsigned char) globalMax);
	v16u8 pixels, temp;

	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - postfixWidth;

	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16u8 vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
#endif
	int minCount = 0, maxCount = 0;
	unsigned char * pLocalSrc;

	for (int height = 0; height < (int) srcHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 0;
		while (width < prefixWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;

			width++;
			pLocalSrc++;
		}
		while (width < alignedWidth)
		{
			int minMask, maxMask;

			pixels = (v16u8) __builtin_msa_ld_b(pLocalSrc, 0);

			temp = (v16u8) __builtin_msa_ceq_b((v16i8) pixels, (v16i8) minVal);
			temp = __builtin_msa_and_v(temp, vmask);
			temp = (v16u8) __builtin_msa_hadd_u_h(temp, temp);
			temp = (v16u8) __builtin_msa_hadd_u_w((v8u16) temp,(v8u16) temp);
			temp = (v16u8) __builtin_msa_hadd_u_d((v4u32) temp,(v4u32) temp);
			minMask = __builtin_msa_copy_u_d((v2i64) temp, 1);
			minMask = (vx_uint16) minMask << 8;
			minMask = minMask | __builtin_msa_copy_u_d((v2i64) temp, 0);

			temp = (v16u8) __builtin_msa_ceq_b((v16i8) pixels, (v16i8) maxVal);
			temp = __builtin_msa_and_v(temp, vmask);
			temp = (v16u8) __builtin_msa_hadd_u_h(temp, temp);
			temp = (v16u8) __builtin_msa_hadd_u_w((v8u16) temp,(v8u16) temp);
			temp = (v16u8) __builtin_msa_hadd_u_d((v4u32) temp,(v4u32) temp);
			maxMask = __builtin_msa_copy_u_d((v2i64) temp, 1);
			maxMask = (vx_uint16) maxMask << 8;
			maxMask = maxMask | __builtin_msa_copy_u_d((v2i64) temp, 0);

			if (minMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (minMask & 1)
						minCount++;
					minMask >>= 1;
				}
			}
			if (maxMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (maxMask & 1)
						maxCount++;
					maxMask >>= 1;
				}
			}
			width += 16;
			pLocalSrc += 16;
		}
		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;

			width++;
			pLocalSrc++;
		}
#else // C
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 0;

		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;

			width++;
			pLocalSrc++;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
	}
	*pMinLocCount = (vx_int32) minCount;
	*pMaxLocCount = (vx_int32) maxCount;

	return AGO_SUCCESS;
}

int HafCpu_MinMax_DATA_S16
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 2 bytes = 1 pixel
	prefixWidth >>= 1;
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - prefixWidth - postfixWidth;
	short maxVal = SHRT_MIN, minVal = SHRT_MAX;
	short * pLocalSrc;
#if ENABLE_MSA
	v8i16 * pLocalSrc_msa;
	v8i16 pixels;
	v8i16 maxVal_msa = __builtin_msa_fill_h(maxVal);
	v8i16 minVal_msa = __builtin_msa_fill_h(minVal);
#endif

	int height = (int) srcHeight;
	while (height)
	{
		pLocalSrc = (short *) pSrcImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}
		pLocalSrc_msa = (v8i16*) pLocalSrc;

		// 8 pixels processed at a time
		int width = (int) (alignedWidth >> 3);
		while (width)
		{
			pixels = __builtin_msa_ld_h(pLocalSrc_msa++, 0);
			maxVal_msa = __builtin_msa_max_s_h(maxVal_msa, pixels);
			minVal_msa = __builtin_msa_min_s_h(minVal_msa, pixels);

			width--;
		}
		pLocalSrc = (short *) pLocalSrc_msa;
		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}

		// Compute the max value out of the max at 16 individual places
		for (int i = 0; i < 8; i++)
		{
			maxVal = max(maxVal, *((short*) &maxVal_msa[i]));
			minVal = min(minVal, *((short*) &minVal_msa[i]));
		}
#else
		for (int x = 0; x < srcWidth; x++, pLocalSrc++)
		{
			maxVal = max(maxVal, *pLocalSrc);
			minVal = min(minVal, *pLocalSrc);
		}
#endif

	pSrcImage += (srcImageStrideInBytes >> 1);
	height--;
	}
	*pDstMinValue = (vx_int32) minVal;
	*pDstMaxValue = (vx_int32) maxVal;

	return AGO_SUCCESS;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_MinMax_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	)
{
	// Compute the global minima and maxima
	vx_int32 globalMin, globalMax;
	HafCpu_MinMaxMerge_DATA_DATA(&globalMin, &globalMax, numDataPartitions, srcMinValue, srcMaxValue);

	*pDstMinValue = globalMin;
	*pDstMaxValue = globalMax;

	int minCount = 0, maxCount = 0;
	short * pLocalSrc;

	bool minListNotFull = (minCount < (int) capacityOfMinLocList);
	bool maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
	vx_coordinates2d_t loc;

	for (int height = 0; height < (int) srcHeight; height++)
	{
		pLocalSrc = (short *) pSrcImage;
		int width = 0;

		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
			{
				if (minListNotFull)
				{
					loc.x = width;
					loc.y = height;
					minLocList[minCount] = loc;
				}
				minCount++;
				minListNotFull = (minCount < (int) capacityOfMinLocList);
			}
			if (*pLocalSrc == globalMax)
			{
				if (maxListNotFull)
				{
					loc.x = width;
					loc.y = height;
					maxLocList[maxCount] = loc;
				}
				maxCount++;
				maxListNotFull = (maxCount < (int) capacityOfMaxLocList);
			}
			width++;
			pLocalSrc++;
		}
		pSrcImage += (srcImageStrideInBytes >> 1);
	}
	*pMinLocCount = (vx_int32) minCount;
	*pMaxLocCount = (vx_int32) maxCount;

	return AGO_SUCCESS;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_None_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	)
{
	// Compute the global minima and maxima
	vx_int32 globalMin, globalMax;
	HafCpu_MinMaxMerge_DATA_DATA(&globalMin, &globalMax, numDataPartitions, srcMinValue, srcMaxValue);

	*pDstMinValue = globalMin;
	*pDstMaxValue = globalMax;
#if ENABLE_MSA
	// Search for the min and the max values in the source image
	v8u16 minVal = (v8u16) __builtin_msa_fill_h((short) globalMin);
	v8u16 maxVal = (v8u16) __builtin_msa_fill_h((short) globalMax);
	v16u8 pixelsL, pixelsH;
	v8u16 temp0, temp1;

	int prefixWidth = intptr_t(pSrcImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	// 2 bytes = 1 pixel
	prefixWidth >>= 1;
	int postfixWidth = ((int) srcWidth - prefixWidth) & 15;
	int alignedWidth = (int) srcWidth - postfixWidth;

	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16u8 vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
#endif
	int minCount = 0, maxCount = 0;
	short * pLocalSrc;

	for (int height = 0; height < (int) srcHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc = (short *) pSrcImage;
		int width = 0;

		while (width < prefixWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;
			width++;
			pLocalSrc++;
		}
		while (width < alignedWidth)
		{
			int minMask, maxMask;

			pixelsL = (v16u8) __builtin_msa_ld_b(pLocalSrc, 0);
			pixelsH = (v16u8) __builtin_msa_ld_b((pLocalSrc + 8), 0);

			temp1 = (v8u16) __builtin_msa_ceq_h((v8i16) pixelsH, (v8i16) minVal);
			temp0 = (v8u16) __builtin_msa_ceq_h((v8i16) pixelsL, (v8i16) minVal);

			temp0 = (v8u16) __builtin_msa_pckev_b((v16i8) __builtin_msa_sat_s_h((v8i16) temp0, 7),
													(v16i8) __builtin_msa_sat_s_h((v8i16) temp1, 7));

			temp0 = (v8u16) __builtin_msa_and_v((v16u8) temp0, vmask);
			temp0 = (v8u16) __builtin_msa_hadd_u_h((v16u8) temp0, (v16u8) temp0);
			temp0 = (v8u16) __builtin_msa_hadd_u_w((v8u16) temp0, (v8u16) temp0);
			temp0 = (v8u16) __builtin_msa_hadd_u_d((v4u32) temp0, (v4u32) temp0);
			minMask = __builtin_msa_copy_u_d((v2i64) temp0, 1);
			minMask = (vx_uint16) minMask << 8;
			minMask = minMask | __builtin_msa_copy_u_d((v2i64) temp0, 0);

			pixelsH = (v16u8) __builtin_msa_ceq_h((v8i16) pixelsH, (v8i16) maxVal);
			pixelsL = (v16u8) __builtin_msa_ceq_h((v8i16) pixelsL, (v8i16) maxVal);

			temp1 = (v8u16) __builtin_msa_pckev_b((v16i8) __builtin_msa_sat_s_h((v8i16) pixelsL, 7),
													(v16i8) __builtin_msa_sat_s_h((v8i16) pixelsH, 7));

			temp1 = (v8u16) __builtin_msa_and_v((v16u8) temp1, vmask);
			temp1 = (v8u16) __builtin_msa_hadd_u_h((v16u8) temp1, (v16u8) temp1);
			temp1 = (v8u16) __builtin_msa_hadd_u_w((v8u16) temp1, (v8u16) temp1);
			temp1 = (v8u16) __builtin_msa_hadd_u_d((v4u32) temp1, (v4u32) temp1);
			maxMask = __builtin_msa_copy_u_d((v2i64) temp1, 1);
			maxMask = (vx_uint16) maxMask << 8;
			maxMask = maxMask | __builtin_msa_copy_u_d((v2i64) temp1, 0);

			if (minMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (minMask & 1)
						minCount++;
					minMask >>= 1;
				}
			}
			if (maxMask)
			{
				for (int i = 0; i < 16; i++)
				{
					if (maxMask & 1)
						maxCount++;
					maxMask >>= 1;
				}
			}
			width += 16;
			pLocalSrc += 16;
		}
		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;
			width++;
			pLocalSrc++;
		}
#else // C
		pLocalSrc = (short *) pSrcImage;
		int width = 0;

		while (width < (int) srcWidth)
		{
			if (*pLocalSrc == globalMin)
				minCount++;
			if (*pLocalSrc == globalMax)
				maxCount++;

			width++;
			pLocalSrc++;
		}
#endif
		pSrcImage += (srcImageStrideInBytes >> 1);
	}
	*pMinLocCount = (vx_int32) minCount;
	*pMaxLocCount = (vx_int32) maxCount;

	return AGO_SUCCESS;
}

int HafCpu_Phase_U8_S16S16
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pPhaseImage,
		vx_uint32     phaseImageStrideInBytes,
		vx_int16    * pGxImage,
		vx_uint32     gxImageStrideInBytes,
		vx_int16    * pGyImage,
		vx_uint32     gyImageStrideInBytes
	)
{
	unsigned int y = 0;
	// do the plain vanilla version with atan2
	while (y < dstHeight)
	{
		vx_uint8 *pdst = pPhaseImage;
		vx_int16 *pGx = pGxImage;
		vx_int16 *pGy = pGyImage;

		for (unsigned int x = 0; x < dstWidth; x++)
		{
			float scale = (float)128 / 180.f;
			float arct = HafCpu_FastAtan2_deg(pGx[x], pGy[x]);
			// normalize and copy to dst
			*pdst++ = (vx_uint8) ((vx_uint32) (arct*scale + 0.5) & 0xFF);

		}
		pPhaseImage += phaseImageStrideInBytes;
		pGxImage += (gxImageStrideInBytes>>1);
		pGyImage += (gyImageStrideInBytes>>1);
		y++;
	}
	return AGO_SUCCESS;
}
int HafCpu_Sub_U8_U8U8_Wrap
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
	v16u8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16u8 pixels1, pixels2;
	v16u8 pixels21, pixels22;

	int alignedWidth = dstWidth & ~31;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16u8 *) pSrcImage1;
		pLocalSrc2_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v16u8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);
			pixels21 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels22 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1 = (v16u8) __builtin_msa_subv_b((v16i8) pixels1, (v16i8) pixels2);
			pixels21 = (v16u8) __builtin_msa_subv_b((v16i8) pixels21, (v16i8) pixels22);

			__builtin_msa_st_b((v16i8) pixels1, pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels21, pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int16 temp = (vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) temp;
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int16 temp = (vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) temp;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_U8_U8U8_Sat
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
	v16u8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16u8 pixels1, pixels2;
	v16u8 pixels21, pixels22;

	int alignedWidth = dstWidth & ~31;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v16u8 *) pSrcImage1;
		pLocalSrc2_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v16u8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 32)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);
			pixels21 = (v16u8) __builtin_msa_ld_b(pLocalSrc1_msa++, 0);
			pixels22 = (v16u8) __builtin_msa_ld_b(pLocalSrc2_msa++, 0);

			pixels1 = __builtin_msa_subs_u_b(pixels1, pixels2);
			pixels21 = __builtin_msa_subs_u_b(pixels21, pixels22);

			__builtin_msa_st_b((v16i8) pixels1, pLocalDst_msa++, 0);
			__builtin_msa_st_b((v16i8) pixels21, pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			int temp = (int) (*pLocalSrc1++) - (int) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) max(min(temp, UINT8_MAX), 0);
		}
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			int temp = (int) (*pLocalSrc1++) - (int) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) max(min(temp, UINT8_MAX), 0);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;

#if ENABLE_MSA
	v8i16 *pLocalSrc16_msa, *pLocalDst_msa;
	v16u8 *pLocalSrc8_msa;
	v16i8 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc16_msa = (v8i16 *) pSrcImage1;
		pLocalSrc8_msa = (v16u8 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_b(pLocalSrc16_msa++, 0);
			pixels1H = __builtin_msa_ld_b(pLocalSrc16_msa++, 0);
			pixels2L = __builtin_msa_ld_b(pLocalSrc8_msa++, 0);

			pixels2H = __builtin_msa_ilvl_b(zeromask, pixels2L);
			pixels2L = __builtin_msa_ilvr_b(zeromask, pixels2L);

			pixels1L = (v16i8) __builtin_msa_subs_s_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixels1H = (v16i8) __builtin_msa_subs_s_h((v8i16) pixels1H, (v8i16) pixels2H);

			__builtin_msa_st_b(pixels1L, pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels1H, pLocalDst_msa++, 0);
		}
		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc16++) - (vx_int32) (*pLocalSrc8++);
			*pLocalDst++ = max(min(temp, INT16_MAX), INT16_MIN);
		}
#else // C
		pLocalSrc16 = (vx_int16 *) pSrcImage1;
		pLocalSrc8 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc16++) - (vx_int32) (*pLocalSrc8++);
			*pLocalDst++ = max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_S16U8_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;

#if ENABLE_MSA
	v16i8 *pLocalSrc16_msa, *pLocalSrc8_msa, *pLocalDst_msa;
	v16i8 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc16_msa = (v16i8 *) pSrcImage1;
		pLocalSrc8_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_b((void *) pLocalSrc16_msa++, 0);
			pixels1H = __builtin_msa_ld_b((void *) pLocalSrc16_msa++, 0);
			pixels2L = __builtin_msa_ld_b((void *) pLocalSrc8_msa++, 0);

			pixels2H = __builtin_msa_ilvl_b(zeromask, pixels2L);
			pixels2L = __builtin_msa_ilvr_b(zeromask, pixels2L);

			pixels1L = (v16i8) __builtin_msa_subv_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixels1H = (v16i8) __builtin_msa_subv_h((v8i16) pixels1H, (v8i16) pixels2H);

			__builtin_msa_st_b(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels1H, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc16++ - (vx_int16) (*pLocalSrc8++);
		}

#else	// C
		pLocalSrc16 = (vx_int16 *) pSrcImage1;
		pLocalSrc8 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;
		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = *pLocalSrc16++ - (vx_int16) (*pLocalSrc8++);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_U8S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;

#if ENABLE_MSA
	v16i8 *pLocalSrc16_msa, *pLocalSrc8_msa, *pLocalDst_msa;
	v16i8 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc8_msa = (v16i8 *) pSrcImage1;
		pLocalSrc16_msa = (v16i8 *) pSrcImage2;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_b((void *) pLocalSrc8_msa++, 0);
			pixels2L = __builtin_msa_ld_b((void *) pLocalSrc16_msa++, 0);
			pixels2H = __builtin_msa_ld_b((void *) pLocalSrc16_msa++, 0);
			pixels1H = __builtin_msa_ilvl_b(zeromask, pixels1L);
			pixels1L = __builtin_msa_ilvr_b(zeromask, pixels1L);
			pixels1L = (v16i8) __builtin_msa_subs_s_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixels1H = (v16i8) __builtin_msa_subs_s_h((v8i16) pixels1H, (v8i16) pixels2H);
			__builtin_msa_st_b(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels1H, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc8++) - (vx_int32) (*pLocalSrc16++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#else	// C
		pLocalSrc16 = (vx_int16 *) pSrcImage2;
		pLocalSrc8 = (vx_uint8 *) pSrcImage1;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc8++) - (vx_int32) (*pLocalSrc16++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_U8S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc8;
	vx_int16 *pLocalSrc16, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc16_msa, *pLocalDst_msa;
	v16u8 *pLocalSrc8_msa;
	v16i8 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc8_msa = (v16u8 *) pSrcImage1;
		pLocalSrc16_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1L = __builtin_msa_ld_b(pLocalSrc8_msa++, 0);
			pixels2L = __builtin_msa_ld_b(pLocalSrc16_msa++, 0);
			pixels2H = __builtin_msa_ld_b(pLocalSrc16_msa++, 0);

			pixels1H = __builtin_msa_ilvl_b(zeromask, pixels1L);
			pixels1L = __builtin_msa_ilvr_b(zeromask, pixels1L);

			pixels1L = (v16i8) __builtin_msa_subv_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixels1H = (v16i8) __builtin_msa_subv_h((v8i16) pixels1H, (v8i16) pixels2H);

			__builtin_msa_st_b(pixels1L, pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels1H, pLocalDst_msa++, 0);
		}
		pLocalSrc8 = (vx_uint8 *) pLocalSrc8_msa;
		pLocalSrc16 = (vx_int16 *) pLocalSrc16_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = (vx_int16) (*pLocalSrc8++) - *pLocalSrc16++;
		}
#else // C
		pLocalSrc8 = (vx_uint8 *) pSrcImage1;
		pLocalSrc16 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = (vx_int16) (*pLocalSrc8++) - *pLocalSrc16++;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}
int HafCpu_Sub_S16_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_uint8 *pLocalSrc1, *pLocalSrc2;
	vx_int16 *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v16i8 pixels1H, pixels1L, pixels2H, pixels2L;
	v16i8 pixels11H, pixels11L, pixels22H, pixels22L;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	int alignedWidth = dstWidth & ~15;
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
			pixels1L = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels11L = __builtin_msa_ld_b((void *) pLocalSrc1_msa++, 0);
			pixels2L = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);
			pixels22L = __builtin_msa_ld_b((void *) pLocalSrc2_msa++, 0);

			pixels1H = __builtin_msa_ilvl_b(zeromask, pixels1L);
			pixels1L = __builtin_msa_ilvr_b(zeromask, pixels1L);
			pixels11H = __builtin_msa_ilvl_b(zeromask, pixels11L);
			pixels11L = __builtin_msa_ilvr_b(zeromask, pixels11L);

			pixels2H = __builtin_msa_ilvl_b(zeromask, pixels2L);
			pixels2L = __builtin_msa_ilvr_b(zeromask, pixels2L);
			pixels22H = __builtin_msa_ilvl_b(zeromask, pixels22L);
			pixels22L = __builtin_msa_ilvr_b(zeromask, pixels22L);

			pixels1L = (v16i8) __builtin_msa_subv_h((v8i16) pixels1L, (v8i16) pixels2L);
			pixels1H = (v16i8) __builtin_msa_subv_h((v8i16) pixels1H, (v8i16) pixels2H);
			pixels11L = (v16i8) __builtin_msa_subv_h((v8i16) pixels11L, (v8i16) pixels22L);
			pixels11H = (v16i8) __builtin_msa_subv_h((v8i16) pixels11H, (v8i16) pixels22H);

			__builtin_msa_st_b(pixels1L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels1H, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels11L, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels11H, (void *) pLocalDst_msa++, 0);
		}
		pLocalSrc1 = (vx_uint8 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_uint8 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;
		for (int width = 0; width < postfixWidth; width++)
		{
			*pLocalDst++ = (vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++);
		}
#else	// C
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			*pLocalDst++ = (vx_int16) (*pLocalSrc1++) - (vx_int16) (*pLocalSrc2++);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}
	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_S16S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v8i16 pixels1, pixels2, pixels3, pixels4;
	v8i16 zeromask = __builtin_msa_ldi_h(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v8i16 *) pSrcImage1;
		pLocalSrc2_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels3 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);
			pixels4 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);

			pixels1 = __builtin_msa_subs_s_h(pixels1, pixels3);
			pixels2 = __builtin_msa_subs_s_h(pixels2, pixels4);

			__builtin_msa_st_h(pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels2, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_int16 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_int16 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#else // C
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}

	return AGO_SUCCESS;
}

int HafCpu_Sub_S16_S16S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	)
{
	vx_int16 *pLocalSrc1, *pLocalSrc2, *pLocalDst;
#if ENABLE_MSA
	v8i16 *pLocalSrc1_msa, *pLocalSrc2_msa, *pLocalDst_msa;
	v8i16 pixels1, pixels2, pixels3, pixels4;
	v8i16 zeromask = __builtin_msa_ldi_h(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc1_msa = (v8i16 *) pSrcImage1;
		pLocalSrc2_msa = (v8i16 *) pSrcImage2;
		pLocalDst_msa = (v8i16 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels2 = __builtin_msa_ld_h((void *) pLocalSrc1_msa++, 0);
			pixels3 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);
			pixels4 = __builtin_msa_ld_h((void *) pLocalSrc2_msa++, 0);

			pixels1 = __builtin_msa_subv_h(pixels1, pixels3);
			pixels2 = __builtin_msa_subv_h(pixels2, pixels4);

			__builtin_msa_st_h(pixels1, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_h(pixels2, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc1 = (vx_int16 *) pLocalSrc1_msa;
		pLocalSrc2 = (vx_int16 *) pLocalSrc2_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) temp;
		}
#else // C
		pLocalSrc1 = (vx_int16 *) pSrcImage1;
		pLocalSrc2 = (vx_int16 *) pSrcImage2;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			vx_int32 temp = (vx_int32) (*pLocalSrc1++) - (vx_int32) (*pLocalSrc2++);
			*pLocalDst++ = (vx_int16) temp;
		}
#endif
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pDstImage += (dstImageStrideInBytes >> 1);
	}

	return AGO_SUCCESS;
}

int HafCpu_Threshold_U1_U8_Binary
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      threshold
	)
{
	vx_uint8 *pLocalSrc, *pLocalDst;
	vx_uint16 pixelmask;
	int height = (int) dstHeight;
#if ENABLE_MSA
	v16u8 *pLocalSrc_msa;
	v16u8 pixels;
	v16u8 thresh = (v16u8) __builtin_msa_fill_b((int) threshold);
	// mask for 2 x 8-bit U8 to U1 conversion
	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16u8 vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	while (height)
	{
#if ENABLE_MSA
		pLocalSrc_msa = (v16u8 *) pSrcImage;
		vx_int16 *pLocalDst_16 = (vx_int16 *) pDstImage;

		// 16 pixels (bits) are processed at a time in the inner loop
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels = (v16u8) __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			pixels = (v16u8) __builtin_msa_clt_u_b(thresh, pixels);

			// Convert U8 to U1
			pixels = __builtin_msa_and_v(pixels, vmask);
			pixels = (v16u8) __builtin_msa_hadd_u_h(pixels, pixels);
			pixels = (v16u8) __builtin_msa_hadd_u_w((v8u16) pixels,(v8u16) pixels);
			pixels = (v16u8) __builtin_msa_hadd_u_d((v4u32) pixels,(v4u32) pixels);
			pixelmask = __builtin_msa_copy_u_d((v2i64) pixels, 1);
			pixelmask = (vx_uint16) pixelmask << 8;
			pixelmask = pixelmask | __builtin_msa_copy_u_d((v2i64) pixels, 0);

			*pLocalDst_16++ = pixelmask;

			width--;
		}
		pLocalSrc = (vx_uint8 *) pLocalSrc_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_16;

		width = 0;
		while (width < postfixWidth)
		{
			pixelmask = 0;
			for (int i = 0; i < 8; i++, width++)
			{
				if (width == postfixWidth)
					break;

				pixelmask >>= 1;
				if (*pLocalSrc++ > threshold)
					pixelmask |= 0x80;
			}
			*pLocalDst++ = (vx_uint8) (pixelmask & 0xFF);
		}
#else
		pLocalDst = (vx_uint8 *) pDstImage;
		pLocalSrc = (vx_uint8 *) pSrcImage;
		int width = 0;
		while (width < dstWidth)
		{
			pixelmask = 0;
			for (int i = 0; i < 8; i++, width++)
			{
				if (width == dstWidth)
					break;

				pixelmask >>= 1;
				if (*pLocalSrc++ > threshold)
					pixelmask |= 0x80;
			}
			*pLocalDst++ = (vx_uint8) (pixelmask & 0xFF);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Mul_U8_U8U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1	= (v16u8 *) pSrcImage1;
		v16u8 *src2	= (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 4);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round to nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_u_w(fpels4);

			// pack to unsigned words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// pack to unsigned bytes
			v4u32 pixels1_temp = (v4u32)__builtin_msa_sat_u_h((v8u16) pixels1, 7);
			v4u32 pixels3_temp = (v4u32)__builtin_msa_sat_u_h((v8u16) pixels3, 7);
			pixels1 = (v16u8) __builtin_msa_pckev_b((v16i8) pixels3_temp, (v16i8) pixels1_temp);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned char *dst = (unsigned char *) pchDst;
		unsigned char *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (unsigned char) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_U8_U8U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1	= (v16u8 *) pSrcImage1;
		v16u8 *src2	= (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 4);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero
			pixels1 = (v16u8) __builtin_msa_ftrunc_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_u_w(fpels4);

			// pack to unsigned words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// pack to unsigned bytes
			v4u32 pixels1_temp = (v4u32)__builtin_msa_sat_u_h((v8u16) pixels1, 7);
			v4u32 pixels3_temp = (v4u32)__builtin_msa_sat_u_h((v8u16) pixels3, 7);
			pixels1 = (v16u8) __builtin_msa_pckev_b((v16i8) pixels3_temp, (v16i8) pixels1_temp);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned char *dst = (unsigned char *) pchDst;
		unsigned char *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			vx_int32 temp = (*src1++ * *src2++ * scale);
			temp = max(min(temp, 255), 0);
			*dst++ = (vx_uint8) temp;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_U8_U8U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
#if ENABLE_MSA
	// do generic floating point calculation
	v16u8 pixels1, pixels2, pixels3, pixels4, mask;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_ldi_b((short) 0x00FF);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1	= (v16u8 *) pSrcImage1;
		v16u8 *src2	= (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 4);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round to nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_u_w(fpels4);

			// pack to unsigned words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// mask for wrap/truncation
			// wrap to U8
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			pixels3 = __builtin_msa_and_v(pixels3, mask);

			// pack to unsigned bytes
			pixels1 = (v16u8) __builtin_msa_pckev_b((v16i8) pixels3, (v16i8) pixels1);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned char *dst = (unsigned char *) pchDst;
		unsigned char *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (unsigned char) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

/* The following are hand optimized CPU based kernels for point-multiply functions */
int HafCpu_Mul_U8_U8U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{

#if ENABLE_MSA
	// do generic floating point calculation
	v16u8 pixels1, pixels2, pixels3, pixels4, mask;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_ldi_b((short) 0x00FF);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1	= (v16u8 *) pSrcImage1;
		v16u8 *src2	= (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 4);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero
			pixels1 = (v16u8) __builtin_msa_ftrunc_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_u_w(fpels4);

			// pack to unsigned words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// mask for wrap/truncation
			// wrap to U8
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			// wrap to U8
			pixels3 = __builtin_msa_and_v(pixels3, mask);
			// pack to unsigned bytes
			pixels1 = (v16u8) __builtin_msa_pckev_b((v16i8) pixels3, (v16i8) pixels1);
			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned char *dst = (unsigned char *) pchDst;
		unsigned char *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (unsigned char) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_U8U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
	v8i16 maxVal = __builtin_msa_fill_h(SHRT_MAX);
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1 = (v16u8 *) pSrcImage1;
		v16u8 *src2 = (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero
			pixels1 = (v16u8) __builtin_msa_ftrunc_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_u_w(fpels4);

			// pack to unsigned words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			v8i16 pixels1_temp = (v8i16)__builtin_msa_min_u_h((v8u16) maxVal, (v8u16) pixels1);
			v8i16 pixels3_temp = (v8i16)__builtin_msa_min_u_h((v8u16) maxVal, (v8u16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1_temp, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3_temp, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			vx_int32 temp = (*src1++ * *src2++ * scale);
			temp = max(min(temp, SHRT_MAX), SHRT_MIN);
			*dst++ = (vx_int16) temp;
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_U8U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, mask;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 * src1 = (v16u8 *) pSrcImage1;
		v16u8 * src2 = (v16u8 *) pSrcImage2;
		v16u8 * dst = (v16u8 *) pchDst;
		v16u8 * dstlast = dst + (dstWidth >> 3);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);
			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero
			pixels1 = (v16u8) __builtin_msa_ftrunc_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_u_w(fpels4);

			// mask for wrap/truncation
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			pixels2 = __builtin_msa_and_v(pixels2, mask);
			pixels3 = __builtin_msa_and_v(pixels3, mask);
			pixels4 = __builtin_msa_and_v(pixels4, mask);

			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned short *dst = (unsigned short *) pchDst;
		unsigned short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (unsigned short) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_U8U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1	= (v16u8 *) pSrcImage1;
		v16u8 *src2	= (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round to nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_u_w(fpels4);

			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (short) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_U8U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, mask;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 * src1 = (v16u8 *) pSrcImage1;
		v16u8 * src2 = (v16u8 *) pSrcImage2;
		v16u8 * dst = (v16u8* ) pchDst;
		v16u8 * dstlast = dst + (dstWidth >> 3);
		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels3 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels1);
			pixels1 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels1);
			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// src1*src2 for (8-15)
			pixels3 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// src1*src2 for (0-7)
			pixels1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);
			pixels4 = pixels3;
			pixels2 = pixels1;

			// convert to 32 bit0
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round to nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_u_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_u_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_u_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_u_w(fpels4);

			// mask for wrap/truncation
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			pixels2 = __builtin_msa_and_v(pixels2, mask);
			pixels3 = __builtin_msa_and_v(pixels3, mask);
			pixels4 = __builtin_msa_and_v(pixels4, mask);

			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
#else // C
		unsigned char *src1 = (unsigned char *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		unsigned short *dst = (unsigned short *) pchDst;
		unsigned short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (unsigned short) (*src1++ * *src2++ * scale);
		}
#endif
		pSrcImage1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, mask, temp1, temp2;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
	v4i32 temp_l, temp_r, diff_l, diff_r;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1 = (v16u8 *) pSrc1;
		v16u8 *src2 = (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);
		while (dst < dstlast)
		{
			// src1 (0-7)
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src1 (8-15)
			pixels3 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src2 (0-15)
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// multiply low for src1*src2 for (8-15)
			temp1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// multiply low for src1*src2 for (0-7)
			temp2 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			// multiply high for src1*src2 for (8-15)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels3 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// multiply high for src1*src2 for (0-7)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels1 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// unpack to 32 bit result
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_s_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_s_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_s_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_s_w(fpels4);

			// pack to words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
		pSrc1	+= srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (short) (*src1++ * (short) *src2++ * scale);
		}

		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, mask, temp1, temp2;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v4f32 fscale = {scale, scale, scale, scale};
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 * src1 = (v16u8 *) pSrc1;
		v16u8 * src2 = (v16u8 *) pSrcImage2;
		v16u8 * dst = (v16u8 *) pchDst;
		v16u8 * dstlast = dst + (dstWidth >> 3);
		while (dst < dstlast)
		{
			// src1 (0-7)
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src1 (8-15)
			pixels3 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src2 (0-15)
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// multiply low for src1*src2 for (8-15)
			temp1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// multiply low for src1*src2 for (0-7)
			temp2 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			// multiply high for src1*src2 for (8-15)
			v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
			v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
			v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels3 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// multiply high for src1*src2 for (0-7)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels1 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// unpack to 32 bit result
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero - use convert with truncation: cvttps2dq
			pixels1 = (v16u8) __builtin_msa_ftrunc_s_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_s_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_s_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_s_w(fpels4);

			// mask for wrap/truncation
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			pixels2 = __builtin_msa_and_v(pixels2, mask);
			pixels3 = __builtin_msa_and_v(pixels3, mask);
			pixels4 = __builtin_msa_and_v(pixels4, mask);

			// pack signed saturation
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}

		pSrc1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (short) (*src1++ * (short) *src2++ * scale);
		}

		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#endif

	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, mask, temp1, temp2;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v4f32 fscale = {scale, scale, scale, scale};
	v4i32 temp_l, temp_r, diff_l, diff_r;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1 = (v16u8 *) pSrc1;
		v16u8 *src2 = (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);
		while (dst < dstlast)
		{
			// src1 (0-7)
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src1 (8-15)
			pixels3 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			// src2 (0-15)
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// multiply low for src1*src2 for (8-15)
			temp1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// multiply low for src1*src2 for (0-7)
			temp2 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			// multiply high for src1*src2 for (8-15)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels3 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// multiply high for src1*src2 for (0-7)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels1 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// unpack to 32 bit result
			// src1*src2 (4-7)
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (0-3)
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
			// src1*src2 (12-15)
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
			// src1*src2 (8-11)
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards nearest even
			pixels1 = (v16u8) __builtin_msa_ftint_s_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftint_s_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftint_s_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftint_s_w(fpels4);

			// mask for wrap/truncation
			pixels1 = __builtin_msa_and_v(pixels1, mask);
			pixels2 = __builtin_msa_and_v(pixels2, mask);
			pixels3 = __builtin_msa_and_v(pixels3, mask);
			pixels4 = __builtin_msa_and_v(pixels4, mask);

			// pack to words
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
		pSrc1	+= srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (short) (*src1++ * (short) *src2++ * scale);
		}

		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16u8 pixels1, pixels2, pixels3, pixels4, temp1, temp2;
	v4f32 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v4f32 fscale = {scale, scale, scale, scale};
	v4i32 temp_l, temp_r, diff_l, diff_r, temp0_i, temp1_i;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1 = (v16u8 *) pSrc1;
		v16u8 *src2 = (v16u8 *) pSrcImage2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);

		while (dst < dstlast)
		{
			pixels1 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels3 = (v16u8) __builtin_msa_ld_b(src1++, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b(src2++, 0);

			pixels4 = (v16u8) __builtin_msa_ilvl_b(zeros, (v16i8) pixels2);
			pixels2 = (v16u8) __builtin_msa_ilvr_b(zeros, (v16i8) pixels2);

			// multiply low for src1*src2 for (8-15)
			temp1 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
			// multiply low for src1*src2 for (0-7)
			temp2 = (v16u8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

			// multiply high for src1*src2 for (8-15)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels3 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// multiply high for src1*src2 for (0-7)
			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
			diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
			diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
			temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
			temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
			pixels1 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			// unpack to 32 bit result
			pixels2 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
			pixels1 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
			pixels4 = (v16u8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
			pixels3 = (v16u8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

			// convert to packed single precision float of src1*src2
			fpels1 = __builtin_msa_ffint_s_w((v4i32) pixels1);
			fpels2 = __builtin_msa_ffint_s_w((v4i32) pixels2);
			fpels3 = __builtin_msa_ffint_s_w((v4i32) pixels3);
			fpels4 = __builtin_msa_ffint_s_w((v4i32) pixels4);

			// multiply with scale
			fpels1 = __builtin_msa_fmul_w(fpels1, fscale);
			fpels2 = __builtin_msa_fmul_w(fpels2, fscale);
			fpels3 = __builtin_msa_fmul_w(fpels3, fscale);
			fpels4 = __builtin_msa_fmul_w(fpels4, fscale);

			// round towards zero
			pixels1 = (v16u8) __builtin_msa_ftrunc_s_w(fpels1);
			pixels2 = (v16u8) __builtin_msa_ftrunc_s_w(fpels2);
			pixels3 = (v16u8) __builtin_msa_ftrunc_s_w(fpels3);
			pixels4 = (v16u8) __builtin_msa_ftrunc_s_w(fpels4);

			// pack signed saturation
			temp0_i = __builtin_msa_sat_s_w((v4i32) pixels1, 15);
			temp1_i = __builtin_msa_sat_s_w((v4i32) pixels2, 15);
			pixels1 = (v16u8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

			temp0_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels3, 15);
			temp1_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels4, 15);
			pixels3 = (v16u8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

			// copy to dest
			__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
			__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
		}
		pSrc1 += srcImage1StrideInBytes;
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		unsigned char *src2 = (unsigned char *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;
		vx_int32 temp;

		while (dst < dstlast)
		{
			temp = (*src1++ * (short) *src2++ * scale);
			*dst++ = max(min(temp, INT16_MAX), INT16_MIN);
		}
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16S16_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16i8 pixels1, pixels2, pixels3, pixels4, temp1, temp2, pixels_tmp;
	v16u8 mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v2f64 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v2f64 fscale = {scale, scale};
	v4i32 temp_32, signmask;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;
	unsigned char *pSrc2 = (unsigned char *) pSrcImage2;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 *src1 = (v16u8 *) pSrc1;
		v16u8 *src2 = (v16u8 *) pSrc2;
		v16u8 *dst = (v16u8 *) pchDst;
		v16u8 *dstlast = dst + (dstWidth >> 3);

		if (scale == 1.0f){
			while (dst < dstlast)
			{
				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// mask for wrap/truncation
				pixels1 = (v16i8) __builtin_msa_and_v((v16u8) pixels1, mask);
				pixels2 = (v16i8) __builtin_msa_and_v((v16u8) pixels2, mask);
				pixels3 = (v16i8) __builtin_msa_and_v((v16u8) pixels3, mask);
				pixels4 = (v16i8) __builtin_msa_and_v((v16u8) pixels4, mask);

				// pack to words
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

				// copy to dest
				__builtin_msa_st_b(pixels1, (void *) dst++, 0);
				__builtin_msa_st_b(pixels3, (void *) dst++, 0);
			}
		}
		else
		{
			while (dst < dstlast)
			{
				v2f64 fpels5, fpels6, fpels7, fpels8;
				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// convert to packed double precision float of src1*src2
				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels1);
				fpels1 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels2);
				fpels2 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels3);
				fpels3 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels4);
				fpels4 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels1);
				fpels5 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels2);
				fpels6 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels3);
				fpels7 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels4);
				fpels8 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				// multiply with scale
				fpels1 = __builtin_msa_fmul_d(fpels1, fscale);
				fpels2 = __builtin_msa_fmul_d(fpels2, fscale);
				fpels3 = __builtin_msa_fmul_d(fpels3, fscale);
				fpels4 = __builtin_msa_fmul_d(fpels4, fscale);
				fpels5 = __builtin_msa_fmul_d(fpels5, fscale);
				fpels6 = __builtin_msa_fmul_d(fpels6, fscale);
				fpels7 = __builtin_msa_fmul_d(fpels7, fscale);
				fpels8 = __builtin_msa_fmul_d(fpels8, fscale);

				// round towards zero
				pixels1 = (v16i8) __builtin_msa_ftrunc_s_d(fpels1);
				pixels1 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels1);

				pixels2 = (v16i8) __builtin_msa_ftrunc_s_d(fpels2);
				pixels2 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels2);

				pixels3 = (v16i8) __builtin_msa_ftrunc_s_d(fpels3);
				pixels3 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels3);

				pixels4 = (v16i8) __builtin_msa_ftrunc_s_d(fpels4);
				pixels4 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels4);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels5);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels1 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels1);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels6);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels2 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels2);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels7);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels3 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels3);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels8);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels4 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels4);

				// mask for wrap/truncation
				pixels1 = (v16i8) __builtin_msa_and_v((v16u8) pixels1, mask);
				pixels2 = (v16i8) __builtin_msa_and_v((v16u8) pixels2, mask);
				pixels3 = (v16i8) __builtin_msa_and_v((v16u8) pixels3, mask);
				pixels4 = (v16i8) __builtin_msa_and_v((v16u8) pixels4, mask);

				// pack to words
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

				// copy to dest
				__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
				__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
			}
		}
		pSrc1 += srcImage1StrideInBytes;
		pSrc2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;

#else // C
		short *src1 = (short *) pSrcImage1;
		short *src2 = (short *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (vx_int16) ((vx_float64) (*(vx_int16 *) src1++ * *(vx_int16 *) src2++) * scale);
		}
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16S16_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16i8 pixels1, pixels2, pixels3, pixels4, temp1, temp2, pixels_tmp;
	v2f64 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v16u8 mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v2f64 fscale = {scale, scale};
	v4i32 temp_32, signmask, temp0_i, temp1_i;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;
	unsigned char *pSrc2 = (unsigned char *) pSrcImage2;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 * src1 = (v16u8 *) pSrc1;
		v16u8 * src2 = (v16u8 *) pSrc2;
		v16u8 * dst = (v16u8 *) pchDst;
		v16u8 * dstlast = dst + (dstWidth >> 3);
		if (scale == 1.0f){
			while (dst < dstlast)
			{
				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// pack to words with saturation
				temp0_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels1, 15);
				temp1_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels2, 15);
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

				temp0_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels3, 15);
				temp1_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels4, 15);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

				// copy to dest
				__builtin_msa_st_b(pixels1, (void *) dst++, 0);
				__builtin_msa_st_b(pixels3, (void *) dst++, 0);
			}
		}
		else
		{
			while (dst < dstlast)
			{
				v2f64 fpels5, fpels6, fpels7, fpels8;
				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// convert to packed double precision float of src1*src2
				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels1);
				fpels1 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels2);
				fpels2 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels3);
				fpels3 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels4);
				fpels4 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels1);
				fpels5 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels2);
				fpels6 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels3);
				fpels7 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels4);
				fpels8 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				// multiply with scale
				fpels1 = __builtin_msa_fmul_d(fpels1, fscale);
				fpels2 = __builtin_msa_fmul_d(fpels2, fscale);
				fpels3 = __builtin_msa_fmul_d(fpels3, fscale);
				fpels4 = __builtin_msa_fmul_d(fpels4, fscale);
				fpels5 = __builtin_msa_fmul_d(fpels5, fscale);
				fpels6 = __builtin_msa_fmul_d(fpels6, fscale);
				fpels7 = __builtin_msa_fmul_d(fpels7, fscale);
				fpels8 = __builtin_msa_fmul_d(fpels8, fscale);

				// round towards zero
				pixels1 = (v16i8) __builtin_msa_ftrunc_s_d(fpels1);
				pixels1 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels1);

				pixels2 = (v16i8) __builtin_msa_ftrunc_s_d(fpels2);
				pixels2 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels2);

				pixels3 = (v16i8) __builtin_msa_ftrunc_s_d(fpels3);
				pixels3 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels3);

				pixels4 = (v16i8) __builtin_msa_ftrunc_s_d(fpels4);
				pixels4 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels4);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels5);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels1 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels1);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels6);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels2 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels2);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels7);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels3 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels3);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels8);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels4 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels4);

				// pack signed saturation
				temp0_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels1, 15);
				temp1_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels2, 15);
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

				temp0_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels3, 15);
				temp1_i = (v4i32)__builtin_msa_sat_s_w((v4i32) pixels4, 15);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_i, (v8i16) temp0_i);

				// copy to dest
				__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
				__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
			}
		}
		pSrc1 += srcImage1StrideInBytes;
		pSrc2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		short *src2 = (short *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;
		vx_int32 temp;

		while (dst < dstlast)
		{
			temp = ((vx_float64) (*(vx_int16 *) src1++ * *(vx_int16 *) src2++) * scale);
			*dst++ = max(min(temp, INT16_MAX), INT16_MIN);
		}
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_Mul_S16_S16S16_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	)
{
	// do generic floating point calculation
#if ENABLE_MSA
	v16i8 pixels1, pixels2, pixels3, pixels4, temp1, temp2, pixels_tmp;
	v2f64 fpels1, fpels2, fpels3, fpels4;
	v16i8 zeros = __builtin_msa_ldi_b(0);
	v16u8 mask = (v16u8) __builtin_msa_fill_w((int) 0x0000FFFF);
	v2f64 fscale = {scale, scale};
	v4i32 temp_32, signmask;
#endif
	unsigned char *pchDst = (unsigned char *) pDstImage;
	unsigned char *pchDstlast = (unsigned char *) pDstImage + dstHeight * dstImageStrideInBytes;
	unsigned char *pSrc1 = (unsigned char *) pSrcImage1;
	unsigned char *pSrc2 = (unsigned char *) pSrcImage2;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16u8 * src1 = (v16u8 *) pSrc1;
		v16u8 * src2 = (v16u8 *) pSrc2;
		v16u8 * dst = (v16u8 *) pchDst;
		v16u8 * dstlast = dst + (dstWidth >> 3);
		if (scale == 1.0f){
			while (dst < dstlast)
			{
				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// mask for wrap/truncation
				pixels1 = (v16i8) __builtin_msa_and_v((v16u8) pixels1, mask);
				pixels2 = (v16i8) __builtin_msa_and_v((v16u8) pixels2, mask);
				pixels3 = (v16i8) __builtin_msa_and_v((v16u8) pixels3, mask);
				pixels4 = (v16i8) __builtin_msa_and_v((v16u8) pixels4, mask);

				// pack to words
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

				// copy to dest
				__builtin_msa_st_b(pixels1, (void *) dst++, 0);
				__builtin_msa_st_b(pixels3, (void *) dst++, 0);
			}
		}
		else
		{
			while (dst < dstlast)
			{
				v2f64 fpels5, fpels6, fpels7, fpels8;

				// src1 (0-7)
				pixels1 = __builtin_msa_ld_b(src1++, 0);
				// src1 (8-15)
				pixels3 = __builtin_msa_ld_b(src1++, 0);
				// src2 (0-7)
				pixels2 = __builtin_msa_ld_b(src2++, 0);
				// src2 (8-15)
				pixels4 = __builtin_msa_ld_b(src2++, 0);

				// multiply low for src1*src2 for (8-15)
				temp1 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels3, (v8i16) pixels4);
				// multiply low for src1*src2 for (0-7)
				temp2 = (v16i8) __builtin_msa_mulv_h((v8i16) pixels1, (v8i16) pixels2);

				// multiply high for src1*src2 for (8-15)
				v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels3);
				v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels4);
				v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels4);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels3 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// multiply high for src1*src2 for (0-7)
				temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels1);
				temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels1);
				diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeros, (v8i16) pixels2);
				diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeros, (v8i16) pixels2);
				temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
				temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
				pixels1 = (v16i8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

				// unpack to 32 bit result
				// src1*src2 (4-7)
				pixels2 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (0-3)
				pixels1 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels1, (v8i16) temp2);
				// src1*src2 (12-15)
				pixels4 = (v16i8) __builtin_msa_ilvl_h((v8i16) pixels3, (v8i16) temp1);
				// src1*src2 (8-11)
				pixels3 = (v16i8) __builtin_msa_ilvr_h((v8i16) pixels3, (v8i16) temp1);

				// convert to packed double precision float of src1*src2
				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels1);
				fpels1 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels2);
				fpels2 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels3);
				fpels3 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvr_w(signmask, (v4i32) pixels4);
				fpels4 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels1, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels1);
				fpels5 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels2, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels2);
				fpels6 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels3, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels3);
				fpels7 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				signmask = __builtin_msa_clti_s_w((v4i32) pixels4, 0);
				temp_32 = __builtin_msa_ilvl_w(signmask, (v4i32) pixels4);
				fpels8 = __builtin_msa_ffint_s_d((v2i64) temp_32);

				// multiply with scale
				fpels1 = __builtin_msa_fmul_d(fpels1, fscale);
				fpels2 = __builtin_msa_fmul_d(fpels2, fscale);
				fpels3 = __builtin_msa_fmul_d(fpels3, fscale);
				fpels4 = __builtin_msa_fmul_d(fpels4, fscale);
				fpels5 = __builtin_msa_fmul_d(fpels5, fscale);
				fpels6 = __builtin_msa_fmul_d(fpels6, fscale);
				fpels7 = __builtin_msa_fmul_d(fpels7, fscale);
				fpels8 = __builtin_msa_fmul_d(fpels8, fscale);

				// round towards zero
				pixels1 = (v16i8) __builtin_msa_ftrunc_s_d(fpels1);
				pixels1 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels1);

				pixels2 = (v16i8) __builtin_msa_ftrunc_s_d(fpels2);
				pixels2 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels2);

				pixels3 = (v16i8) __builtin_msa_ftrunc_s_d(fpels3);
				pixels3 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels3);

				pixels4 = (v16i8) __builtin_msa_ftrunc_s_d(fpels4);
				pixels4 = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels4);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels5);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels1 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels1);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels6);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels2 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels2);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels7);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels3 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels3);

				pixels_tmp = (v16i8) __builtin_msa_ftrunc_s_d(fpels8);
				pixels_tmp = (v16i8) __builtin_msa_pckev_w((v4i32) zeros, (v4i32) pixels_tmp);
				pixels4 = (v16i8) __builtin_msa_pckev_d((v2i64) pixels_tmp, (v2i64) pixels4);

				// mask for wrap/truncation
				pixels1 = (v16i8) __builtin_msa_and_v((v16u8) pixels1, mask);
				pixels2 = (v16i8) __builtin_msa_and_v((v16u8) pixels2, mask);
				pixels3 = (v16i8) __builtin_msa_and_v((v16u8) pixels3, mask);
				pixels4 = (v16i8) __builtin_msa_and_v((v16u8) pixels4, mask);

				// pack signed saturation
				pixels1 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels2, (v8i16) pixels1);
				pixels3 = (v16i8) __builtin_msa_pckev_h((v8i16) pixels4, (v8i16) pixels3);

				// copy to dest
				__builtin_msa_st_b((v16i8) pixels1, (void *) dst++, 0);
				__builtin_msa_st_b((v16i8) pixels3, (void *) dst++, 0);
			}
		}
		pSrc1 += srcImage1StrideInBytes;
		pSrc2 += srcImage2StrideInBytes;
		pchDst += dstImageStrideInBytes;
#else // C
		short *src1 = (short *) pSrcImage1;
		short *src2 = (short *) pSrcImage2;
		short *dst = (short *) pchDst;
		short *dstlast = dst + dstWidth;

		while (dst < dstlast)
		{
			*dst++ = (vx_int16) ((vx_float64) (*(vx_int16 *) src1++ * *(vx_int16 *) src2++) * scale);
		}
		pSrcImage1 += (srcImage1StrideInBytes >> 1);
		pSrcImage2 += (srcImage2StrideInBytes >> 1);
		pchDst += dstImageStrideInBytes;
#endif
	}

	return AGO_SUCCESS;
}

int HafCpu_ColorDepth_U8_S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	)
{
#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v8i16 shift_v = __builtin_msa_fill_h(shift);
	v16i8 maskL = {(char) 0x0,  (char) 0x02, (char) 0x04, (char) 0x06,
				   (char) 0x08, (char) 0x0A, (char) 0x0C, (char) 0x0E,
				   (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
				   (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF};
	v16i8 maskH = {(char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
				   (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
				   (char) 0x0,  (char) 0x02, (char) 0x04, (char) 0x06,
				   (char) 0x08, (char) 0x0A, (char) 0x0C, (char) 0x0E};
	v16i8 pixels1, pixels2;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		vx_int16 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

		for (int width = 0; width < prefixWidth; width++)
		{
			int pix = (int) (*pLocalSrc++);
			*pLocalDst++ = (vx_uint8) ((pix >> shift) & 0xFF);
		}

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixels1 = __builtin_msa_ld_b(pLocalSrc, 0);
			pixels2 = __builtin_msa_ld_b((pLocalSrc + 8), 0);

			pixels1 = (v16i8) __builtin_msa_sra_h((v8i16) pixels1, shift_v);
			pixels2 = (v16i8) __builtin_msa_sra_h((v8i16) pixels2, shift_v);

			pixels1 = __builtin_msa_vshf_b(maskL, (v16i8) pixels1, (v16i8) pixels1);
			pixels2 = __builtin_msa_vshf_b(maskH, (v16i8) pixels2, (v16i8) pixels2);

			pixels1 = (v16i8) __builtin_msa_or_v((v16u8) pixels1, (v16u8) pixels2);
			__builtin_msa_st_b(pixels1, (void *) pLocalDst, 0);

			pLocalSrc += 16;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			int pix = *pLocalSrc++;
			*pLocalDst++ = (vx_uint8) ((pix >> shift) & 0xFF);
		}
#else // C
		vx_int16 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			int pix = *pLocalSrc++;
			*pLocalDst++ = (vx_uint8) ((pix >> shift) & 0xFF);
		}
#endif
		pSrcImage += (srcImageStrideInBytes >> 1);
		pDstImage += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_ColorDepth_S16_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	)
{

#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstImage) & 7;			// Two bytes in output = 1 pixel
	prefixWidth = (prefixWidth == 0) ? 0 : (8 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v8i16 shift_v = __builtin_msa_fill_h(shift);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v16i8 pixelsL, pixelsH;
#endif

	for (int height = 0; height < (int) dstHeight; height++)

	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_int16 * pLocalDst = pDstImage;
#if ENABLE_MSA

		for (int width = 0; width < prefixWidth; width++)
		{
			int pix = (int) (*pLocalSrc++);
			*pLocalDst++ = (vx_int16) (pix << shift);
		}

		for (int width = 0; width < alignedWidth; width += 16)
		{
			pixelsL = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			pixelsH = __builtin_msa_ilvl_b(zeromask, pixelsL);
			pixelsL = __builtin_msa_ilvr_b(zeromask, pixelsL);
			pixelsH = (v16i8) __builtin_msa_sll_h((v8i16) pixelsH, shift_v);
			pixelsL = (v16i8) __builtin_msa_sll_h((v8i16) pixelsL, shift_v);
			__builtin_msa_st_b((v16i8) pixelsL, (void *) pLocalDst, 0);
			__builtin_msa_st_b((v16i8) pixelsH, (void *) (pLocalDst + 8), 0);

			pLocalSrc += 16;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			int pix = (int) (*pLocalSrc++);
			*pLocalDst++ = (vx_int16) (pix << shift);
		}
#else	//C
		for (int width = 0; width < (int) dstWidth; width++)
		{
			int pix = (int) (*pLocalSrc++);
			*pLocalDst++ = (vx_int16) (pix << shift);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}

	return AGO_SUCCESS;
}

int HafCpu_ColorDepth_U8_S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	)

{

#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v8i16 shift_v = __builtin_msa_fill_h(shift);
	v16i8 pixels1, pixels2;
	v8u16 min_255 = (v8u16) __builtin_msa_fill_h(255);
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_int16 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		for (int width = 0; width < prefixWidth; width++)
		{
			int pix = (int) (*pLocalSrc++);
			pix >>= shift;
			pix = min(max(pix, 0), 255);
			*pLocalDst++ = (vx_uint8) (pix);
		}

		for (int width = 0; width < (int) alignedWidth; width += 16)
		{

			pixels1 = __builtin_msa_ld_b((void *) pLocalSrc, 0);
			pixels2 = __builtin_msa_ld_b((void *) (pLocalSrc + 8), 0);
			pixels1 = (v16i8) __builtin_msa_sra_h( (v8i16) pixels1, shift_v);
			pixels2 = (v16i8) __builtin_msa_sra_h( (v8i16) pixels2, shift_v);
			pixels1 = (v16i8) __builtin_msa_maxi_s_h((v8i16) pixels1, 0);
			pixels1 = (v16i8) __builtin_msa_min_u_h((v8u16) pixels1, min_255);
			pixels2 = (v16i8) __builtin_msa_maxi_s_h((v8i16) pixels2, 0);
			pixels2 = (v16i8) __builtin_msa_min_u_h((v8u16) pixels2, min_255);
			pixels1 = (v16i8) __builtin_msa_pckev_b((v16i8) pixels2, (v16i8) pixels1);
			__builtin_msa_st_b(pixels1, (void *) pLocalDst, 0);

			pLocalSrc += 16;
			pLocalDst += 16;

		}

		for (int width = 0; width < postfixWidth; width++)
		{
			int pix = *pLocalSrc++;
			pix >>= shift;
			pix = min(max(pix, 0), 255);
			*pLocalDst++ = (vx_uint8) (pix);
		}
#else // C
		for (int width = 0; width < (int) dstWidth; width++)
		{
			int pix = *pLocalSrc++;
			pix >>= shift;
			pix = min(max(pix, 0), 255);
			*pLocalDst++ = (vx_uint8) (pix);
		}
#endif

		pSrcImage += (srcImageStrideInBytes >> 1);
		pDstImage += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_Accumulate_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	vx_uint8 *pLocalSrc;
	vx_int16 *pLocalDst;

#if ENABLE_MSA
	v16i8 *pLocalSrc_msa, *pLocalDst_msa;
	v16i8 resultL, resultH, pixelsL, pixelsH;
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc_msa = (v16i8 *) pSrcImage;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			resultL = __builtin_msa_ld_b(pLocalDst_msa, 0);
			resultH = __builtin_msa_ld_b((pLocalDst_msa + 1), 0);
			pixelsL = __builtin_msa_ld_b(pLocalSrc_msa++, 0);

			pixelsH = __builtin_msa_ilvl_b(zeromask, (v16i8) pixelsL);
			pixelsL = __builtin_msa_ilvr_b(zeromask, (v16i8) pixelsL);

			resultL = (v16i8) __builtin_msa_adds_s_h((v8i16) resultL, (v8i16) pixelsL);
			resultH = (v16i8) __builtin_msa_adds_s_h((v8i16) resultH, (v8i16) pixelsH);

			__builtin_msa_st_b(resultL, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(resultH, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc = (vx_uint8 *) pLocalSrc_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++, pLocalSrc++)
		{
			vx_int32 temp = (vx_int32) *pLocalDst + (vx_int32) *pLocalSrc;
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#else // C
		pLocalSrc = (vx_uint8 *) pSrcImage;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++, pLocalSrc++)
		{
			vx_int32 temp = (vx_int32) *pLocalDst + (vx_int32) *pLocalSrc;
			*pLocalDst++ = (vx_int16) max(min(temp, INT16_MAX), INT16_MIN);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
	}

	return AGO_SUCCESS;
}

int HafCpu_AccumulateSquared_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint32     shift
	)
{
	vx_uint8 *pLocalSrc;
	vx_int16 *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc_msa, *pLocalDst_msa;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v16i8 resultHH, resultHL, resultLH, resultLL, pixelsHH, pixelsHL, pixelsLH, pixelsLL;
	v4i32 shift_v = __builtin_msa_fill_w(shift);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif
	int height = (int) dstHeight;
	while (height)
	{
#if ENABLE_MSA
		pLocalSrc_msa = (v16i8 *) pSrcImage;
		pLocalDst_msa = (v16i8 *) pDstImage;

		// 16 pixels at a time
		int width = alignedWidth >> 4;
		while (width)
		{
			pixelsLL = __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			resultLL = __builtin_msa_ld_b(pLocalDst_msa, 0);
			resultHL = __builtin_msa_ld_b(pLocalDst_msa + 1, 0);

			// Convert input to 32 bit
			pixelsHL = __builtin_msa_ilvl_b(zeromask, pixelsLL);
			pixelsHH = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) pixelsHL);
			pixelsHL = __builtin_msa_ilvr_b(zeromask, pixelsHL);
			pixelsLL = __builtin_msa_ilvr_b(zeromask, pixelsLL);
			pixelsLH = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) pixelsLL);
			pixelsLL = __builtin_msa_ilvr_b(zeromask, pixelsLL);

			// Convert result to 32 bit
			v8i16 signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) resultHL, 0);
			resultHH = (v16i8) __builtin_msa_ilvl_h(signmask, (v8i16) resultHL);
			resultHL = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) resultHL);

			signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) resultLL, 0);
			resultLH = (v16i8) __builtin_msa_ilvl_h(signmask, (v8i16) resultLL);
			resultLL = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) resultLL);

			// Multiply
			pixelsHH = (v16i8) __builtin_msa_mulv_w((v4i32) pixelsHH, (v4i32) pixelsHH);
			pixelsHL = (v16i8) __builtin_msa_mulv_w((v4i32) pixelsHL, (v4i32) pixelsHL);
			pixelsLH = (v16i8) __builtin_msa_mulv_w((v4i32) pixelsLH, (v4i32) pixelsLH);
			pixelsLL = (v16i8) __builtin_msa_mulv_w((v4i32) pixelsLL, (v4i32) pixelsLL);

			pixelsHH = (v16i8) __builtin_msa_sra_w((v4i32) pixelsHH, shift_v);
			pixelsHL = (v16i8) __builtin_msa_sra_w((v4i32) pixelsHL, shift_v);
			pixelsLH = (v16i8) __builtin_msa_sra_w((v4i32) pixelsLH, shift_v);
			pixelsLL = (v16i8) __builtin_msa_sra_w((v4i32) pixelsLL, shift_v);

			resultHH = (v16i8) __builtin_msa_addv_w((v4i32) resultHH, (v4i32) pixelsHH);
			resultHL = (v16i8) __builtin_msa_addv_w((v4i32) resultHL, (v4i32) pixelsHL);
			resultLH = (v16i8) __builtin_msa_addv_w((v4i32) resultLH, (v4i32) pixelsLH);
			resultLL = (v16i8) __builtin_msa_addv_w((v4i32) resultLL, (v4i32) pixelsLL);

			v4i32 temp0_u = __builtin_msa_sat_s_w((v4i32) resultHL, 15);
			v4i32 temp1_u = __builtin_msa_sat_s_w((v4i32) resultHH, 15);
			resultHL = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			temp0_u = __builtin_msa_sat_s_w((v4i32) resultLL, 15);
			temp1_u = __builtin_msa_sat_s_w((v4i32) resultLH, 15);
			resultLL = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			__builtin_msa_st_b(resultLL, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(resultHL, (void *) pLocalDst_msa++, 0);

			width--;
		}

		pLocalSrc = (vx_uint8 *) pLocalSrc_msa;
		pLocalDst = (vx_int16 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++, pLocalSrc++)
		{
			vx_int32 temp = ((vx_int32) *pLocalSrc * (vx_int32) *pLocalSrc) >> shift;
			temp += (vx_int32) *pLocalDst;
			temp = max(min(temp, (vx_int32) INT16_MAX), (vx_int32) INT16_MIN);
			*pLocalDst++ = (vx_int16) temp;
		}
#else // C
		pLocalSrc = (vx_uint8 *) pSrcImage;
		pLocalDst = (vx_int16 *) pDstImage;

		for (int width = 0; width < dstWidth; width++, pLocalSrc++)
		{
			vx_int32 temp = ((vx_int32) *pLocalSrc * (vx_int32) *pLocalSrc) >> shift;
			temp += (vx_int32) *pLocalDst;
			temp = max(min(temp, (vx_int32) INT16_MAX), (vx_int32) INT16_MIN);
			*pLocalDst++ = (vx_int16) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_AccumulateWeighted_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_float32    alpha
	)
{
	vx_uint8 *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16i8 *pLocalSrc_msa, *pLocalDst_msa;
	v16i8 pixelsI0;
	v4i32 pixelsI1, tempI;
	v4f32 pixelsF0, pixelsF1, pixelsF2, pixelsF3, temp;
	v16i8 zero = (v16i8) __builtin_msa_ldi_b(0);
	v4f32 a = {alpha, alpha, alpha, alpha};
	v4f32 aprime = {(float) (1.0 - alpha), (float) (1.0 - alpha), (float) (1.0 - alpha), (float) (1.0 - alpha)};

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = dstWidth - alignedWidth;
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if ENABLE_MSA
		pLocalSrc_msa = (v16i8 *) pSrcImage;
		pLocalDst_msa = (v16i8 *) pDstImage;

		for (int width = 0; width < alignedWidth; width += 16)
		{
			// For the input pixels
			pixelsI0 = __builtin_msa_ld_b(pLocalSrc_msa++, 0);

			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, pixelsI0);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			pixelsF0 = __builtin_msa_ffint_s_w(pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 4);
			pixelsI1[3] = 0;

			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			pixelsF1 = __builtin_msa_ffint_s_w(pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 8);
			pixelsI1[2] = 0;
			pixelsI1[3] = 0;

			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			pixelsF2 = __builtin_msa_ffint_s_w(pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 12);
			pixelsI1[1] = 0;
			pixelsI1[2] = 0;
			pixelsI1[3] = 0;

			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			pixelsF3 = __builtin_msa_ffint_s_w(pixelsI1);

			// input * alpha
			pixelsF0 = __builtin_msa_fmul_w(pixelsF0, a);
			pixelsF1 = __builtin_msa_fmul_w(pixelsF1, a);
			pixelsF2 = __builtin_msa_fmul_w(pixelsF2, a);
			pixelsF3 = __builtin_msa_fmul_w(pixelsF3, a);

			// For the output pixels
			pixelsI0 = __builtin_msa_ld_b(pLocalDst_msa, 0);

			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, pixelsI0);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			temp = __builtin_msa_ffint_s_w(pixelsI1);
			// (1 - alpha) * output
			temp = __builtin_msa_fmul_w(temp, aprime);
			// (1 - alpha) * output + alpha * input
			pixelsF0 = __builtin_msa_fadd_w(pixelsF0, temp);

			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 4);
			pixelsI1[3] = 0;
			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			temp = __builtin_msa_ffint_s_w(pixelsI1);
			// (1 - alpha) * output
			temp = __builtin_msa_fmul_w(temp, aprime);
			// (1 - alpha) * output + alpha * input
			pixelsF1 = __builtin_msa_fadd_w(pixelsF1, temp);

			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 8);
			pixelsI1[2] = 0;
			pixelsI1[3] = 0;
			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			temp = __builtin_msa_ffint_s_w(pixelsI1);
			// (1 - alpha) * output
			temp = __builtin_msa_fmul_w(temp, aprime);
			// (1 - alpha) * output + alpha * input
			pixelsF2 = __builtin_msa_fadd_w(pixelsF2, temp);

			pixelsI1 = (v4i32) __builtin_msa_sldi_b(pixelsI0, pixelsI0, 12);
			pixelsI1[1] = 0;
			pixelsI1[2] = 0;
			pixelsI1[3] = 0;
			// Convert to int32
			pixelsI1 = (v4i32) __builtin_msa_ilvr_b(zero, (v16i8) pixelsI1);
			pixelsI1 = (v4i32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixelsI1);
			// Convert to float32
			temp = __builtin_msa_ffint_s_w(pixelsI1);
			// (1 - alpha) * output
			temp = __builtin_msa_fmul_w(temp, aprime);
			// (1 - alpha) * output + alpha * input
			pixelsF3 = __builtin_msa_fadd_w(pixelsF3, temp);

			pixelsI0 = (v16i8) __builtin_msa_ftrunc_s_w(pixelsF0);
			pixelsI1 = __builtin_msa_ftrunc_s_w(pixelsF1);

			// lower 8 values (word)
			v4u32 temp0_u = (v4u32) __builtin_msa_sat_u_h((v8u16) pixelsI0, 15);
			v4u32 temp1_u = (v4u32) __builtin_msa_sat_u_h((v8u16) pixelsI1, 15);
			pixelsI0 = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			pixelsI1 = __builtin_msa_ftrunc_s_w(pixelsF2);
			tempI = __builtin_msa_ftrunc_s_w(pixelsF3);

			// upper 8 values (word)
			temp0_u = (v4u32) __builtin_msa_sat_u_h((v8u16) pixelsI1, 15);
			temp1_u = (v4u32) __builtin_msa_sat_u_h((v8u16) tempI, 15);
			pixelsI1 = (v4i32) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			v8u16 temp0_u8 = (v8u16) __builtin_msa_sat_u_b((v16u8) pixelsI0, 7);
			v8u16 temp1_u8 = (v8u16) __builtin_msa_sat_u_b((v16u8) pixelsI1, 7);
			pixelsI0 = (v16i8) __builtin_msa_pckev_b((v16i8) temp1_u8, (v16i8) temp0_u8);

			__builtin_msa_st_b(pixelsI0, (void *) pLocalDst_msa++, 0);
		}

		pLocalSrc = (vx_uint8 *) pLocalSrc_msa;
		pLocalDst = (vx_uint8 *) pLocalDst_msa;

		for (int width = 0; width < postfixWidth; width++, pLocalSrc++)
		{
			vx_float32 temp = ((1 - alpha) * (vx_float32) *pLocalDst) + (alpha * (vx_float32) *pLocalSrc);
			*pLocalDst++ = (vx_uint8) temp;
		}
#else
		pLocalSrc = (vx_uint8 *) pSrcImage;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++, pLocalSrc++)
		{
			vx_float32 temp = ((1 - alpha) * (vx_float32) *pLocalDst) + (alpha * (vx_float32) *pLocalSrc);
			*pLocalDst++ = (vx_uint8) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}