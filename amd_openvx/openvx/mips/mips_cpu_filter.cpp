#include "ago_internal.h"
#include "mips_internal.h"

/* The function assumes at least one pixel padding on the top, left, right and bottom
Separable filter
	1    1 1 1
	1
	1
*/

int HafCpu_Box_U8_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pScratch
	)
{
	unsigned char *pLocalSrc = (unsigned char *) pSrcImage;
	unsigned char *pLocalDst = (unsigned char *) pDstImage;

	int tmpWidth = (dstWidth + 15) & ~15;
	vx_uint16 *pPrevRow = (vx_uint16 *) pScratch;
	vx_uint16 *pCurrRow = ((vx_uint16 *) pScratch) + tmpWidth;
	vx_uint16 *pNextRow = ((vx_uint16 *) pScratch) + (tmpWidth + tmpWidth);

#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v16i8 row0, shiftedR, shiftedL, temp0_16i8, temp1_16i8;
	v16i8 zeromask = (v16i8) __builtin_msa_ldi_b(0);
	v8i16 resultL, resultH, temp0, temp1;
	v4i32 divFactor = {7282, 7282, 7282, 7282}; // ceil((2^16)/9) = 7282
	v4i32 temp_l, temp_r;
	v8u16 temp1_u, temp0_u;
	v16i8 res;
#endif

	vx_uint16 *pLocalPrevRow = pPrevRow;
	vx_uint16 *pLocalCurrRow = pCurrRow;
	vx_uint16 *pLocalNextRow = pNextRow;
	vx_uint16 *pTemp;

#if ENABLE_MSA
	// Process first two rows - Horizontal filtering
	for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[- (int) srcImageStrideInBytes - 1]
			+ (vx_uint16) pLocalSrc[- (int) srcImageStrideInBytes]
			+ (vx_uint16) pLocalSrc[- (int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[- 1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}

	for (int x = 0; x < (alignedWidth >> 4); x++)
	{
		row0 = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes), 0);
		shiftedL = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes - 1), 0);
		shiftedR = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes + 1), 0);

		resultL = (v8i16) __builtin_msa_ilvr_b(zeromask, shiftedL);
		resultH = (v8i16) __builtin_msa_ilvl_b(zeromask, shiftedL);

		shiftedL = __builtin_msa_ilvl_b(zeromask, row0);
		row0 = __builtin_msa_ilvr_b(zeromask, row0);
		resultH = __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
		resultL = __builtin_msa_addv_h(resultL, (v8i16) row0);

		shiftedL = __builtin_msa_ilvl_b(zeromask, shiftedR);
		shiftedR = __builtin_msa_ilvr_b(zeromask, shiftedR);
		resultH = __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
		resultL = __builtin_msa_addv_h(resultL, (v8i16) shiftedR);

		__builtin_msa_st_h(resultL, (void *) pLocalPrevRow, 0);
		__builtin_msa_st_h(resultH, (void *) (pLocalPrevRow + 8), 0);

		// current row
		row0 = (v16i8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
		shiftedL = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
		shiftedR = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
		resultL = (v8i16) __builtin_msa_ilvr_b(zeromask, shiftedL);
		resultH = (v8i16) __builtin_msa_ilvl_b(zeromask, shiftedL);

		shiftedL = __builtin_msa_ilvl_b(zeromask, row0);
		row0 = __builtin_msa_ilvr_b(zeromask, row0);
		resultH = __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
		resultL = __builtin_msa_addv_h(resultL, (v8i16) row0);

		shiftedL = __builtin_msa_ilvl_b(zeromask, shiftedR);
		shiftedR = __builtin_msa_ilvr_b(zeromask, shiftedR);
		resultH = (v8i16) __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
		resultL = (v8i16) __builtin_msa_addv_h(resultL, (v8i16) shiftedR);

		__builtin_msa_st_h(resultL, (void *) pLocalCurrRow, 0);
		__builtin_msa_st_h(resultH, (void *) (pLocalCurrRow + 8), 0);

		pLocalSrc += 16;
		pLocalPrevRow += 16;
		pLocalCurrRow += 16;
	}

	for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[- (int) srcImageStrideInBytes - 1]
			+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes]
			+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[-1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}
#else // C variant
	for (int x = 0; x < dstWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[- (int) srcImageStrideInBytes - 1]
				+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes]
				+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[-1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}
#endif
	pLocalPrevRow = pPrevRow;
	pLocalCurrRow = pCurrRow;
	pLocalNextRow = pNextRow;

	// Process rows 3 till the end
	int height = (int) dstHeight;
	while (height)
	{
		pLocalSrc = (unsigned char *) (pSrcImage + srcImageStrideInBytes);	// Pointing to the row below
		pLocalDst = (unsigned char *) pDstImage;

#if ENABLE_MSA
	for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
	{
		vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
		*pLocalNextRow++ = temp;	// Save the next row temp pixels
		*pLocalDst++ = (char) ((float) (temp + *pLocalPrevRow++ + *pLocalCurrRow++) / 9.0f);
	}

		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			// Horizontal Filtering
			// current row
			row0 = (v16i8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			shiftedL = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			shiftedR = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);

			resultL = (v8i16) __builtin_msa_ilvr_b(zeromask, shiftedL);
			resultH = (v8i16) __builtin_msa_ilvl_b(zeromask, shiftedL);

			shiftedL = __builtin_msa_ilvl_b(zeromask, row0);
			row0 = __builtin_msa_ilvr_b(zeromask, row0);
			resultH = __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
			resultL = __builtin_msa_addv_h(resultL, (v8i16) row0);

			temp0_16i8 = (v16i8) __builtin_msa_ld_b((void *) pLocalPrevRow, 0);
			temp1_16i8 = (v16i8) __builtin_msa_ld_b((void *) (pLocalPrevRow + 8), 0);

			shiftedL = __builtin_msa_ilvl_b(zeromask, shiftedR);
			shiftedR = __builtin_msa_ilvr_b(zeromask, shiftedR);
			resultH = (v8i16) __builtin_msa_addv_h(resultH, (v8i16) shiftedL);
			resultL = (v8i16) __builtin_msa_addv_h(resultL, (v8i16) shiftedR);

			shiftedL = (v16i8) __builtin_msa_ld_b((void *) pLocalCurrRow, 0);
			shiftedR = (v16i8) __builtin_msa_ld_b((void *) (pLocalCurrRow + 8), 0);

			temp1 = (v8i16) __builtin_msa_addv_h((v8i16) temp1_16i8, resultH);
			temp0 = (v8i16) __builtin_msa_addv_h((v8i16) temp0_16i8, resultL);

			__builtin_msa_st_h(resultL, (void *) pLocalNextRow, 0);
			__builtin_msa_st_h(resultH, (void *) (pLocalNextRow + 8), 0);

			temp1 = (v8i16) __builtin_msa_addv_h(temp1, (v8i16) shiftedR);
			temp0 = (v8i16) __builtin_msa_addv_h(temp0, (v8i16) shiftedL);

			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeromask, temp1);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeromask, temp1);

			temp_l = (v4i32) __builtin_msa_mulv_w(temp_l, divFactor);
			temp_r = (v4i32) __builtin_msa_mulv_w(temp_r, divFactor);

			temp1 = (v8i16) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) zeromask, temp0);
			temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) zeromask, temp0);

			temp_l = (v4i32) __builtin_msa_mulv_w(temp_l, divFactor);
			temp_r = (v4i32) __builtin_msa_mulv_w(temp_r, divFactor);

			temp0 = (v8i16) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

			temp0_u = (v8u16) __builtin_msa_sat_u_h((v8u16) temp0, 15);
			temp1_u = (v8u16) __builtin_msa_sat_u_h((v8u16) temp1, 15);

			res = (v16i8) __builtin_msa_pckev_b((v16i8) temp1_u, (v16i8) temp0_u);
			__builtin_msa_st_b(res, (void *) pLocalDst, 0);

			pLocalSrc += 16;
			pLocalDst += 16;
			pLocalPrevRow += 16;
			pLocalCurrRow += 16;
			pLocalNextRow += 16;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
			// Save the next row temp pixels
			*pLocalNextRow++ = temp;
			*pLocalDst++ = (char) ((float) (temp + *pLocalPrevRow++ + *pLocalCurrRow++) / 9.0f);
		}
#else // C variant
		for (int x = 0; x < dstWidth; x++, pLocalSrc++)
	{
		vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
		// Save the next row temp pixels
		*pLocalNextRow++ = temp;
		*pLocalDst++ = (char) ((float) (temp + *pLocalPrevRow++ + *pLocalCurrRow++) / 9.0f);
	}
#endif
		pTemp = pPrevRow;
		pPrevRow = pCurrRow;
		pCurrRow = pNextRow;
		pNextRow = pTemp;

		pLocalPrevRow = pPrevRow;
		pLocalCurrRow = pCurrRow;
		pLocalNextRow = pNextRow;

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_S16_U8_3xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc;
	short *pLocalDst;

#if ENABLE_MSA
	short *pLocalConvMat;
	v16i8 *pLocalDst_msa;
	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 sat_temp1, sat_temp2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 2 bytes = 1 pixel
	prefixWidth >>= 1;

	// 16 pixels processed at a time in SSE loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 3 * (int) convolutionHeight;

while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (short *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}

			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}

		pLocalDst_msa = (v16i8 *) pLocalDst;
		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				int offset = y * srcStride;

				// shifted left pixels
				row = __builtin_msa_ld_b((void *) (pLocalSrc + offset - 1), 0);
				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - shiftedL pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - shiftedL pixels
				v8i16 signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - shiftedL pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// pixels at the location
				row = __builtin_msa_ld_b((void *) (pLocalSrc + offset), 0);

				// Lowest 4 bytes - shiftedL pixels
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);

				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - at loc pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - at loc pixels
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - at loc pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// shifted right pixels
				row = __builtin_msa_ld_b((void *) (pLocalSrc + offset + 1), 0);

				// Lowest 4 bytes - at loc pixels
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);

				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - shiftedR pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - shiftedR pixels
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - shiftedR pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// Lowest 4 bytes - shiftedR pixels
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
			}

			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			__builtin_msa_st_b(temp0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}

		pLocalDst = (short *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}

			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_U8_U8_3xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16u8 *pLocalDst_msa;
	v8i16 signmask;
	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 temp_w1, temp_w2;
	short *pLocalConvMat;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 16 pixels processed at a time in MSE loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 3 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
		pLocalDst_msa = (v16u8 *) pLocalDst;

		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;

			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				int offset = y * srcStride;

				// shifted left pixels
				row = __builtin_msa_ld_b((void *) (pLocalSrc + offset - 1), 0);
				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - shiftedL pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - shiftedL pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - shiftedL pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// pixels at the location
				row = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc + offset), 0);

				// Lowest 4 bytes - shiftedL pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);

				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - at loc pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - at loc pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - at loc pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// shifted right pixels
				row = (v16i8) __builtin_msa_ld_b((void *) (pLocalSrc + offset + 1), 0);

				// Lowest 4 bytes - at loc pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);

				mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

				// Upper 4 bytes - shiftedR pixels
				temp1 = __builtin_msa_ilvl_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

				// Next 4 bytes - shiftedR pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

				// Next 4 bytes - shiftedR pixels
				temp1 = __builtin_msa_ilvr_b(zeromask, row);
				temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

				// Lowest 4 bytes - shiftedR pixels
				signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
				temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
				temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
				result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
			}
			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = (v4i32)__builtin_msa_sat_u_h((v8u16) temp0, 7);
			temp_w2 = (v4i32)__builtin_msa_sat_u_h((v8u16) row, 7);
			row = __builtin_msa_pckev_b((v16i8) temp_w2, (v16i8) temp_w1);

			__builtin_msa_st_b((v16i8) row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}
		pLocalDst = (unsigned char *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}
int HafCpu_Convolve_S16_U8_5xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc;
	short * pLocalDst;

#if ENABLE_MSA
	short *pLocalConvMat;
	v16i8 *pLocalDst_msa;
	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 sat_temp1, sat_temp2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	prefixWidth >>= 1;
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif

	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 5 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (short *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}

		pLocalDst_msa = (v16i8 *) pLocalDst;
		int width = (int) (alignedWidth >> 4);

		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -2; x <= 2; x++)
				{
					row = __builtin_msa_ld_b((void*) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = __builtin_msa_ilvl_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					v8i16 signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = __builtin_msa_ilvr_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
				}
			}

			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			__builtin_msa_st_b(temp0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}

		pLocalDst = (short *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#endif

		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_U8_U8_5xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16u8 *pLocalDst_msa;
	short *pLocalConvMat;

	v16u8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v8i16 signmask;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 temp_w1, temp_w2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 5 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}

		pLocalDst_msa = (v16u8 *) pLocalDst;
		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;

			result0 = (v16u8) __builtin_msa_ldi_b(0);
			result1 = (v16u8) __builtin_msa_ldi_b(0);
			result2 = (v16u8) __builtin_msa_ldi_b(0);
			result3 = (v16u8) __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -2; x <= 2; x++)
				{
					row = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16u8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) row);
					temp0 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16u8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16u8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16u8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16u8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16u8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) row);
					temp0 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16u8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16u8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16u8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16u8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16u8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);

				}
			}

			result0 = (v16u8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16u8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16u8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16u8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16u8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16u8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = (v4i32)__builtin_msa_sat_u_h((v8u16) temp0, 7);
			temp_w2 = (v4i32)__builtin_msa_sat_u_h((v8u16) row, 7);
			row = (v16u8) __builtin_msa_pckev_b((v16i8) temp_w2, (v16i8) temp_w1);

			__builtin_msa_st_b((v16i8) row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}

		pLocalDst = (unsigned char *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_S16_U8_7xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc;
	short * pLocalDst;

#if ENABLE_MSA
	short *pLocalConvMat;
	v16i8 *pLocalDst_msa;
	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 sat_temp1, sat_temp2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	// 2 bytes = 1 pixel
	prefixWidth >>= 1;
	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif

	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 7 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (short *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}

		pLocalDst_msa = (v16i8 *) pLocalDst;
		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -3; x <= 3; x++)
				{
					row = __builtin_msa_ld_b((void *) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = __builtin_msa_ilvl_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					v8i16 signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = __builtin_msa_ilvr_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
				}
			}

			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			__builtin_msa_st_b(temp0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}

		pLocalDst = (short *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#else

		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i*srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}

#endif

		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_U8_U8_7xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16u8 *pLocalDst_msa;
	short *pLocalConvMat;

	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v8i16 signmask;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 temp_w1, temp_w2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 7 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;

#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
		pLocalDst_msa = (v16u8 *) pLocalDst;

		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -3; x <= 3; x++)
				{
					row = __builtin_msa_ld_b((void *) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = (v16i8) __builtin_msa_ilvl_b(zeromask, (v16i8) row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = __builtin_msa_ilvr_b(zeromask, (v16i8) row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
				}
			}
			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = (v4i32)__builtin_msa_sat_u_h((v8u16) temp0, 7);
			temp_w2 = (v4i32)__builtin_msa_sat_u_h((v8u16) row, 7);
			row = (v16i8) __builtin_msa_pckev_b((v16i8) temp_w2, (v16i8) temp_w1);

			__builtin_msa_st_b((v16i8) row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}
		pLocalDst = (unsigned char *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Convolve_U8_U8_9xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc, *pLocalDst;
#if ENABLE_MSA
	v16u8 *pLocalDst_msa;
	short *pLocalConvMat;

	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v8i16 signmask;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 temp_w1, temp_w2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 9 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
		pLocalDst_msa = (v16u8 *) pLocalDst;

		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -4; x <= 4; x++)
				{
					row = __builtin_msa_ld_b((void *) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = (v16i8) __builtin_msa_ilvl_b(zeromask, (v16i8) row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = __builtin_msa_ilvr_b(zeromask, (v16i8) row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
				}
			}
			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			temp_w2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) temp_w2, (v8i16) temp_w1);

			temp_w1 = (v4i32)__builtin_msa_sat_u_h((v8u16) temp0, 7);
			temp_w2 = (v4i32)__builtin_msa_sat_u_h((v8u16) row, 7);
			row = (v16i8) __builtin_msa_pckev_b((v16i8) temp_w2, (v16i8) temp_w1);

			__builtin_msa_st_b((v16i8) row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}
		pLocalDst = (unsigned char *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, 255);
			temp = max(temp, 0);
			*pLocalDst++ = (unsigned char) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;

		height--;
	}
	return AGO_SUCCESS;
}
int HafCpu_Convolve_S16_U8_9xN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_size		  convolutionHeight,
		vx_int32      shift
	)
{
	unsigned char *pLocalSrc;
	short * pLocalDst;

#if ENABLE_MSA
	short *pLocalConvMat;
	v16i8 *pLocalDst_msa;
	v16i8 result0, result1, result2, result3, row, mul, temp0, temp1;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v4i32 shift_v = __builtin_msa_fill_w(shift);
	v4i32 sat_temp1, sat_temp2;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	// 2 bytes = 1 pixel
	prefixWidth >>= 1;
	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif

	int height = (int) dstHeight;
	int srcStride = (int) srcImageStrideInBytes;
	int rowLimit = (int) (convolutionHeight >> 1);
	int numConvCoeffs = 9 * (int) convolutionHeight;

	while (height)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (short *) pDstImage;
#if ENABLE_MSA
		for (int w = 0; w < prefixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}

		pLocalDst_msa = (v16i8 *) pLocalDst;
		// Each loop processess 16 pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pLocalConvMat = convMatrix + numConvCoeffs - 1;
			result0 = __builtin_msa_ldi_b(0);
			result1 = __builtin_msa_ldi_b(0);
			result2 = __builtin_msa_ldi_b(0);
			result3 = __builtin_msa_ldi_b(0);

			for (int y = -rowLimit; y <= rowLimit; y++)
			{
				for (int x = -4; x <= 4; x++)
				{
					row = __builtin_msa_ld_b((void *) (pLocalSrc + (y * srcStride) + x), 0);
					mul = (v16i8) __builtin_msa_fill_w((int) (*pLocalConvMat--));

					// Upper 4 bytes
					temp1 = __builtin_msa_ilvl_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result3 = (v16i8) __builtin_msa_addv_w((v4i32) result3, (v4i32) temp0);

					// Next 4 bytes
					v8i16 signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result2 = (v16i8) __builtin_msa_addv_w((v4i32) result2, (v4i32) temp0);

					// Next 4 bytes
					temp1 = __builtin_msa_ilvr_b(zeromask, row);
					temp0 = (v16i8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result1 = (v16i8) __builtin_msa_addv_w((v4i32) result1, (v4i32) temp0);

					// Lowest 4 bytes
					signmask = __builtin_msa_clti_s_h((v8i16) temp1, 0);
					temp0 = (v16i8) __builtin_msa_ilvr_h(signmask, (v8i16) temp1);
					temp0 = (v16i8) __builtin_msa_mulv_w((v4i32) temp0, (v4i32) mul);
					result0 = (v16i8) __builtin_msa_addv_w((v4i32) result0, (v4i32) temp0);
				}
			}

			result0 = (v16i8) __builtin_msa_srl_w((v4i32) result0, shift_v);
			result1 = (v16i8) __builtin_msa_srl_w((v4i32) result1, shift_v);
			result2 = (v16i8) __builtin_msa_srl_w((v4i32) result2, shift_v);
			result3 = (v16i8) __builtin_msa_srl_w((v4i32) result3, shift_v);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result2, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result3, 15);
			row = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			sat_temp1 = __builtin_msa_sat_s_w((v4i32) result0, 15);
			sat_temp2 = __builtin_msa_sat_s_w((v4i32) result1, 15);
			temp0 = (v16i8) __builtin_msa_pckev_h((v8i16) sat_temp2, (v8i16) sat_temp1);

			__builtin_msa_st_b(temp0, (void *) pLocalDst_msa++, 0);
			__builtin_msa_st_b(row, (void *) pLocalDst_msa++, 0);

			pLocalSrc += 16;
			width--;
		}

		pLocalDst = (short *) pLocalDst_msa;
		for (int w = 0; w < postfixWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#else
		for (int w = 0; w < dstWidth; w++, pLocalSrc++)
		{
			int temp = 0;
			int idx = numConvCoeffs - 1;
			for (int i = -rowLimit; i <= rowLimit; i++)
			{
				for (int j = -4; j <= 4; j++)
				{
					temp += ((int) pLocalSrc[i * srcStride + j] * (int) convMatrix[idx--]);
				}
			}
			temp >>= shift;
			temp = min(temp, SHRT_MAX);
			temp = max(temp, SHRT_MIN);
			*pLocalDst++ = (short) temp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes >> 1);

		height--;
	}
	return AGO_SUCCESS;
}

int HafCpu_Dilate_U8_U8_3x3
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
#if ENABLE_MSA
	v16u8 row0, row1, row2, shiftedR, shiftedL;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;

#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = max(max(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
						pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = max(max(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = max(temp1, temp2);
			temp2 = max(max(pLocalSrc[(int) srcImageStrideInBytes - 1],
							pLocalSrc[(int) srcImageStrideInBytes]),
						pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = max(temp1, temp2);
		}

		for (int width = 0; width < (int) (alignedWidth >> 4); width++, pLocalSrc += 16, pLocalDst += 16)
		{
			// For the row above
			row0 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes), 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes + 1), 0);
			row0 = __builtin_msa_max_u_b(row0, shiftedL);
			row0 = __builtin_msa_max_u_b(row0, shiftedR);

			// For the current row
			row1 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
			row1 = __builtin_msa_max_u_b(row1, shiftedL);
			row1 = __builtin_msa_max_u_b(row1, shiftedR);

			// For the row below
			row2 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes), 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes + 1), 0);
			row2 = __builtin_msa_max_u_b(row2, shiftedL);
			row2 = __builtin_msa_max_u_b(row2, shiftedR);

			row0 = __builtin_msa_max_u_b(row0, row1);
			row0 = __builtin_msa_max_u_b(row0, row2);
			__builtin_msa_st_b((v16i8) row0, (void *) pLocalDst, 0);
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = max(max(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
						pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = max(max(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = max(temp1, temp2);
			temp2 = max(max(pLocalSrc[(int) srcImageStrideInBytes - 1],
							pLocalSrc[(int) srcImageStrideInBytes]),
						pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = max(temp1, temp2);
		}
#else
		for (int x = 0; x < dstWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = max(max(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
						pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = max(max(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = max(temp1, temp2);
			temp2 = max(max(pLocalSrc[(int) srcImageStrideInBytes - 1],
						pLocalSrc[(int) srcImageStrideInBytes]),
					pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = max(temp1, temp2);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_Erode_U8_U8_3x3
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
#if ENABLE_MSA
	v16u8 row0, row1, row2, shiftedR, shiftedL;

	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = min(min(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
							pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = min(min(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = min(temp1, temp2);
			temp2 = min(min(pLocalSrc[(int) srcImageStrideInBytes - 1],
							pLocalSrc[(int) srcImageStrideInBytes]),
							pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = min(temp1, temp2);
		}
		for (int width = 0; width < (int) (alignedWidth >> 4); width++, pLocalSrc += 16, pLocalDst += 16)
		{
			// For the row above
			row0 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes), 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes + 1), 0);
			row0 = __builtin_msa_min_u_b(row0, shiftedL);
			row0 = __builtin_msa_min_u_b(row0, shiftedR);

			// For the current row
			row1 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
			row1 = __builtin_msa_min_u_b(row1, shiftedL);
			row1 = __builtin_msa_min_u_b(row1, shiftedR);

			// For the row below
			row2 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes), 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + srcImageStrideInBytes + 1), 0);
			row2 = __builtin_msa_min_u_b(row2, shiftedL);
			row2 = __builtin_msa_min_u_b(row2, shiftedR);

			row0 = __builtin_msa_min_u_b(row0, row1);
			row0 = __builtin_msa_min_u_b(row0, row2);

			__builtin_msa_st_b((v16i8) row0, (void *) pLocalDst, 0);
		}
		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = min(min(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
							pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = min(min(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = min(temp1, temp2);
			temp2 = min(min(pLocalSrc[(int) srcImageStrideInBytes - 1],
							pLocalSrc[(int) srcImageStrideInBytes]),
							pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = min(temp1, temp2);
		}
#else
		for (int x = 0; x < dstWidth; x++, pLocalSrc++)
		{
			unsigned char temp1, temp2;
			temp1 = min(min(pLocalSrc[-(int) srcImageStrideInBytes - 1],
							pLocalSrc[-(int) srcImageStrideInBytes]),
							pLocalSrc[-(int) srcImageStrideInBytes + 1]);
			temp2 = min(min(pLocalSrc[-1], pLocalSrc[0]), pLocalSrc[1]);
			temp1 = min(temp1, temp2);
			temp2 = min(min(pLocalSrc[(int) srcImageStrideInBytes - 1],
							pLocalSrc[(int) srcImageStrideInBytes]),
							pLocalSrc[(int) srcImageStrideInBytes + 1]);
			*pLocalDst++ = min(temp1, temp2);
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

/* The function assumes at least one pixel padding on the top, left, right and bottom
   Separable filter
	1    1 2 1
	2
	1
*/
int HafCpu_Gaussian_U8_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8	* pScratch
	)
{
	unsigned char *pLocalSrc = (unsigned char *) pSrcImage;
	unsigned char *pLocalDst = (unsigned char *) pDstImage;

	int tmpWidth = (dstWidth + 15) & ~15;
	vx_uint16 * pPrevRow = (vx_uint16 *) pScratch;
	vx_uint16 * pCurrRow = ((vx_uint16 *) pScratch) + tmpWidth;
	vx_uint16 * pNextRow = ((vx_uint16 *) pScratch) + (tmpWidth + tmpWidth);

	vx_uint16 * pLocalPrevRow = pPrevRow;
	vx_uint16 * pLocalCurrRow = pCurrRow;
	vx_uint16 * pLocalNextRow = pNextRow;
	vx_uint16 * pTemp;
#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v16u8 row0, shiftedR, shiftedL, temp0, temp1, resultH, resultL;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
#endif

#if ENABLE_MSA
	// Process first two rows - Horizontal filtering
	for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
							+ 2 * (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes]
							+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}

	for (int x = 0; x < (alignedWidth >> 4); x++)
	{
		// row above
		row0 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes), 0);
		shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes - 1), 0);
		shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes + 1), 0);
		// L: 1 * (-1,-1)
		resultL = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
		// H: 1 * (-1,-1)
		resultH = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);
		// H: 2 * (0,-1)
		shiftedL = (v16u8) __builtin_msa_slli_h((v8i16) shiftedL, 1);
		row0 = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
		// L: 2 * (0,-1)
		row0 = (v16u8) __builtin_msa_slli_h((v8i16) row0, 1);
		resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
		resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) row0);
		// H: 1 * (1,-1)
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
		// L: 1 * (1,-1)
		shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
		resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
		resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) shiftedR);

		__builtin_msa_st_b((v16i8) resultL, (void *) pLocalPrevRow, 0);
		__builtin_msa_st_b((v16i8) resultH, (void *) (pLocalPrevRow + 8), 0);

		// current row
		row0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
		shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
		shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
		// L: 1 * (-1,0)
		resultL = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
		// H: 1 * (-1,0)
		resultH = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);
		// H: 2 * (0,0)
		shiftedL = (v16u8) __builtin_msa_slli_h((v8i16) shiftedL, 1);
		row0 = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
		// L: 2 * (0,0)
		row0 = (v16u8) __builtin_msa_slli_h((v8i16) row0, 1);
		resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
		resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) row0);
		// H: 1 * (1,0)
		// L: 1 * (1,0)
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
		shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
		resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
		resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) shiftedR);

		__builtin_msa_st_b((v16i8) resultL, (void *) pLocalCurrRow, 0);
		__builtin_msa_st_b((v16i8) resultH, (void *) (pLocalCurrRow + 8), 0);

		pLocalSrc += 16;
		pLocalPrevRow += 16;
		pLocalCurrRow += 16;
	}

	for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
							+ 2 * (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes]
							+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}
#else
	for (int x = 0; x < dstWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
							+ 2 * (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes]
							+ (vx_uint16) pLocalSrc[-(int) srcImageStrideInBytes + 1];
		*pLocalCurrRow++ = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
	}
#endif
	pLocalPrevRow = pPrevRow;
	pLocalCurrRow = pCurrRow;
	pLocalNextRow = pNextRow;

	// Process rows 3 till the end
	int height = (int) dstHeight;
	while (height)
	{
		// Pointing to the row below
		pLocalSrc = (unsigned char *) (pSrcImage + srcImageStrideInBytes);
		pLocalDst = (unsigned char *) pDstImage;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
			// Save the next row temp pixels
			*pLocalNextRow++ = temp;
			*pLocalDst++ = (char) ((temp + *pLocalPrevRow++ + 2 * (*pLocalCurrRow++)) >> 4);
		}

		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			// Horizontal Filtering

			// current row
			row0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
			// L: 1 * (-1,-1)
			resultL = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
			// H: 1 * (-1,-1)
			resultH = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
			shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);
			// H: 2 * (0,0)
			shiftedL = (v16u8) __builtin_msa_slli_h((v8i16) shiftedL, 1);
			row0 = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
			// L: 2 * (0,0)
			row0 = (v16u8) __builtin_msa_slli_h((v8i16) row0, 1);
			resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
			resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) row0);

			// Prev Row
			temp0 = (v16u8) __builtin_msa_ld_b((void *) pLocalPrevRow, 0);
			temp1 = (v16u8) __builtin_msa_ld_b((void *) (pLocalPrevRow + 8), 0);
			// H: 1 * (1,-1)
			shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
			// L: 1 * (1,-1)
			shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
			resultH = (v16u8) __builtin_msa_addv_h((v8i16) resultH, (v8i16) shiftedL);
			resultL = (v16u8) __builtin_msa_addv_h((v8i16) resultL, (v8i16) shiftedR);

			// Current Row
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) pLocalCurrRow, 0);
			shiftedL = (v16u8) __builtin_msa_slli_h((v8i16) shiftedL, 1);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalCurrRow + 8), 0);
			shiftedR = (v16u8) __builtin_msa_slli_h((v8i16) shiftedR, 1);

			// Prev row + next row
			temp1 = (v16u8) __builtin_msa_addv_h((v8i16) temp1, (v8i16) resultH);
			temp0 = (v16u8) __builtin_msa_addv_h((v8i16) temp0, (v8i16) resultL);

			// Save the horizontal filtered pixels from the next row
			__builtin_msa_st_b((v16i8) resultL, (void *) pLocalNextRow, 0);
			__builtin_msa_st_b((v16i8) resultH, (void *) (pLocalNextRow + 8), 0);

			// Prev row + curr row + next row
			temp1 = (v16u8) __builtin_msa_addv_h((v8i16) temp1, (v8i16) shiftedR);
			temp0 = (v16u8) __builtin_msa_addv_h((v8i16) temp0, (v8i16) shiftedL);

			temp1 = (v16u8) __builtin_msa_srli_h((v8i16) temp1, 4);
			temp0 = (v16u8) __builtin_msa_srli_h((v8i16) temp0, 4);

			v8u16 temp0_u = (v8u16) __builtin_msa_sat_u_b((v16u8) temp0, 7);
			v8u16 temp1_u = (v8u16) __builtin_msa_sat_u_b((v16u8) temp1, 7);
			temp0 = (v16u8) __builtin_msa_pckev_b((v16i8) temp1_u, (v16i8) temp0_u);

			__builtin_msa_st_b((v16i8) temp0, (void *) pLocalDst, 0);

			pLocalSrc += 16;
			pLocalDst += 16;
			pLocalPrevRow += 16;
			pLocalCurrRow += 16;
			pLocalNextRow += 16;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
			// Save the next row temp pixels
			*pLocalNextRow++ = temp;
			*pLocalDst++ = (char) ((temp + *pLocalPrevRow++ + 2 * (*pLocalCurrRow++)) >> 4);
		}
#else
		for (int x = 0; x < dstWidth; x++, pLocalSrc++)
		{
			vx_uint16 temp = (vx_uint16) pLocalSrc[-1] + 2 * (vx_uint16) pLocalSrc[0] + (vx_uint16) pLocalSrc[1];
			// Save the next row temp pixels
			*pLocalNextRow++ = temp;
			*pLocalDst++ = (char) ((temp + *pLocalPrevRow++ + 2 * (*pLocalCurrRow++)) >> 4);
		}
#endif
		pTemp = pPrevRow;
		pPrevRow = pCurrRow;
		pCurrRow = pNextRow;
		pNextRow = pTemp;

		pLocalPrevRow = pPrevRow;
		pLocalCurrRow = pCurrRow;
		pLocalNextRow = pNextRow;

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
		height--;
	}
	return AGO_SUCCESS;
}

#if ENABLE_MSA
static inline void CompareAndSwap(v16u8 &p1, v16u8 &p2)
{
	v16u8 First = __builtin_msa_min_u_b(p1, p2);
	v16u8 Sec = __builtin_msa_max_u_b(p1, p2);
	p1 = First;
	p2 = Sec;
}
#endif

int compareTwo(const void *a, const void *b)
{
	return(*(unsigned char *) a > *(unsigned char *) b ? 1 : -1);
}

int HafCpu_Median_U8_U8_3x3
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
	int prefixWidth = intptr_t(pDstImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v16u8 pixels0, pixels1, pixels2, pixels3, pixels4, pixels5, pixels6, pixels7, pixels8;
#endif
	unsigned char *pLocalSrc, *pPrevSrc, *pNextSrc, *pLocalDst;
	unsigned char pixelArr[9];

	for (int height = 0; height < (int) dstHeight; height++)
	{
		pLocalDst = (unsigned char *) pDstImage;
		pLocalSrc = (unsigned char *) pSrcImage;
		pPrevSrc = pLocalSrc - srcImageStrideInBytes;
		pNextSrc = pLocalSrc + srcImageStrideInBytes;
#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalDst++, pLocalSrc++, pPrevSrc++, pNextSrc++)
		{
			pixelArr[0] = pPrevSrc[-1];
			pixelArr[1] = pPrevSrc[0];
			pixelArr[2] = pPrevSrc[1];
			pixelArr[3] = pLocalSrc[-1];
			pixelArr[4] = pLocalSrc[0];
			pixelArr[5] = pLocalSrc[1];
			pixelArr[6] = pNextSrc[-1];
			pixelArr[7] = pNextSrc[0];
			pixelArr[8] = pNextSrc[1];
			qsort(pixelArr, 9, sizeof(unsigned char), compareTwo);
			*pLocalDst = pixelArr[4];
		}

		for (int width = 0; width < (alignedWidth >> 4); width++)
		{
			pixels0 = (v16u8) __builtin_msa_ld_b((void *) (pPrevSrc - 1), 0);
			pixels1 = (v16u8) __builtin_msa_ld_b((void *) pPrevSrc, 0);
			pixels2 = (v16u8) __builtin_msa_ld_b((void *) (pPrevSrc + 1), 0);
			pixels3 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			pixels4 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			pixels5 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);
			pixels6 = (v16u8) __builtin_msa_ld_b((void *) (pNextSrc - 1), 0);
			pixels7 = (v16u8) __builtin_msa_ld_b((void *) pNextSrc, 0);
			pixels8 = (v16u8) __builtin_msa_ld_b((void *) (pNextSrc + 1), 0);

			// sort by compare and swap : no branching required
			CompareAndSwap(pixels1, pixels2);
			CompareAndSwap(pixels4, pixels5);
			CompareAndSwap(pixels7, pixels8);
			CompareAndSwap(pixels0, pixels1);
			CompareAndSwap(pixels3, pixels4);
			CompareAndSwap(pixels6, pixels7);
			CompareAndSwap(pixels1, pixels2);
			CompareAndSwap(pixels4, pixels5);
			CompareAndSwap(pixels7, pixels8);
			CompareAndSwap(pixels0, pixels3);
			CompareAndSwap(pixels5, pixels8);
			CompareAndSwap(pixels4, pixels7);
			CompareAndSwap(pixels3, pixels6);
			CompareAndSwap(pixels1, pixels4);
			CompareAndSwap(pixels2, pixels5);
			CompareAndSwap(pixels4, pixels7);
			CompareAndSwap(pixels4, pixels2);
			CompareAndSwap(pixels6, pixels4);
			CompareAndSwap(pixels4, pixels2);

			// store median value
			__builtin_msa_st_b((v16i8) pixels4, (void *) pLocalDst, 0);

			pPrevSrc += 16;
			pLocalSrc += 16;
			pNextSrc += 16;
			pLocalDst += 16;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalDst++, pLocalSrc++, pPrevSrc++, pNextSrc++)
		{
			pixelArr[0] = pPrevSrc[-1];
			pixelArr[1] = pPrevSrc[0];
			pixelArr[2] = pPrevSrc[1];
			pixelArr[3] = pLocalSrc[-1];
			pixelArr[4] = pLocalSrc[0];
			pixelArr[5] = pLocalSrc[1];
			pixelArr[6] = pNextSrc[-1];
			pixelArr[7] = pNextSrc[0];
			pixelArr[8] = pNextSrc[1];
			qsort(pixelArr, 9, sizeof(unsigned char), compareTwo);
			*pLocalDst = pixelArr[4];
		}
#else
		for (int x = 0; x < dstWidth; x++, pLocalDst++, pLocalSrc++, pPrevSrc++, pNextSrc++)
		{
			pixelArr[0] = pPrevSrc[-1];
			pixelArr[1] = pPrevSrc[0];
			pixelArr[2] = pPrevSrc[1];
			pixelArr[3] = pLocalSrc[-1];
			pixelArr[4] = pLocalSrc[0];
			pixelArr[5] = pLocalSrc[1];
			pixelArr[6] = pNextSrc[-1];
			pixelArr[7] = pNextSrc[0];
			pixelArr[8] = pNextSrc[1];
			qsort(pixelArr, 9, sizeof(unsigned char), compareTwo);
			*pLocalDst = pixelArr[4];
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

/* The function assumes at least one pixel padding on the top, left, right and bottom
	Separable filter
		1	 -1 0 1					-1	  1 2 1
   Gx = 2						Gy = 0
		1							-1
*/
int HafCpu_Sobel_S16S16_U8_3x3_GXY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstGxImage,
		vx_uint32     dstGxImageStrideInBytes,
		vx_int16    * pDstGyImage,
		vx_uint32     dstGyImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8	* pScratch
	)
{
	unsigned char *pLocalSrc = (unsigned char *) pSrcImage;
	short *pLocalDstGx, *pLocalDstGy;

#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstGxImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v16u8 row0, shiftedR, shiftedL, temp0, temp1, temp2;
	v8i16 GxH, GxL, GyH, GyL;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
#endif

	int tmpWidth = (dstWidth + 15) & ~15;
	vx_int16 *pPrevRow = (vx_int16 *) pScratch;
	vx_int16 *pCurrRow = ((vx_int16 *) pScratch) + (2 * tmpWidth);
	vx_int16 *pNextRow = ((vx_int16 *) pScratch) + (4 * tmpWidth);

	vx_int16 *pLocalPrevRow = pPrevRow;
	vx_int16 *pLocalCurrRow = pCurrRow;
	vx_int16 *pLocalNextRow = pNextRow;
	vx_int16 *pTemp;

#if ENABLE_MSA
	// Process first two rows - Horizontal filtering
	for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1]
			- (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1];					// Gx
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
			+ ((vx_int16) pLocalSrc[-(int) srcImageStrideInBytes] << 1)
			+ (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1];					// Gy
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];			// Gx
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1)
			+ (vx_int16) pLocalSrc[1];													 // Gy
	}

	for (int x = 0; x < (int) (alignedWidth >> 4); x++)
	{
		// row above
		row0 = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes), 0);
		shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes - 1), 0);
		shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - srcImageStrideInBytes + 1), 0);

		GyH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);

		// GyH: 2 * (0,-1)
		GyH = __builtin_msa_slli_h(GyH, 1);
		GyL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
		// GyL: 2 * (0,-1)
		GyL = __builtin_msa_slli_h(GyL, 1);

		// GxL: -1 * (-1,-1)	GyL: 1 * (-1,-1)
		GxL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
		// GxH: -1 * (-1,-1)	GyH: 1 * (-1,-1)
		GxH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
		GyH = __builtin_msa_addv_h(GyH, GxH);
		GyL = __builtin_msa_addv_h(GyL, GxL);

		// GxH: 1 * (1,-1)		GyH: 1 * (1,-1)
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
		// GxL: 1 * (1,-1)		GyL: 1 * (1,-1)
		shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
		GxH = __builtin_msa_subv_h((v8i16) shiftedL, GxH);
		GxL = __builtin_msa_subv_h((v8i16) shiftedR, GxL);
		GyH = __builtin_msa_addv_h(GyH, (v8i16) shiftedL);
		GyL = __builtin_msa_addv_h(GyL, (v8i16) shiftedR);

		__builtin_msa_st_h(GxL, (void *) pLocalPrevRow, 0);
		__builtin_msa_st_h(GxH, (void *) (pLocalPrevRow + 8), 0);
		__builtin_msa_st_h(GyL, (void *) (pLocalPrevRow + 16), 0);
		__builtin_msa_st_h(GyH, (void *) (pLocalPrevRow + 24), 0);

		// current row
		row0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
		shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
		shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);

		GyH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);
		// GyH: 2 * (-1, 0)
		GyH = __builtin_msa_slli_h(GyH, 1);
		GyL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
		// GyL: 2 * (-1, 0)
		GyL = __builtin_msa_slli_h(GyL, 1);

		// GxL: -1 * (-1,-1)	GyL: 1 * (-1,-1)
		GxL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
		// GxH: -1 * (-1,-1)	GyH: 1 * (-1,-1)
		GxH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
		GyH = __builtin_msa_addv_h(GyH, GxH);
		GyL = __builtin_msa_addv_h(GyL, GxL);

		// GxH: 1 * (1,-1)		GyH: 1 * (1,-1)
		shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
		// GxL: 1 * (1,-1)		GyL: 1 * (1,-1)
		shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
		GxH = __builtin_msa_subv_h((v8i16) shiftedL, GxH);
		GxL = __builtin_msa_subv_h((v8i16) shiftedR, GxL);
		GyH = __builtin_msa_addv_h(GyH, (v8i16) shiftedL);
		GyL = __builtin_msa_addv_h(GyL, (v8i16) shiftedR);

		__builtin_msa_st_h(GxL, (void *) pLocalCurrRow, 0);
		__builtin_msa_st_h(GxH, (void *) (pLocalCurrRow + 8), 0);
		__builtin_msa_st_h(GyL, (void *) (pLocalCurrRow + 16), 0);
		__builtin_msa_st_h(GyH, (void *) (pLocalCurrRow + 24), 0);

		pLocalSrc += 16;
		pLocalPrevRow += 32;
		pLocalCurrRow += 32;
	}

	for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1]
			- (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1];					// Gx
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
			+ ((vx_int16) pLocalSrc[-(int) srcImageStrideInBytes] << 1)
			+ (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1];					// Gy
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];			// Gx
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1)
			+ (vx_int16) pLocalSrc[1];													// Gy
	}
#else
	for (int x = 0; x < dstWidth; x++, pLocalSrc++)
	{
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1]
			- (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1];					// Gx
		*pLocalPrevRow++ = (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes - 1]
			+ ((vx_int16) pLocalSrc[-(int) srcImageStrideInBytes] << 1)
			+ (vx_int16) pLocalSrc[-(int) srcImageStrideInBytes + 1];					// Gy
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];			// Gx
		*pLocalCurrRow++ = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1)
			+ (vx_int16) pLocalSrc[1];													// Gy
	}
#endif

	pLocalPrevRow = pPrevRow;
	pLocalCurrRow = pCurrRow;
	pLocalNextRow = pNextRow;

	// Process rows 3 till the end
	int height = (int) dstHeight;
	while (height)
	{
		// Pointing to the row below
		pLocalSrc = (unsigned char *) (pSrcImage + srcImageStrideInBytes);
		pLocalDstGx = (short *) pDstGxImage;
		pLocalDstGy = (short *) pDstGyImage;

#if ENABLE_MSA
		for (int x = 0; x < prefixWidth; x++, pLocalSrc++)
		{
			vx_int16 tempGx = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];
			*pLocalNextRow++ = tempGx;
			vx_int16 tempGy = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1) + (vx_int16) pLocalSrc[1];
			*pLocalNextRow++ = tempGy;

			*pLocalDstGx++ = *pLocalPrevRow++ + ((*pLocalCurrRow++) << 1) + tempGx;
			*pLocalDstGy++ = tempGy - *pLocalPrevRow++;
			pLocalCurrRow++;
		}

		int width = (int) (dstWidth >> 4);
		while (width)
		{
			// Horizontal Filtering
			// next row
			row0 = (v16u8) __builtin_msa_ld_b((void *) pLocalSrc, 0);
			shiftedL = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc - 1), 0);
			shiftedR = (v16u8) __builtin_msa_ld_b((void *) (pLocalSrc + 1), 0);

			GyH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) row0);
			// GyH: 2 * (-1, 0)
			GyH = __builtin_msa_slli_h(GyH, 1);
			GyL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) row0);
			// GyL: 2 * (-1, 0)
			GyL = __builtin_msa_slli_h(GyL, 1);

			// GxL: -1 * (-1,-1)	GyL: 1 * (-1,-1)
			GxL = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedL);
			// GxH: -1 * (-1,-1)	GyH: 1 * (-1,-1)
			GxH = (v8i16) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedL);
			GyH = __builtin_msa_addv_h(GyH, GxH);
			GyL = __builtin_msa_addv_h(GyL, GxL);

			// Prev Row - Gx
			temp0 = (v16u8) __builtin_msa_ld_h((void *) pLocalPrevRow, 0);
			temp1 = (v16u8) __builtin_msa_ld_h((void *) (pLocalPrevRow + 8), 0);
			// Prev Row - Gy
			row0 = (v16u8) __builtin_msa_ld_h((void *) (pLocalPrevRow + 16), 0);
			temp2 = (v16u8) __builtin_msa_ld_h((void *) (pLocalPrevRow + 24), 0);

			// GxH: 1 * (1,-1)		GyH: 1 * (1,-1)
			shiftedL = (v16u8) __builtin_msa_ilvl_b(zeromask, (v16i8) shiftedR);
			// GxL: 1 * (1,-1)		GyL: 1 * (1,-1)
			shiftedR = (v16u8) __builtin_msa_ilvr_b(zeromask, (v16i8) shiftedR);
			GxH = __builtin_msa_subv_h((v8i16) shiftedL, GxH);
			GxL = __builtin_msa_subv_h((v8i16) shiftedR, GxL);
			GyH = __builtin_msa_addv_h(GyH, (v8i16) shiftedL);
			GyL = __builtin_msa_addv_h(GyL, (v8i16) shiftedR);

			// Current Row
			shiftedL = (v16u8) __builtin_msa_ld_h((void *) pLocalCurrRow, 0);
			shiftedR = (v16u8) __builtin_msa_ld_h((void *) (pLocalCurrRow + 8), 0);

			// Prev row + next row
			temp1 = (v16u8) __builtin_msa_addv_h((v8i16) temp1, GxH);
			temp0 = (v16u8) __builtin_msa_addv_h((v8i16) temp0, GxL);

			shiftedR = (v16u8) __builtin_msa_slli_h((v8i16) shiftedR, 1);
			shiftedL = (v16u8) __builtin_msa_slli_h((v8i16) shiftedL, 1);

			// Save the horizontal filtered pixels from the next row - Gx
			__builtin_msa_st_h(GxL, (void *) pLocalNextRow, 0);
			__builtin_msa_st_h(GxH, (void *) (pLocalNextRow + 8), 0);
			// Save the horizontal filtered pixels from the next row - Gy
			__builtin_msa_st_h(GyL, (void *) (pLocalNextRow + 16), 0);
			__builtin_msa_st_h(GyH, (void *) (pLocalNextRow + 24), 0);

			// next row - Prev row
			temp1 = (v16u8) __builtin_msa_addv_h((v8i16) temp1, (v8i16) shiftedR);
			temp0 = (v16u8) __builtin_msa_addv_h((v8i16) temp0, (v8i16) shiftedL);
			row0 = (v16u8) __builtin_msa_subv_h(GyL, (v8i16) row0);
			temp2 = (v16u8) __builtin_msa_subv_h(GyH, (v8i16) temp2);

			__builtin_msa_st_h((v8i16) temp0, (void *) pLocalDstGx, 0);
			__builtin_msa_st_h((v8i16) temp1, (void *) (pLocalDstGx + 8), 0);
			__builtin_msa_st_h((v8i16) row0, (void *) pLocalDstGy, 0);
			__builtin_msa_st_h((v8i16) temp2, (void *) (pLocalDstGy + 8), 0);

			pLocalSrc += 16;
			pLocalDstGx += 16;
			pLocalDstGy += 16;
			pLocalPrevRow += 32;
			pLocalCurrRow += 32;
			pLocalNextRow += 32;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++, pLocalSrc++)
		{
			vx_int16 tempGx = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];
			*pLocalNextRow++ = tempGx;
			vx_int16 tempGy = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1) + (vx_int16) pLocalSrc[1];
			*pLocalNextRow++ = tempGy;

			*pLocalDstGx++ = *pLocalPrevRow++ + ((*pLocalCurrRow++) << 1) + tempGx;
			*pLocalDstGy++ = tempGy - *pLocalPrevRow++;
			pLocalCurrRow++;
		}
#else
		for (int x = 0; x < dstWidth; x++, pLocalSrc++)
		{
			vx_int16 tempGx = (vx_int16) pLocalSrc[1] - (vx_int16) pLocalSrc[-1];
			*pLocalNextRow++ = tempGx;
			vx_int16 tempGy = (vx_int16) pLocalSrc[-1] + ((vx_int16) pLocalSrc[0] << 1) + (vx_int16) pLocalSrc[1];
			*pLocalNextRow++ = tempGy;

			*pLocalDstGx++ = *pLocalPrevRow++ + ((*pLocalCurrRow++) << 1) + tempGx;
			*pLocalDstGy++ = tempGy - *pLocalPrevRow++;
			pLocalCurrRow++;
		}
#endif
		pTemp = pPrevRow;
		pPrevRow = pCurrRow;
		pCurrRow = pNextRow;
		pNextRow = pTemp;

		pLocalPrevRow = pPrevRow;
		pLocalCurrRow = pCurrRow;
		pLocalNextRow = pNextRow;

		pSrcImage += srcImageStrideInBytes;
		pDstGxImage += (dstGxImageStrideInBytes >> 1);
		pDstGyImage += (dstGyImageStrideInBytes >> 1);
		height--;
	}
	return AGO_SUCCESS;
}
