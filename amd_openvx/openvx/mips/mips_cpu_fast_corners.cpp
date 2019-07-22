#include "ago_internal.h"
#include "mips_internal.h"

DECL_ALIGN(16) unsigned char dataFastCornersPixelMask[7 * 16] ATTR_ALIGN(16) = {
	  1,   2, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,
	255, 255,   4, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0, 255,
	255, 255, 255,   6, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0, 255, 255,
	255, 255, 255, 255,   6, 255, 255, 255, 255, 255, 255, 255,   0, 255, 255, 255,
	255, 255, 255, 255, 255,   6, 255, 255, 255, 255, 255,   0, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255,   4, 255, 255, 255,   0, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255,   2,   1,   0, 255, 255, 255, 255, 255, 255
};

static inline void generateOffset(int srcStride, int * offsets)
{
	offsets[0] = -3 * srcStride;
	offsets[15] = offsets[0] - 1;
	offsets[1] = offsets[0] + 1;
	offsets[2] = -(srcStride << 1) + 2;
	offsets[14] = offsets[2] - 4;
	offsets[3] = -srcStride + 3;
	offsets[13] = offsets[3] - 6;
	offsets[4] = 3;
	offsets[12] = -3;
	offsets[5] = srcStride + 3;
	offsets[11] = offsets[5] - 6;
	offsets[6] = (srcStride << 1) + 2;
	offsets[10] = offsets[6] - 4;
	offsets[7] = 3 * srcStride + 1;
	offsets[8] = offsets[7] - 1;
	offsets[9] = offsets[8] - 1;

	return;
}

static inline void generateMasks_C(unsigned char * src, int srcStride, int* offsets, short t, int mask[2])
{
	mask[0] = 0;
	mask[1] = 0;
	int iterMask = 1;

	// Early exit conditions

	// Pixels 1 and 9 within t of the candidate
	if ((abs((short) src[0] - (short) src[offsets[0]]) < t) && (abs((short) src[0] - (short) src[offsets[8]]) < t))
		return;
	// Pixels 5 and 13 within t of the candidate
	if ((abs((short) src[0] - (short) src[offsets[4]]) < t) && (abs((short) src[0] - (short) src[offsets[12]]) < t))
		return;

	// Check for I_p + t
	short cand = (short) (*src) + t;
	for (int i = 0; i < 16; i++)
	{
		if ((short) src[offsets[i]] > cand)
			mask[0] |= iterMask;
		iterMask <<= 1;
	}

	// Check for I_p - t
	iterMask = 1;
	cand = (short) (*src) - t;
	for (int i = 0; i < 16; i++)
	{
		if ((short) src[offsets[i]] < cand)
			mask[1] |= iterMask;
		iterMask <<= 1;
	}
}

static inline bool isCorner(int mask[2])
{
	// Nine 1's in the LSB
	int cornerMask = 0x1FF;

	if (mask[0] || mask[1])
	{
		mask[0] = mask[0] | (mask[0] << 16);
		mask[1] = mask[1] | (mask[1] << 16);

		for (int i = 0; i < 16; i++)
		{
			if (((mask[0] & cornerMask) == cornerMask) || ((mask[1] & cornerMask) == cornerMask))
				return true;
			mask[0] >>= 1;
			mask[1] >>= 1;
		}
	}
	return false;
}

static inline bool isCorner(int mask)
{
	// Nine 1's in the LSB
	int cornerMask = 0x1FF;

	if (mask)
	{
		mask = mask | (mask << 16);
		for (int i = 0; i < 16; i++)
		{
			if ((mask & cornerMask) == cornerMask)
				return true;
			mask >>= 1;
		}
	}
	return false;
}

static inline bool isCornerPlus(short candidate, short * boundary, short t)
{
	// Early exit conditions
	// Pixels 1 and 9 within t of the candidate
	if ((abs(candidate - boundary[0]) < t) && (abs(candidate - boundary[8]) < t))
		return false;
	// Pixels 5 and 13 within t of the candidate
	if ((abs(candidate - boundary[4]) < t) && (abs(candidate - boundary[12]) < t))
		return false;

	candidate += t;
	int mask = 0;
	int iterMask = 1;
	for (int i = 0; i < 16; i++)
	{
		if (boundary[i] > candidate)
			mask |= iterMask;
		iterMask <<= 1;
	}
	return isCorner(mask);
}

static inline bool isCornerMinus(short candidate, short * boundary, short t)
{
	// Early exit conditions
	// Pixels 1 and 9 within t of the candidate
	if ((abs(candidate - boundary[0]) < t) && (abs(candidate - boundary[8]) < t))
		return false;
	// Pixels 5 and 13 within t of the candidate
	if ((abs(candidate - boundary[4]) < t) && (abs(candidate - boundary[12]) < t))
		return false;

	candidate -= t;
	int mask = 0;
	int iterMask = 1;
	for (int i = 0; i < 16; i++)
	{
		if (boundary[i] < candidate)
			mask |= iterMask;
		iterMask <<= 1;
	}
	return isCorner(mask);
}

static inline bool checkForCornerAndGetStrength(unsigned char * src, int* offsets, short t, short * strength)
{
	// Early exit conditions
	// Pixels 1 and 9 within t of the candidate
	if ((abs((short) src[0] - (short) src[offsets[0]]) < t) && (abs((short) src[0] - (short) src[offsets[8]]) < t))
		return false;
	// Pixels 5 and 13 within t of the candidate
	if ((abs((short) src[0] - (short) src[offsets[4]]) < t) && (abs((short) src[0] - (short) src[offsets[12]]) < t))
		return false;

	// Get boundary
	short boundary[16];
	for (int i = 0; i < 16; i++)
		boundary[i] = (short) src[offsets[i]];

	// Check for I_p + t
	short cand = (short) (*src) + t;
	int maskP = 0;
	int iterMask = 1;
	for (int i = 0; i < 16; i++)
	{
		if (boundary[i] > cand)
			maskP |= iterMask;
		iterMask <<= 1;
	}

	// If it is a corner, then compute the threshold
	short strength_pos = 0;
	cand = src[0];
	if (isCorner(maskP))
	{
		short thresh_upper = 255;
		short thresh_lower = t;

		// Binary search
		while (thresh_upper - thresh_lower > 1)
		{
			strength_pos = (thresh_upper + thresh_lower) >> 1;
			if (isCornerPlus(cand, boundary, strength_pos))
				thresh_lower = strength_pos;
			else
				thresh_upper = strength_pos;
		}
		strength_pos = thresh_lower;
	}

	// Check for I_p - t
	cand = (short) (*src) - t;
	int maskN = 0;
	iterMask = 1;
	for (int i = 0; i < 16; i++)
	{
		if (boundary[i] < cand)
			maskN |= iterMask;
		iterMask <<= 1;
	}

	// If it is a corner, then compute the threshold
	short strength_neg = 0;
	cand = src[0];
	if (isCorner(maskN))
	{
		short thresh_upper = 255;
		short thresh_lower = t;

		// Binary search
		while (thresh_upper - thresh_lower > 1)
		{
			strength_neg = (thresh_upper + thresh_lower) >> 1;
			if (isCornerMinus(cand, boundary, strength_neg))
				thresh_lower = strength_neg;
			else
				thresh_upper = strength_neg;
		}
		strength_neg = thresh_lower;
	}

	if (maskP || maskN)
	{
		*strength = max(strength_pos, strength_neg);
		return true;
	}
	return false;
}

#if ENABLE_MSA
bool isCorner_MSA(unsigned char pixel, v16i8 boundary, v8i16 t)
{
	// Check for boundary > pixel + t

	v16u8 vmask;
	unsigned char mask_arr[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	// The candidate pixel
	v8i16 cand = __builtin_msa_fill_h((short) pixel);

	// Pixel + t
	cand = __builtin_msa_addv_h(cand, t);

	// Boundary 8..15 (words)
	v16i8 temp0 = __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	v16i8 temp1 = __builtin_msa_ilvr_b(zeromask, boundary);

	temp0 = (v16i8) __builtin_msa_clt_s_h(cand, (v8i16) temp0);
	temp1 = (v16i8) __builtin_msa_clt_s_h(cand, (v8i16) temp1);

	// 255 at ith byte if boundary[i] > pixel + t
	temp1 = __builtin_msa_pckev_b(temp1, temp0);

	vmask = (v16u8) __builtin_msa_ld_w(&mask_arr, 0);
	temp1 = (v16i8) __builtin_msa_and_v((v16u8) temp1, vmask);
	temp1 = (v16i8) __builtin_msa_hadd_u_h((v16u8) temp1, (v16u8) temp1);
	temp1 = (v16i8) __builtin_msa_hadd_u_w((v8u16) temp1, (v8u16) temp1);
	temp1 = (v16i8) __builtin_msa_hadd_u_d((v4u32) temp1, (v4u32) temp1);
	int mask = __builtin_msa_copy_u_d((v2i64) temp1, 1);
	mask = (vx_uint16) mask << 8;
	mask = mask | __builtin_msa_copy_u_d((v2i64) temp1, 0);

	int plusMask = mask | (mask << 16);

	// Check for boundary > pixel - t
	// pixel + t - t = pixel
	cand = __builtin_msa_subv_h(cand, t);
	// pixel - t
	cand = __builtin_msa_subv_h(cand, t);

	// Boundary 8..15 (words)
	temp0 = __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	temp1 = __builtin_msa_ilvr_b(zeromask, boundary);

	temp0 = (v16i8) __builtin_msa_clt_s_h((v8i16) temp0, cand);
	temp1 = (v16i8) __builtin_msa_clt_s_h((v8i16) temp1, cand);

	// 255 at ith byte if boundary[i] > pixel + t
	temp1 = __builtin_msa_pckev_b(temp1, temp0);

	vmask = (v16u8) __builtin_msa_ld_w(&mask_arr, 0);
	temp1 = (v16i8) __builtin_msa_and_v((v16u8) temp1, vmask);
	temp1 = (v16i8) __builtin_msa_hadd_u_h((v16u8) temp1, (v16u8) temp1);
	temp1 = (v16i8) __builtin_msa_hadd_u_w((v8u16) temp1, (v8u16) temp1);
	temp1 = (v16i8) __builtin_msa_hadd_u_d((v4u32) temp1, (v4u32) temp1);
	mask = __builtin_msa_copy_u_d((v2i64) temp1, 1);
	mask = (vx_uint16) mask << 8;
	mask = mask | __builtin_msa_copy_u_d((v2i64) temp1, 0);

	int minusMask = mask | (mask << 16);

	if (plusMask || minusMask)
	{
		// Nine 1's in the LSB
		int cornerMask = 0x1FF;

		for (int i = 0; i < 16; i++)
		{
			if (((plusMask & cornerMask) == cornerMask) || ((minusMask & cornerMask) == cornerMask))
				return true;
			plusMask >>= 1;
			minusMask >>= 1;
		}
	}

	return false;
}

static inline bool isCornerPlus_MSA(v16i8 candidate, v16i8 boundary, short t)
{
	v16u8 vmask;
	unsigned char mask_arr[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	// Boundary 8..15 (words)
	v16i8 boundaryH = __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	v16i8 boundaryL = __builtin_msa_ilvr_b(zeromask, boundary);
	v8i16 threshold = __builtin_msa_fill_h(t);

	short cand =  ((int16_t*) &candidate)[0];

	// Early exit conditions
	// Pixels 1 and 9 within t of the candidate
	if ((abs(cand - ((int16_t*) &boundaryL)[0]) < t) && (abs(cand - ((int16_t*) &boundaryH)[0]) < t))
		return false;
	// Pixels 5 and 13 within t of the candidate
	if ((abs(cand - ((int16_t*) &boundaryL)[4]) < t) && (abs(cand - ((int16_t*) &boundaryH)[4]) < t))
		return false;

	candidate = __builtin_msa_addv_b(candidate, (v16i8) threshold);
	boundaryH = (v16i8) __builtin_msa_clt_s_h((v8i16) candidate, (v8i16) boundaryH);
	boundaryL = (v16i8) __builtin_msa_clt_s_h((v8i16) candidate, (v8i16) boundaryL);

	// 255 at ith byte if boundary[i] > pixel + t
	boundaryL = __builtin_msa_pckev_b(boundaryL, boundaryH);

	vmask = (v16u8) __builtin_msa_ld_w(&mask_arr, 0);
	boundaryL = (v16i8) __builtin_msa_and_v((v16u8) boundaryL, vmask);
	boundaryL = (v16i8) __builtin_msa_hadd_u_h((v16u8) boundaryL, (v16u8) boundaryL);
	boundaryL = (v16i8) __builtin_msa_hadd_u_w((v8u16) boundaryL, (v8u16) boundaryL);
	boundaryL = (v16i8) __builtin_msa_hadd_u_d((v4u32) boundaryL, (v4u32) boundaryL);
	int mask = __builtin_msa_copy_u_d((v2i64) boundaryL, 1);
	mask = (vx_uint16) mask << 8;
	mask = mask | __builtin_msa_copy_u_d((v2i64) boundaryL, 0);

	return isCorner(mask);
}

static inline bool isCornerMinus_MSA(v16i8 candidate, v16i8 boundary, short t)
{
	v16u8 vmask;
	unsigned char mask_arr[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	// Boundary 8..15 (words)
	v16i8 boundaryH = __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	v16i8 boundaryL = __builtin_msa_ilvr_b(zeromask, boundary);
	v8i16 threshold = __builtin_msa_fill_h(t);

	short cand = ((int16_t*) &candidate)[0];

	// Early exit conditions
	// Pixels 1 and 9 within t of the candidate
	if ((abs(cand - ((int16_t*) &boundaryL)[0]) < t) && (abs(cand - ((int16_t*) &boundaryH)[0]) < t))
		return false;
	// Pixels 5 and 13 within t of the candidate
	if ((abs(cand - ((int16_t*) &boundaryL)[4]) < t) && (abs(cand - ((int16_t*) &boundaryH)[4]) < t))
		return false;

	candidate = __builtin_msa_subv_b(candidate, (v16i8) threshold);
	boundaryH = (v16i8) __builtin_msa_clt_s_h((v8i16) boundaryH, (v8i16) candidate);
	boundaryL = (v16i8) __builtin_msa_clt_s_h((v8i16) boundaryL, (v8i16) candidate);

	// 255 at ith byte if boundary[i] > pixel + t
	boundaryL = __builtin_msa_pckev_b(boundaryH, boundaryL);

	vmask = (v16u8) __builtin_msa_ld_w(&mask_arr, 0);
	boundaryL = (v16i8) __builtin_msa_and_v((v16u8) boundaryL, vmask);
	boundaryL = (v16i8) __builtin_msa_hadd_u_h((v16u8) boundaryL, (v16u8) boundaryL);
	boundaryL = (v16i8) __builtin_msa_hadd_u_w((v8u16) boundaryL, (v8u16) boundaryL);
	boundaryL = (v16i8) __builtin_msa_hadd_u_d((v4u32) boundaryL, (v4u32) boundaryL);
	int mask = __builtin_msa_copy_u_d((v2i64) boundaryL, 1);
	mask = (vx_uint16) mask << 8;
	mask = mask | __builtin_msa_copy_u_d((v2i64) boundaryL, 0);

	return isCorner(mask);
}

static inline bool checkForCornerAndGetStrength_MSA(unsigned char pixel, v16i8 boundary, short threshold, short * strength)
{
	v16u8 vmask;
	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	v8i16 t = __builtin_msa_fill_h(threshold);

	// Check for boundary > pixel + t
	// The candidate pixel
	v8i16 cand = __builtin_msa_fill_h((short) pixel);

	// Pixel + t
	cand = __builtin_msa_addv_h(cand, t);

	// Boundary 8..15 (words)
	v8i16 temp0 = (v8i16) __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	v8i16 temp1 = (v8i16) __builtin_msa_ilvr_b(zeromask, boundary);

	temp0 = __builtin_msa_clt_s_h(cand, temp0);
	temp1 = __builtin_msa_clt_s_h(cand, temp1);

	// 255 at ith byte if boundary[i] > pixel + t
	temp1 = (v8i16) __builtin_msa_pckev_b((v16i8) temp0, (v16i8) temp1);

	vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
	temp1 = (v8i16) __builtin_msa_and_v((v16u8) temp1, vmask);
	temp1 = (v8i16) __builtin_msa_hadd_u_h((v16u8) temp1, (v16u8) temp1);
	temp1 = (v8i16) __builtin_msa_hadd_u_w((v8u16) temp1, (v8u16) temp1);
	temp1 = (v8i16) __builtin_msa_hadd_u_d((v4u32) temp1, (v4u32) temp1);
	int plusMask = __builtin_msa_copy_u_d((v2i64) temp1, 1);
	plusMask = (vx_uint16) plusMask << 8;
	plusMask = plusMask | __builtin_msa_copy_u_d((v2i64) temp1, 0);

	// If it is a corner, then compute the threshold
	short strength_pos = 0;
	cand = __builtin_msa_subv_h(cand, t);

	if (isCorner(plusMask))
	{
		short thresh_upper = 255;
		short thresh_lower = threshold;

		// Binary search
		while (thresh_upper - thresh_lower > 1)
		{
			strength_pos = (thresh_upper + thresh_lower) >> 1;
			if (isCornerPlus_MSA((v16i8) cand, boundary, strength_pos))
				thresh_lower = strength_pos;
			else
				thresh_upper = strength_pos;
		}
		strength_pos = thresh_lower;
	}

	// Check for boundary > pixel - t
	// pixel - t
	cand = __builtin_msa_subv_h(cand, t);

	// Boundary 8..15 (words)
	temp0 = (v8i16) __builtin_msa_ilvl_b(zeromask, boundary);
	// Boundary 0..7 (words)
	temp1 = (v8i16) __builtin_msa_ilvr_b(zeromask, boundary);

	temp0 = __builtin_msa_clt_s_h(temp0, cand);
	temp1 = __builtin_msa_clt_s_h(temp1, cand);

	// 255 at ith byte if boundary[i] > pixel + t
	temp1 = (v8i16) __builtin_msa_pckev_b((v16i8) temp0, (v16i8) temp1);

	vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
	temp1 = (v8i16) __builtin_msa_and_v((v16u8) temp1, vmask);
	temp1 = (v8i16) __builtin_msa_hadd_u_h((v16u8) temp1, (v16u8) temp1);
	temp1 = (v8i16) __builtin_msa_hadd_u_w((v8u16) temp1, (v8u16) temp1);
	temp1 = (v8i16) __builtin_msa_hadd_u_d((v4u32) temp1, (v4u32) temp1);
	int minusMask = __builtin_msa_copy_u_d((v2i64) temp1, 1);
	minusMask = (vx_uint16) minusMask << 8;
	minusMask = minusMask | __builtin_msa_copy_u_d((v2i64) temp1, 0);

	// If it is a corner, then compute the threshold
	short strength_neg = 0;
	cand = __builtin_msa_addv_h(cand, t);

	if (isCorner(minusMask))
	{
		short thresh_upper = 255;
		short thresh_lower = threshold;

		// Binary search
		while (thresh_upper - thresh_lower > 1)
		{
			strength_neg = (thresh_upper + thresh_lower) >> 1;
			if (isCornerMinus_MSA((v16i8) cand, boundary, strength_neg))
				thresh_lower = strength_neg;
			else
				thresh_upper = strength_neg;
		}
		strength_neg = thresh_lower;
	}
	if (plusMask || minusMask)
	{
		*strength = max(strength_pos, strength_neg);
		return true;
	}
	return false;
}
#endif

int HafCpu_FastCorners_XY_U8_NoSupression
	(
		vx_uint32       capacityOfDstCorner,
		vx_keypoint_t   dstCorner[],
		vx_uint32     * pDstCornerCount,
		vx_uint32       srcWidth,
		vx_uint32       srcHeight,
		vx_uint8      * pSrcImage,
		vx_uint32       srcImageStrideInBytes,
		vx_float32      strength_threshold
	)
{
	unsigned char * pLocalSrc;
	int srcStride = (int) srcImageStrideInBytes;
	vx_uint32 cornerCount = 0;
	short t = (short) floorf(strength_threshold);

	// Leave first three rows
	pSrcImage += (srcStride * 3);

	// Generate offsets for C code if necessary
	int neighbor_offset[16] = { 0 };

#if ENABLE_MSA
	int alignedWidth = (int) srcWidth & ~7;
	int postfixWidth = (int) srcWidth & 7;

	if (postfixWidth)
		generateOffset(srcStride, neighbor_offset);

	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v16u8 vmask;
	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
#else
	generateOffset(srcStride, neighbor_offset);
#endif

	for (int height = 0; height < (int) (srcHeight - 6); height++)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 0;
#if ENABLE_MSA
		for (int x = 0; x < (alignedWidth >> 3); x++)
		{
			v16i8 rowMinus3, rowMinus2, rowMinus1, row, rowPlus1, rowPlus2, rowPlus3;
			v8i16 thresh = __builtin_msa_fill_h(t);

			// Check for early escape based on pixels 1 and 9 around the candidate
			rowMinus3 = __builtin_msa_ld_b((void *) (pLocalSrc - 3 * srcStride - 1), 0);

			// row - 3: Pixels 0..7 in lower 7 bytes
			rowMinus2 = __builtin_msa_sldi_b(rowMinus3, rowMinus3, 1);
			rowMinus2 = __builtin_msa_ilvr_b(zeromask, rowMinus2);

			row = __builtin_msa_ld_b((void *) (pLocalSrc - 3), 0);

			// row: Pixels 0..7 in lower 7 bytes
			rowMinus1 = __builtin_msa_sldi_b(row, row, 3);
			rowMinus1 = __builtin_msa_ilvr_b(zeromask, rowMinus1);

			rowPlus3 = __builtin_msa_ld_b((void *) (pLocalSrc + 3 * srcStride - 1), 0);

			// row + 3: Pixels 0..7 in lower 7 bytes
			rowPlus2 = __builtin_msa_sldi_b(rowPlus3, rowPlus3, 1);
			rowPlus2 = __builtin_msa_ilvr_b(zeromask, rowPlus2);

			rowPlus1 = __builtin_msa_ld_b((void *) (pLocalSrc + srcStride - 3), 0);

			rowMinus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowMinus2, (v8i16) rowMinus1);
			rowPlus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowPlus2, (v8i16) rowMinus1);

			// Check if pixel 0 is less than 't' different from the candidate
			rowMinus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowMinus2, thresh);
			// Check if pixel 0 is less than 't' different from the candidate
			rowPlus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowPlus2, thresh);

			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowMinus2 = (v16i8) __builtin_msa_and_v((v16u8) rowMinus2, vmask);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowMinus2, (v16u8) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowMinus2, (v8u16) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowMinus2,(v4u32) rowMinus2);
			int maskSkip = __builtin_msa_copy_u_d((v2i64) rowMinus2, 1);
			maskSkip = (vx_uint16) maskSkip << 8;
			maskSkip = maskSkip | __builtin_msa_copy_u_d((v2i64) rowMinus2, 0);

			// 1 if both 0 and 8 are within 't' of the candidate pixel
			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowPlus2 = (v16i8) __builtin_msa_and_v((v16u8) rowPlus2, vmask);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowPlus2, (v16u8) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowPlus2, (v8u16) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowPlus2,(v4u32) rowPlus2);
			int maskSkip1 = __builtin_msa_copy_u_d((v2i64) rowPlus2, 1);
			maskSkip1 = (vx_uint16) maskSkip1 << 8;
			maskSkip1 = maskSkip1 | __builtin_msa_copy_u_d((v2i64) rowPlus2, 0);

			maskSkip &= maskSkip1;

			// Check for early escape based on pixels 12 and 4 around the candidate
			rowMinus2 = __builtin_msa_ilvr_b(zeromask, row);
			rowPlus2 = __builtin_msa_sldi_b(row, row, 6);
			rowPlus2 = __builtin_msa_ilvr_b(zeromask, rowPlus2);

			rowMinus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowMinus2, (v8i16) rowMinus1);
			rowPlus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowPlus2, (v8i16) rowMinus1);

			rowMinus1 = __builtin_msa_ld_b((void *) (pLocalSrc - srcStride - 3), 0);

			// Check if pixel 0 is less than 't' different from the candidate
			rowMinus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowMinus2, thresh);
			// Check if pixel 0 is less than 't' different from the candidate
			rowPlus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowPlus2, thresh);

			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowMinus2 = (v16i8) __builtin_msa_and_v((v16u8) rowMinus2, vmask);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowMinus2, (v16u8) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowMinus2, (v8u16) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowMinus2,(v4u32) rowMinus2);
			maskSkip1 = __builtin_msa_copy_u_d((v2i64) rowMinus2, 1);
			maskSkip1 = (vx_uint16) maskSkip1 << 8;
			maskSkip1 = maskSkip1 | __builtin_msa_copy_u_d((v2i64) rowMinus2, 0);

			rowMinus2 = __builtin_msa_ld_b((void *) (pLocalSrc - (srcStride + srcStride) - 2), 0);

			// 1 if both 0 and 8 are within 't' of the candidate pixel
			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowPlus2 = (v16i8) __builtin_msa_and_v((v16u8) rowPlus2, vmask);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowPlus2, (v16u8) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowPlus2, (v8u16) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowPlus2,(v4u32) rowPlus2);
			int maskSkip2 = __builtin_msa_copy_u_d((v2i64) rowPlus2, 1);
			maskSkip2 = (vx_uint16) maskSkip2 << 8;
			maskSkip2 = maskSkip2 | __builtin_msa_copy_u_d((v2i64) rowPlus2, 0);

			maskSkip1 &= maskSkip2;

			rowPlus2 = __builtin_msa_ld_b((void *) (pLocalSrc + (srcStride + srcStride) - 2), 0);

			maskSkip |= maskSkip1;

			// Check for corners in the eight pixels
			if (maskSkip != 0xFFFF)
			{
				for (int i = 0; i < 8; i++)
				{
					v16i8 *tbl = (v16i8 *) dataFastCornersPixelMask;

					if (!(maskSkip & 1))
					{
						// Get the boundary pixels in an MSA register
						v16i8 mask = __builtin_msa_ld_b((void *) tbl++, 0);
						v16i8 boundary = __builtin_msa_vshf_b(mask, (v16i8) rowMinus3, (v16i8) rowMinus3);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowMinus2, (v16i8) rowMinus2);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowMinus1, (v16i8) rowMinus1);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) row, (v16i8) row);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus1, (v16i8) rowPlus1);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus2, (v16i8) rowPlus2);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus3, (v16i8) rowPlus3);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						if (isCorner_MSA((unsigned char) row[3], boundary, thresh))
						{
							if (cornerCount < capacityOfDstCorner)
							{
								dstCorner[cornerCount].y = height + 3;
								dstCorner[cornerCount].x = width + i;
								// Undefined as per the 1.0.1 spec
								dstCorner[cornerCount].strength = strength_threshold;
								dstCorner[cornerCount].scale = 0;
								dstCorner[cornerCount].orientation = 0;
								dstCorner[cornerCount].error = 0;
								dstCorner[cornerCount++].tracking_status = 1;
							}
							else
								cornerCount++;
						}
					}
					maskSkip >>= 2;
					rowMinus3 = __builtin_msa_sldi_b(rowMinus3, rowMinus3, 1);
					rowMinus2 = __builtin_msa_sldi_b(rowMinus2, rowMinus2, 1);
					rowMinus1 = __builtin_msa_sldi_b(rowMinus1, rowMinus1, 1);
					row = __builtin_msa_sldi_b(row, row, 1);
					rowPlus1 = __builtin_msa_sldi_b(rowPlus1, rowPlus1, 1);
					rowPlus2 = __builtin_msa_sldi_b(rowPlus2, rowPlus2, 1);
					rowPlus3 = __builtin_msa_sldi_b(rowPlus3, rowPlus3, 1);
				}
			}

			width += 8;
			pLocalSrc += 8;
		}

		for (int x = 0; x < postfixWidth; x++)
		{
			int masks[2];
			generateMasks_C(pLocalSrc, srcStride, neighbor_offset, t, masks);
			if (isCorner(masks))
			{
				if (cornerCount < capacityOfDstCorner)
				{
					dstCorner[cornerCount].y = height + 3;
					dstCorner[cornerCount].x = width;
					// Undefined as per the 1.0.1 spec
					dstCorner[cornerCount].strength = strength_threshold;
					dstCorner[cornerCount].scale = 0;
					dstCorner[cornerCount].orientation = 0;
					dstCorner[cornerCount].error = 0;
					dstCorner[cornerCount++].tracking_status = 1;
				}
				else
					cornerCount++;
			}
			width++;
			pLocalSrc++;
		}
#else
		for (int x = 0; x < srcWidth; x++)
		{
			int masks[2];
			generateMasks_C(pLocalSrc, srcStride, neighbor_offset, t, masks);
			if (isCorner(masks))
			{
				if (cornerCount < capacityOfDstCorner)
				{
					dstCorner[cornerCount].y = height + 3;
					dstCorner[cornerCount].x = width;
					// Undefined as per the 1.0.1 spec
					dstCorner[cornerCount].strength = strength_threshold;
					dstCorner[cornerCount].scale = 0;
					dstCorner[cornerCount].orientation = 0;
					dstCorner[cornerCount].error = 0;
					dstCorner[cornerCount++].tracking_status = 1;
				}
				else
					cornerCount++;
			}
			width++;
			pLocalSrc++;
		}
#endif
		pSrcImage += srcStride;
	}

	*pDstCornerCount = cornerCount;
	return AGO_SUCCESS;
}

int HafCpu_FastCorners_XY_U8_Supression
	(
		vx_uint32       capacityOfDstCorner,
		vx_keypoint_t   dstCorner[],
		vx_uint32     * pDstCornerCount,
		vx_uint32       srcWidth,
		vx_uint32       srcHeight,
		vx_uint8      * pSrcImage,
		vx_uint32       srcImageStrideInBytes,
		vx_float32      strength_threshold,
		vx_uint8	  * pScratch
	)
{
	unsigned char * pLocalSrc;
	int srcStride = (int) srcImageStrideInBytes;
	vx_uint32 cornerCount = 0;
	short t = (short) floorf(strength_threshold);

	// Generate offsets for C code if necessary
	int neighbor_offset[16] = { 0 };

#if ENABLE_MSA
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v16u8 vmask;
	unsigned char mask[16] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

	int alignedWidth = (int) (srcWidth - 6) & ~7;
	int postfixWidth = (int) (srcWidth - 6) & 7;

	if (postfixWidth)
		generateOffset(srcStride, neighbor_offset);
#else
	generateOffset(srcStride, neighbor_offset);
#endif

	// Leave first three rows and start from the third pixel
	pSrcImage += (srcStride * 3) + 3;

	memset(pScratch, 0, sizeof(vx_uint8) * srcWidth * srcHeight);

	for (int height = 0; height < (int) (srcHeight - 6); height++)
	{
		pLocalSrc = (unsigned char *) pSrcImage;
		int width = 3;

#if ENABLE_MSA
		for (int x = 0; x < (alignedWidth >> 3); x++)
		{
			v16i8 rowMinus3, rowMinus2, rowMinus1, row, rowPlus1, rowPlus2, rowPlus3;
			v8i16 thresh = __builtin_msa_fill_h(t);

			// Check for early escape based on pixels 1 and 9 around the candidate
			rowMinus3 = __builtin_msa_ld_b((void *) (pLocalSrc - 3 * srcStride - 1), 0);
			// row - 3: Pixels 0..7 in lower 7 bytes
			rowMinus2 = __builtin_msa_sldi_b(rowMinus3, rowMinus3, 1);
			rowMinus2 = __builtin_msa_ilvr_b(zeromask, rowMinus2);

			row = __builtin_msa_ld_b((void *) (pLocalSrc - 3), 0);
			// row: Pixels 0..7 in lower 7 bytes
			rowMinus1 = __builtin_msa_sldi_b(row, row, 3);
			rowMinus1 = __builtin_msa_ilvr_b(zeromask, rowMinus1);

			rowPlus3 = __builtin_msa_ld_b((void *) (pLocalSrc + 3 * srcStride - 1), 0);

			// row + 3: Pixels 0..7 in lower 7 bytes
			rowPlus2 = __builtin_msa_sldi_b(rowPlus3, rowPlus3, 1);
			rowPlus2 = __builtin_msa_ilvr_b(zeromask, rowPlus2);

			rowPlus1 = __builtin_msa_ld_b((void *) (pLocalSrc + srcStride - 3), 0);

			rowMinus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowMinus2, (v8i16) rowMinus1);
			rowPlus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowPlus2, (v8i16) rowMinus1);

			// Check if pixel 0 is less than 't' different from the candidate
			rowMinus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowMinus2, thresh);
			// Check if pixel 0 is less than 't' different from the candidate
			rowPlus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowPlus2, thresh);

			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowMinus2 = (v16i8) __builtin_msa_and_v((v16u8) rowMinus2, vmask);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowMinus2, (v16u8) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowMinus2, (v8u16) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowMinus2,(v4u32) rowMinus2);
			int maskSkip = __builtin_msa_copy_u_d((v2i64) rowMinus2, 1);
			maskSkip = (vx_uint16) maskSkip << 8;
			maskSkip = maskSkip | __builtin_msa_copy_u_d((v2i64) rowMinus2, 0);

			// 1 if both 0 and 8 are within 't' of the candidate pixel
			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowPlus2 = (v16i8) __builtin_msa_and_v((v16u8) rowPlus2, vmask);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowPlus2, (v16u8) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowPlus2, (v8u16) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowPlus2,(v4u32) rowPlus2);
			int maskSkip1 = __builtin_msa_copy_u_d((v2i64) rowPlus2, 1);
			maskSkip1 = (vx_uint16) maskSkip1 << 8;
			maskSkip1 = maskSkip1 | __builtin_msa_copy_u_d((v2i64) rowPlus2, 0);

			maskSkip &= maskSkip1;

			// Check for early escape based on pixels 12 and 4 around the candidate
			rowMinus2 = __builtin_msa_ilvr_b(zeromask, row);
			rowPlus2 = __builtin_msa_sldi_b(row, row, 6);
			rowPlus2 = __builtin_msa_ilvr_b(zeromask, rowPlus2);

			rowMinus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowMinus2, (v8i16) rowMinus1);
			rowPlus2 = (v16i8) __builtin_msa_asub_s_h((v8i16) rowPlus2, (v8i16) rowMinus1);

			rowMinus1 = __builtin_msa_ld_b((void *) (pLocalSrc - srcStride - 3), 0);

			// Check if pixel 0 is less than 't' different from the candidate
			rowMinus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowMinus2, thresh);
			// Check if pixel 0 is less than 't' different from the candidate
			rowPlus2 = (v16i8) __builtin_msa_clt_s_h((v8i16) rowPlus2, thresh);

			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowMinus2 = (v16i8) __builtin_msa_and_v((v16u8) rowMinus2, vmask);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowMinus2, (v16u8) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowMinus2, (v8u16) rowMinus2);
			rowMinus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowMinus2,(v4u32) rowMinus2);
			maskSkip1 = __builtin_msa_copy_u_d((v2i64) rowMinus2, 1);
			maskSkip1 = (vx_uint16) maskSkip1 << 8;
			maskSkip1 = maskSkip1 | __builtin_msa_copy_u_d((v2i64) rowMinus2, 0);

			rowMinus2 = __builtin_msa_ld_b((void *) (pLocalSrc - (srcStride + srcStride) - 2), 0);

			// 1 if both 0 and 8 are within 't' of the candidate pixel
			vmask = (v16u8) __builtin_msa_ld_w(&mask, 0);
			rowPlus2 = (v16i8) __builtin_msa_and_v((v16u8) rowPlus2, vmask);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_h((v16u8) rowPlus2, (v16u8) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_w((v8u16) rowPlus2, (v8u16) rowPlus2);
			rowPlus2 = (v16i8) __builtin_msa_hadd_u_d((v4u32) rowPlus2,(v4u32) rowPlus2);
			int maskSkip2 = __builtin_msa_copy_u_d((v2i64) rowPlus2, 1);
			maskSkip2 = (vx_uint16) maskSkip2 << 8;
			maskSkip2 = maskSkip2 | __builtin_msa_copy_u_d((v2i64) rowPlus2, 0);

			maskSkip1 &= maskSkip2;

			rowPlus2 = __builtin_msa_ld_b((void *) (pLocalSrc + (srcStride + srcStride) - 2), 0);

			maskSkip |= maskSkip1;

			// Check for corners in the eight pixels
			if (maskSkip != 0xFFFF)
			{
				for (int i = 0; i < 8; i++)
				{
					v16i8 *tbl = (v16i8 *) dataFastCornersPixelMask;

					if (!(maskSkip & 1))
					{
						// Get the boundary pixels in an MSA register
						v16i8 mask = __builtin_msa_ld_b((void *) tbl++, 0);
						v16i8 boundary = __builtin_msa_vshf_b(mask, (v16i8) rowMinus3, (v16i8) rowMinus3);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowMinus2, (v16i8) rowMinus2);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowMinus1, (v16i8) rowMinus1);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) row, (v16i8) row);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus1, (v16i8) rowPlus1);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus2, (v16i8) rowPlus2);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						mask = __builtin_msa_ld_b((void *) tbl++, 0);
						mask = __builtin_msa_vshf_b(mask, (v16i8) rowPlus3, (v16i8) rowPlus3);
						boundary = (v16i8) __builtin_msa_or_v((v16u8) boundary, (v16u8) mask);

						short strength = 0;
						if (checkForCornerAndGetStrength_MSA(((uint8_t *) &row)[3], boundary, t, &strength))
							pScratch[(height + 3) * srcWidth + width + i] = (vx_uint8) strength;
					}
					maskSkip >>= 2;
					rowMinus3 = __builtin_msa_sldi_b(rowMinus3, rowMinus3, 1);
					rowMinus2 = __builtin_msa_sldi_b(rowMinus2, rowMinus2, 1);
					rowMinus1 = __builtin_msa_sldi_b(rowMinus1, rowMinus1, 1);
					row = __builtin_msa_sldi_b(row, row, 1);
					rowPlus1 = __builtin_msa_sldi_b(rowPlus1, rowPlus1, 1);
					rowPlus2 = __builtin_msa_sldi_b(rowPlus2, rowPlus2, 1);
					rowPlus3 = __builtin_msa_sldi_b(rowPlus3, rowPlus3, 1);
				}
			}
			width += 8;
			pLocalSrc += 8;
		}

		for (int x = 0; x < postfixWidth; x++)
		{
			short strength = 0;
			if (checkForCornerAndGetStrength(pLocalSrc, neighbor_offset, t, &strength))
				pScratch[(height + 3) * srcWidth + width] = (vx_uint8) strength;
			width++;
			pLocalSrc++;
		}
#else
		for (int x = 0; x < srcWidth; x++)
		{
			short strength = 0;
			if (checkForCornerAndGetStrength(pLocalSrc, neighbor_offset, t, &strength))
				pScratch[(height + 3) * srcWidth + width] = (vx_uint8) strength;
			width++;
			pLocalSrc++;
		}
#endif
		pSrcImage += srcStride;
	}

	// Non-max supression
	pScratch += (3 * srcWidth + 3);
	cornerCount = 0;
	for (int height = 0; height < int(srcHeight - 6); height++)
	{
		for (int width = 0; width < int(srcWidth - 6); width++)
		{
			vx_uint8 * prev = pScratch - srcWidth;
			vx_uint8 * nxt = pScratch + srcWidth;
			vx_uint8 cand = *pScratch;
			if (cand && (cand >= *(prev - 1)) && (cand >= *prev) && (cand >= *(prev + 1))
				&& (cand >= *(pScratch - 1)) && (cand > *(pScratch + 1))
				&& (cand > *(nxt - 1)) && (cand > *nxt) && (cand > *(nxt + 1)))
			{
				if (cornerCount < capacityOfDstCorner)
				{
					dstCorner[cornerCount].x = (vx_int32) (width + 3);
					dstCorner[cornerCount].y = (vx_int32) (height + 3);
					dstCorner[cornerCount].strength = (vx_float32) cand;
					dstCorner[cornerCount].scale = 0;
					dstCorner[cornerCount].orientation = 0;
					dstCorner[cornerCount].error = 0;
					dstCorner[cornerCount++].tracking_status = 1;
				}
				else
					cornerCount++;
			}
			pScratch++;
		}
		pScratch += 6;
	}
	*pDstCornerCount = cornerCount;
	return AGO_SUCCESS;
}
