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

	for (unsigned int height = 0; height < dstHeight; height++)
	{
		pLocalGx = (short *) pGxImage;
		pLocalGy = (short *) pGyImage;
		pLocalDst = (short *) pMagImage;

		for (int x = 0; x < dstWidth; x++, pLocalGx++, pLocalGy++)
		{
			float temp = (float) (*pLocalGx * *pLocalGx) + (float) (*pLocalGy * *pLocalGy);
			temp = sqrtf(temp);
			*pLocalDst++ = max(min((vx_int32) temp, INT16_MAX), INT16_MIN);
		}
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
