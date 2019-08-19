#include "ago_internal.h"
#include "mips_internal.h"

#define FP_BITS		16
#define FP_MUL		(1<<FP_BITS)
#define FP_ROUND	(1<<15)

static inline unsigned short Horizontal5x5GaussianFilter_C
	(
		unsigned char * srcImage
	)
{
	return((unsigned short) srcImage[-2] + 4 * (unsigned short) srcImage[-1] +
		   6 * (unsigned short) srcImage[0] + 4 * (unsigned short) srcImage[1] +
		   (unsigned short) srcImage[2]);
}

#if ENABLE_MSA
static inline v8i16 Horizontal3x3GaussianFilter_SampleFirstPixel_MSA
	(
		unsigned char * srcImage
	)
{
	v8i16 shiftedL2, shiftedL1, row, shiftedR2, shiftedR1;
	v8i16 resultH, resultL;
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	// load r[-2] and r[+2]
	shiftedL2 = __builtin_msa_ld_h((v8u16 *) (srcImage - 2), 0);
	shiftedR2 = __builtin_msa_ld_h((v8u16 *) (srcImage + 2), 0);

	// interleave r[-2]
	resultH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL2);
	resultL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL2);

	// interleave r[+2]
	shiftedL2 = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR2);
	shiftedR2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR2);

	// load r[-1]
	shiftedL1 = __builtin_msa_ld_h((v8u16 *) (srcImage - 1), 0);

	// r[-2] + r[2]
	resultH = __builtin_msa_addv_h(resultH, shiftedL2);
	resultL = __builtin_msa_addv_h(resultL, shiftedR2);

	// load r[+1]
	shiftedR1 = __builtin_msa_ld_h((v8u16 *) (srcImage + 1), 0);

	// interleave r[-1]
	shiftedL2 = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL1);
	shiftedL1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL1);

	// laod [r0]
	row = __builtin_msa_ld_h((v8u16 *) (srcImage), 0);

	// interleave r[+1]
	shiftedR2 = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR1);
	shiftedR1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR1);

	// r[-1] + r[1]
	shiftedL2 = __builtin_msa_addv_h(shiftedL2, shiftedR2);
	shiftedL1 = __builtin_msa_addv_h(shiftedL1, shiftedR1);

	// interleave [r0]
	shiftedR1 = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) row);
	row = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) row);

	// r[-1] + r[1] + r[0]
	shiftedL2 = __builtin_msa_addv_h(shiftedL2, shiftedR1);
	shiftedL1 = __builtin_msa_addv_h(shiftedL1, row);

	// 4 * (r[-1] + r[+1] + r[0])
	shiftedL2 = __builtin_msa_slli_h(shiftedL2, 2);
	shiftedL1 = __builtin_msa_slli_h(shiftedL1, 2);

	// 2 * r[0]
	shiftedR1 = __builtin_msa_slli_h(shiftedR1, 1);
	row = __builtin_msa_slli_h(row, 1);

	// 2 * r[0] + 4 * (r[-1] + r[+1] + r[0])
	shiftedL2 = __builtin_msa_addv_h(shiftedL2, shiftedR1);
	shiftedL1 = __builtin_msa_addv_h(shiftedL1, row);

	// r[-2] + r[2] + 2 * r[0] + 4 * (r[-1] + r[+1] + r[0])
	resultH = __builtin_msa_addv_h(resultH, shiftedL2);
	resultL = __builtin_msa_addv_h(resultL, shiftedL1);

	// Select words: 0, 2, 4, 6
	resultL = (v8i16) __builtin_msa_pckev_h((v8i16) resultH, (v8i16) resultL);

	return(resultL);
}
#endif

/* Kernel			1   4	6   4	1			1		1   4	6   4	1
				    4  16  24  16   4			4
			1/256	6  24  36  24	6    =		6									>> 8
					4  16  24  16	4			4
					1   4	6   4	1			1
*/
int HafCpu_ScaleGaussianHalf_U8_U8_5x5
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		bool		  sampleFirstRow,
		bool		  sampleFirstColumn,
		vx_uint8	* pScratch
	)
{
	int alignedDstStride = (dstImageStrideInBytes + 15) & ~15;
	alignedDstStride <<= 2;				// Each row stores two short values (Gx,Gy) for each pixel
	unsigned short *r0 = (unsigned short *) pScratch;
	unsigned short *r1 = (unsigned short *) (pScratch + alignedDstStride);
	unsigned short *r2 = (unsigned short *) (pScratch + 2 * alignedDstStride);
	unsigned short *r3 = (unsigned short *) (pScratch + 3 * alignedDstStride);
	unsigned short *r4 = (unsigned short *) (pScratch + 4 * alignedDstStride);

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
	int srcRowOffset = sampleFirstRow ? 0 : srcImageStrideInBytes;
	int srcColOffset = sampleFirstColumn ? 0 : 1;

	pSrcImage += srcRowOffset;																	// Offset for odd/even row sampling
	unsigned char *pLocalSrc = (unsigned char *) pSrcImage;
	unsigned char *pLocalDst = (unsigned char *) pDstImage;

	unsigned short *pRowMinus2 = r0;
	unsigned short *pRowMinus1 = r1;
	unsigned short *pRowCurr = r2;
	unsigned short *pRowPlus1 = r3;
	unsigned short *pRowPlus2 = r4;


	unsigned short *pLocalRowMinus2 = pRowMinus2;
	unsigned short *pLocalRowMinus1 = pRowMinus1;
	unsigned short *pLocalRowCurr = pRowCurr;
	unsigned short *pLocalRowPlus1 = pRowPlus1;
	unsigned short *pLocalRowPlus2 = pRowPlus2;
	unsigned short *pTemp0, *pTemp1;

	int srcStride = (int) srcImageStrideInBytes;
	pLocalSrc += srcColOffset;

#if ENABLE_MSA
	v8i16 temp0, temp1, temp2, temp3, pixels_plus1H, pixels_plus1L, pixels_plus2H, pixels_plus2L;

	// Process first three rows - Horizontal filtering
	for (int x = 0; x < (prefixWidth << 1); x++, pLocalSrc += 2)
	{
		*pLocalRowMinus2++ = Horizontal5x5GaussianFilter_C(pLocalSrc - (srcStride + srcStride));
		*pLocalRowMinus1++ = Horizontal5x5GaussianFilter_C(pLocalSrc - srcStride);
		*pLocalRowCurr++ = Horizontal5x5GaussianFilter_C(pLocalSrc);
	}
	for (int x = 0; x < (alignedWidth >> 3); x++)
	{
		temp0 = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc - (srcStride + srcStride));
		__builtin_msa_st_h(temp0, (void *) pLocalRowMinus2, 0);

		temp1 = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc - srcStride);
		__builtin_msa_st_h(temp1, (void *) pLocalRowMinus1, 0);

		temp0 = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc);
		__builtin_msa_st_h(temp0, (void *) pLocalRowCurr, 0);

		pLocalSrc += 16;
		pLocalRowMinus2 += 8;
		pLocalRowMinus1 += 8;
		pLocalRowCurr += 8;
	}

	for (int x = 0; x < (postfixWidth << 1); x++, pLocalSrc += 2)
	{
		*pLocalRowMinus2++ = Horizontal5x5GaussianFilter_C(pLocalSrc - (srcStride + srcStride));
		*pLocalRowMinus1++ = Horizontal5x5GaussianFilter_C(pLocalSrc - srcStride);
		*pLocalRowCurr++ = Horizontal5x5GaussianFilter_C(pLocalSrc);
	}
#else
	for (int x = 0; x < (dstWidth << 1); x++, pLocalSrc += 2)
	{
		*pLocalRowMinus2++ = Horizontal5x5GaussianFilter_C(pLocalSrc - (srcStride + srcStride));
		*pLocalRowMinus1++ = Horizontal5x5GaussianFilter_C(pLocalSrc - srcStride);
		*pLocalRowCurr++ = Horizontal5x5GaussianFilter_C(pLocalSrc);
	}
#endif
	pLocalRowMinus2 = pRowMinus2;
	pLocalRowMinus1 = pRowMinus1;
	pLocalRowCurr = pRowCurr;

	// Process rows 4 till the end
	int height = (int) dstHeight;
	while (height)
	{
		pLocalSrc = (unsigned char *) (pSrcImage + srcStride + srcColOffset);			// Pointing to the row below
		unsigned char *pLocalSrc_NextRow = pLocalSrc + srcStride;
		pLocalDst = (unsigned char *) pDstImage;

		for (int x = 0; x < prefixWidth; x++, pLocalSrc += 2)
		{
			short temp_plus1 = Horizontal5x5GaussianFilter_C(pLocalSrc);				// row + 1
			*pLocalRowPlus1++ = temp_plus1;
			short temp_plus2 = Horizontal5x5GaussianFilter_C(pLocalSrc_NextRow);		// row + 2
			*pLocalRowPlus2++ = temp_plus2;

			*pLocalDst++ = (unsigned char) ((*pLocalRowMinus2++ + 4 * (*pLocalRowMinus1++) + 6 * (*pLocalRowCurr++) + 4 * temp_plus1 + temp_plus2) >> 8);
		}

#if ENABLE_MSA


		int width = (int) (alignedWidth >> 4);															// 16 dst pixels processed in one go
		while (width)
		{
			// load c[0]
			temp0 = __builtin_msa_ld_h((v8u16 *) (pLocalRowCurr), 0);
			temp1 = __builtin_msa_ld_h((v8u16 *) (pLocalRowCurr + 8), 0);

			// Horizontal filtering - c[1]
			 pixels_plus1L = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc);
			__builtin_msa_st_h(pixels_plus1L, (void *) pLocalRowPlus1, 0);
			 pixels_plus1H = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc + 16);
			__builtin_msa_st_h(pixels_plus1H, (void *) (pLocalRowPlus1 + 8), 0);

			// c[0] + c[1]
			 pixels_plus1H = __builtin_msa_addv_h(pixels_plus1H, temp1);
			 pixels_plus1L = __builtin_msa_addv_h(pixels_plus1L, temp0);

			// Horizontal filtering - c[2]
			 pixels_plus2L = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc_NextRow);
			__builtin_msa_st_h(pixels_plus2L, (void *) pLocalRowPlus2, 0);
			 pixels_plus2H = Horizontal3x3GaussianFilter_SampleFirstPixel_MSA(pLocalSrc_NextRow + 16);
			__builtin_msa_st_h(pixels_plus2H, (void *) (pLocalRowPlus2 + 8), 0);

			// load c[-1]
			temp2 = __builtin_msa_ld_h((v8u16 *) (pLocalRowMinus1), 0);
			temp3 = __builtin_msa_ld_h((v8u16 *) (pLocalRowMinus1 + 8), 0);

			// 2 * c[0]
			temp1 = __builtin_msa_slli_h(temp1, 1);
			temp0 = __builtin_msa_slli_h(temp0, 1);

			// c[-1] + c[0] + c[1]
			pixels_plus1H = __builtin_msa_addv_h(pixels_plus1H, temp3);
			pixels_plus1L = __builtin_msa_addv_h(pixels_plus1L, temp2);

			// load c[-2]
			temp2 = __builtin_msa_ld_h((v8u16 *) (pLocalRowMinus2), 0);
			temp3 = __builtin_msa_ld_h((v8u16 *) (pLocalRowMinus2 + 8), 0);


			// 4*c[-1] + 4*c[0] + 4 * c[1]
			pixels_plus1H = __builtin_msa_slli_h(pixels_plus1H, 2);
			pixels_plus1L = __builtin_msa_slli_h(pixels_plus1L, 2);

			// 4*c[-1] + 6*c[0] + 4 * c[1]
			pixels_plus1H = __builtin_msa_addv_h(pixels_plus1H, temp1);
			pixels_plus1L = __builtin_msa_addv_h(pixels_plus1L, temp0);

			// c[-2] + c[2]
			pixels_plus2H = __builtin_msa_addv_h(pixels_plus2H, temp3);
			pixels_plus2L = __builtin_msa_addv_h(pixels_plus2L, temp2);

			// c[-2] + 4 * c[-1] + 4 * c[0] + 4 * c[1] + c[2]
			pixels_plus1H = __builtin_msa_addv_h(pixels_plus1H, pixels_plus2H);
			pixels_plus1L = __builtin_msa_addv_h(pixels_plus1L, pixels_plus2L);

			// divide by 256 is done by pack odd
			pixels_plus1L = (v8i16) __builtin_msa_pckod_b((v16i8) pixels_plus1H, (v16i8) pixels_plus1L);

			__builtin_msa_st_h(pixels_plus1L, (void *) pLocalDst, 0);
			pLocalSrc += 32;
			pLocalSrc_NextRow += 32;
			pLocalDst += 16;
			pLocalRowMinus2 += 16;
			pLocalRowMinus1 += 16;
			pLocalRowCurr += 16;
			pLocalRowPlus1 += 16;
			pLocalRowPlus2 += 16;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc += 2, pLocalSrc_NextRow += 2)
		{
			short temp_plus1 = Horizontal5x5GaussianFilter_C(pLocalSrc);				// row + 1
			*pLocalRowPlus1++ = temp_plus1;
			short temp_plus2 = Horizontal5x5GaussianFilter_C(pLocalSrc_NextRow);		// row + 2
			*pLocalRowPlus2++ = temp_plus2;

			*pLocalDst++ = (unsigned char) ((*pLocalRowMinus2++ + 4 * (*pLocalRowMinus1++) + 6 * (*pLocalRowCurr++) + 4 * temp_plus1 + temp_plus2) >> 8);
		}

#else
		for (int x = 0; x < dstWidth; x++, pLocalSrc += 2, pLocalSrc_NextRow += 2)
		{
			short temp_plus1 = Horizontal5x5GaussianFilter_C(pLocalSrc);				// row + 1
			*pLocalRowPlus1++ = temp_plus1;
			short temp_plus2 = Horizontal5x5GaussianFilter_C(pLocalSrc_NextRow);		// row + 2
			*pLocalRowPlus2++ = temp_plus2;

			*pLocalDst++ = (unsigned char) ((*pLocalRowMinus2++ + 4 * (*pLocalRowMinus1++) + 6 * (*pLocalRowCurr++) + 4 * temp_plus1 + temp_plus2) >> 8);
		}

#endif

		// Move two rows ahead
		pTemp0 = pRowMinus2;
		pTemp1 = pRowMinus1;
		pRowMinus2 = pRowCurr;
		pRowMinus1 = pRowPlus1;
		pRowCurr = pRowPlus2;
		pRowPlus1 = pTemp1;
		pRowPlus2 = pTemp0;

		pLocalRowMinus2 = pRowMinus2;
		pLocalRowMinus1 = pRowMinus1;
		pLocalRowCurr = pRowCurr;
		pLocalRowPlus1 = pRowPlus1;
		pLocalRowPlus2 = pRowPlus2;

		pSrcImage += (srcImageStrideInBytes + srcImageStrideInBytes);
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

// The kernel does a gaussian blur followed by ORB scaling

/* Gaussian Kernel			1   4   6   4   1			1		1   4   6   4   1
							4  16  24  16   4			4
					1/256	6  24  36  24   6    =		6									>> 8
							4  16  24  16   4			4
							1   4   6   4   1			1
*/
int HafCpu_ScaleGaussianOrb_U8_U8_5x5
(
	vx_uint32     dstWidth,
	vx_uint32     dstHeight,
	vx_uint8    * pDstImage,
	vx_uint32     dstImageStrideInBytes,
	vx_uint8    * pSrcImage,
	vx_uint32     srcImageStrideInBytes,
	vx_uint32     srcWidth,
	vx_uint32     srcHeight,
	vx_uint8    * pLocalData
	)
{
	int xpos, ypos, x;
	// need to recalculate scale_factor because they might differ from
	// global scale_factor for different pyramid levels.
	float xcale = (float) srcWidth / dstWidth;
	float yscale = (float) srcHeight / (dstHeight + 4);

	// to convert to fixed point
	int xinc = (int) (FP_MUL * xcale);
	int yinc = (int) (FP_MUL * yscale);

	unsigned short *Xmap = (unsigned short *) pLocalData;
	unsigned short *r0 = (Xmap + ((dstWidth + 15) & ~15));
	vx_uint8 *r1 = (vx_uint8 *) (r0 + ((srcWidth & 15) & ~15));

#if ENABLE_MSA
	v16i8 z = __builtin_msa_ldi_b(0);
	v8i16 c6 = __builtin_msa_ldi_h(6);
#endif

	// generate xmap for orbit scaling
	// generate xmap;
	xpos = (int) (0.5f * xinc);
	for (x = 0; x < (int) dstWidth; x++, xpos += xinc)
	{
		int xmap;
		xmap = (xpos >> FP_BITS);
		Xmap[x] = (unsigned short) xmap;
	}

	//starting from row 2 of dstimage
	ypos = (int) ((2.5f) * yinc);

	// do gaussian verical filter for ypos
	for (int y = 0; y < (int) dstHeight; y++, ypos += yinc)
	{
		unsigned int x;
		unsigned int *pdst = (unsigned int *) pDstImage;
		const vx_uint8 *pSrc = pSrcImage + (ypos >> FP_BITS) * srcImageStrideInBytes;
		const vx_uint8 *srow0 = pSrc - 2 * srcImageStrideInBytes;
		const vx_uint8 *srow1 = pSrc - srcImageStrideInBytes;
		const vx_uint8 *srow2 = pSrc + srcImageStrideInBytes;
		const vx_uint8 *srow3 = pSrc + 2 * srcImageStrideInBytes;

#if ENABLE_MSA
		// do vertical convolution
		for (x = 0; x < srcWidth; x += 16)
		{
			v16i8 s0 = __builtin_msa_ld_b((v8u16 *) (srow0 + x), 0);
			v16i8 s1 = __builtin_msa_ld_b((v8u16 *) (srow1 + x), 0);
			v16i8 s2 = __builtin_msa_ld_b((v8u16 *) (pSrc + x), 0);
			v16i8 s3 = __builtin_msa_ld_b((v8u16 *) (srow2 + x), 0);
			v16i8 s4 = __builtin_msa_ld_b((v8u16 *) (srow3 + x), 0);

			v16i8 s0_L = __builtin_msa_ilvr_b(z, s0);
			v16i8 s4_L = __builtin_msa_ilvr_b(z, s4);

			s0 = __builtin_msa_ilvl_b(z, s0);
			s4 = __builtin_msa_ilvl_b(z, s4);

			s0_L = (v16i8) __builtin_msa_addv_h((v8i16) s0_L, (v8i16) s4_L);
			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, (v8i16) s4);

			v8i16 s1_L = __builtin_msa_addv_h((v8i16) __builtin_msa_ilvr_b(z, s1), (v8i16) __builtin_msa_ilvr_b(z, s3));
			s1 = (v16i8) __builtin_msa_addv_h((v8i16) __builtin_msa_ilvl_b(z, s1), (v8i16) __builtin_msa_ilvl_b(z, s3));

			s4_L = __builtin_msa_ilvr_b(z, s2);
			s2 = __builtin_msa_ilvl_b(z, s2);

			s0_L = (v16i8) __builtin_msa_addv_h((v8i16) s0_L, (v8i16) __builtin_msa_slli_h(s1_L, 2));
			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, (v8i16) __builtin_msa_slli_h((v8i16) s1, 2));

			// low 8 filtered
			s0_L = (v16i8) __builtin_msa_addv_h((v8i16) s0_L, (v8i16) __builtin_msa_mulv_h((v8i16) s4_L, c6));
			// Hi 8 filtered.
			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, (v8i16) __builtin_msa_mulv_h((v8i16) s2, c6));

			// copy to temp
			__builtin_msa_st_h((v8i16) s0_L, (void *) (r0 + x), 0);
			__builtin_msa_st_h((v8i16) s0, (void *) (r0 + x + 8), 0);
		}

		// do horizontal convolution and copy to r1
		for (x = 0; x <srcWidth; x += 8)
		{
			v16i8 s0 = __builtin_msa_ld_b((v8u16 *) (r0 + x - 2), 0);
			v16i8 s1 = __builtin_msa_ld_b((v8u16 *) (r0 + x - 1), 0);
			v16i8 s2 = __builtin_msa_ld_b((v8u16 *) (r0 + x), 0);
			v16i8 s3 = __builtin_msa_ld_b((v8u16 *) (r0 + x + 1), 0);
			v16i8 s4 = __builtin_msa_ld_b((v8u16 *) (r0 + x + 2), 0);

			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, (v8i16) s4);
			s1 = (v16i8) __builtin_msa_addv_h((v8i16) s1, (v8i16) s3);
			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, __builtin_msa_slli_h((v8i16) s1, 2));
			s0 = (v16i8) __builtin_msa_addv_h((v8i16) s0, __builtin_msa_mulv_h((v8i16) s2, c6));

			s0 = (v16i8) __builtin_msa_srli_h((v8i16) s0, 8);

			v8u16 temp0_u = (v8u16)__builtin_msa_sat_u_b((v16u8) s0, 7);
			s0 = (v16i8) __builtin_msa_pckev_b((v16i8) temp0_u, (v16i8) temp0_u);

			*(long long*) (r1 + x) = ((v2i64) s0)[0];
		}
		// do NN scaling and copy to dst
		for (x = 0; x <= dstWidth - 4; x += 4)
		{
			const unsigned short *xm = &Xmap[x];
			*pdst++ = r1[xm[0]] | (r1[xm[1]] << 8) |
				(r1[xm[2]] << 16) | (r1[xm[3]] << 24);
		}
		for (; x < dstWidth; x++)
			pDstImage[x] = r1[Xmap[x]];
#else // C
		// do vertical convolution
		for (x = 0; x < srcWidth; x += 1)
		{
			r0[x] = srow0[x] + srow3[x] + ((srow1[x] + srow2[x]) << 2) + 6 * pSrc[x];
		}

		// x = 0
		r1[0] = (r0[2] + (r0[1] << 2) + 6 * r0[0]) >> 8;
		// x = 1
		r1[1] = (r0[3] + ((r0[0] + r0[2]) << 2) + 6 * r0[1]) >> 8;

		// do horizontal convolution and copy to r1
		for (x = 2; x < srcWidth; x += 1)
		{
			r1[x] = (r0[x - 2] + r0[x + 2] + ((r0[x - 1] + r0[x + 1]) << 2) + 6 * r0[x]) >> 8;
		}

		// do NN scaling and copy to dst
		for (x = 0; x <= dstWidth - 4; x += 4)
		{
			const unsigned short *xm = &Xmap[x];
			*pdst++ = r1[xm[0]] | (r1[xm[1]] << 8) |
				(r1[xm[2]] << 16) | (r1[xm[3]] << 24);
		}
		for (; x < dstWidth; x++)
			pDstImage[x] = r1[Xmap[x]];
#endif
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}
