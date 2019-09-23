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

	for (int height = 0; height < (int) dstHeight; height++)
	{
#if 0 //ENABLE_MSA
#else
		pLocalSrc1 = (vx_uint8 *) pSrcImage1;
		pLocalSrc2 = (vx_uint8 *) pSrcImage2;
		pLocalDst = (vx_uint8 *) pDstImage;

		for (int width = 0; width < dstWidth; width++)
		{
			int temp = (int) (*pLocalSrc1++) + (int) (*pLocalSrc2++);
			*pLocalDst++ = (vx_uint8) max(min((int) temp, UINT8_MAX), 0);
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
