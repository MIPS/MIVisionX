#include "ago_internal.h"
#include "mips_internal.h"

static const int n_offset[][2][2] = {
	{ { -1,  0 }, {  1, 0 } },
	{ {  1, -1 }, { -1, 1 } },
	{ {  0, -1 }, {  0, 1 } },
	{ { -1, -1 }, {  1, 1 } },
};
static const ago_coord2d_short_t dir_offsets[8] = {
	{ -1, -1 },
	{  0, -1 },
	{ +1, -1 },
	{ -1,  0 },
	{ +1,  0 },
	{ -1, +1 },
	{  0, +1 },
	{ +1, +1 },
};

#if ENABLE_MSA
v8i16 HafCpu_FastAtan2_Canny_MSA
	(
		v8i16	 Gx,
		v8i16	 Gy
	)
{
	v8i16 ax, ay;
	v4i32 axL, axH, ayL, ayH;
	v4f32 tg22, tg67;
	v8i16 zeromask = __builtin_msa_ldi_h(0);
	v8u16 one = (v8u16) __builtin_msa_ldi_h(1);
	v4u32 two = (v4u32) __builtin_msa_ldi_w(2);
	v8u16 three = (v8u16) __builtin_msa_ldi_h(3);
	v4i32 cmp1L, cmp1H, cmp2L, cmp2H;
	v8u16 ret;
	v4u32 retL, retH;

	float ftg22[4] = {0.4142135623730950488016887242097f, 0.4142135623730950488016887242097f,
					  0.4142135623730950488016887242097f, 0.4142135623730950488016887242097f};
	float ftg67[4] = {2.4142135623730950488016887242097f, 2.4142135623730950488016887242097f,
					  2.4142135623730950488016887242097f, 2.4142135623730950488016887242097f};

	tg22 = (v4f32) __builtin_msa_ld_w(&ftg22, 0);
	tg67 = (v4f32) __builtin_msa_ld_w(&ftg67, 0);

	// abs(ax)
	ax = __builtin_msa_add_a_h(Gx, zeromask);
	axL = (v4i32) __builtin_msa_ilvl_h(zeromask, (v8i16) ax);
	axH = (v4i32) __builtin_msa_ilvr_h(zeromask, (v8i16) ax);

	// abs(ay)
	ay = __builtin_msa_add_a_h(Gy, zeromask);
	ayL = (v4i32) __builtin_msa_ilvl_h(zeromask, (v8i16) ay);
	ayH = (v4i32) __builtin_msa_ilvr_h(zeromask, (v8i16) ay);

	v4f32 axLf, axHf, ayLf, ayHf;

	axLf = __builtin_msa_ffint_s_w(axL);
	axHf = __builtin_msa_ffint_s_w(axH);
	ayLf = __builtin_msa_ffint_s_w(ayL);
	ayHf = __builtin_msa_ffint_s_w(ayH);

	// calc d1
	v4f32 d1L  = __builtin_msa_fmul_w(axLf, tg22);
	v4f32 d1H  = __builtin_msa_fmul_w(axHf, tg22);

	//calc d2
	v4f32 d2L  = __builtin_msa_fmul_w(axLf, tg67);
	v4f32 d2H  = __builtin_msa_fmul_w(axHf, tg67);

	// ret = (Gx*Gy) < 0 ? 3 : 1;
	// check most significant bit
	ret = (v8u16) __builtin_msa_xor_v((v16u8) Gx,(v16u8) Gy);
	ret = (v8u16) __builtin_msa_clti_s_h((v8i16) ret, 0);
	ret = (v8u16) __builtin_msa_bsel_v((v16u8) ret, (v16u8) one, (v16u8) three);
	// if Gx=0 or Gy=0 than ret=1
	v8i16 Gx0 = __builtin_msa_ceqi_h(Gx, 0);
	v8i16 Gy0 = __builtin_msa_ceqi_h(Gy, 0);
	ret = (v8u16) __builtin_msa_bmnz_v((v16u8) ret, (v16u8) one, (v16u8) Gx0);
	ret = (v8u16) __builtin_msa_bmnz_v((v16u8) ret, (v16u8) one, (v16u8) Gy0);

	retL = (v4u32) __builtin_msa_ilvl_h(zeromask, (v8i16) ret);
	retH = (v4u32) __builtin_msa_ilvr_h(zeromask, (v8i16) ret);

	// if (ay <= d1) ret = 0;
	cmp1L = __builtin_msa_fcle_w(d1L, ayLf);
	retL = (v4u32) __builtin_msa_and_v((v16u8) retL, (v16u8) cmp1L);
	cmp1H = __builtin_msa_fcle_w(d1H, ayHf);
	retH = (v4u32) __builtin_msa_and_v((v16u8) retH, (v16u8) cmp1H);

	// if (ay >= d2) ret = 2;
	cmp2L = __builtin_msa_fcle_w(d2L, ayLf);
	retL = (v4u32) __builtin_msa_bmnz_v((v16u8) retL, (v16u8) two, (v16u8) cmp2L);
	cmp2H = __builtin_msa_fcle_w(d2H, ayHf);
	retH = (v4u32) __builtin_msa_bmnz_v((v16u8) retH, (v16u8) two, (v16u8) cmp2H);

	ret = (v8u16) __builtin_msa_pckev_h((v8i16) retL, (v8i16) retH);

	return (v8i16) ret;
}
#endif

int HafCpu_CannySobel_U16_U8_3x3_L1NORM
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint16   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	)
{
	int x, y;
	pSrcImage += srcImageStrideInBytes;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += dstride;		// don't care about border. start processing from row2
#if ENABLE_MSA
	int prefixWidth = ((intptr_t) (pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16 *) (pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 s0, s1, s2, s3, s4;
#endif

	for (y = 1; y < (int) dstHeight - 1; y++)
	{
		const vx_uint8* srow0 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage;
		const vx_uint8* srow2 = pSrcImage + srcImageStrideInBytes;
		vx_uint16* drow = (vx_uint16 *) pDstImage;

#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16) srow0[x + 1] - (vx_int16) srow0[x - 1] + (vx_int16) srow2[x + 1] - (vx_int16) srow2[x - 1] + 2 * ((vx_int16) srow1[x + 1] - (vx_int16) srow1[x - 1]);
			vx_int16 Gy = (vx_int16) srow2[x - 1] + (vx_int16) srow2[x + 1] - (vx_int16) srow0[x - 1] - (vx_int16) srow0[x + 1] + 2 * ((vx_int16) srow2[x] - (vx_int16) srow0[x]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);

			v8i16 t0 = __builtin_msa_addv_h(__builtin_msa_addv_h(s0, s2), __builtin_msa_slli_h(s1, 1));
			v8i16 t1 = __builtin_msa_subv_h(s2, s0);

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);

		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);

			v8i16 t0 = __builtin_msa_subv_h(s1, s0);
			v8i16 t1 =  __builtin_msa_addv_h(__builtin_msa_addv_h(s2, s4),__builtin_msa_slli_h(s3, 1));
			t1 = __builtin_msa_subv_h((v8i16) zeromask, t1);
			s1 =  HafCpu_FastAtan2_Canny_MSA(t0, t1);

			t0 = __builtin_msa_add_a_h(t0, t1);
			t0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(t0, 2), (v16u8) s1);
			 __builtin_msa_st_h(t0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int) dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16) srow0[x + 1] - (vx_int16) srow0[x - 1] + (vx_int16) srow2[x + 1] - (vx_int16) srow2[x - 1] + 2 * ((vx_int16) srow1[x + 1] - (vx_int16) srow1[x - 1]);
			vx_int16 Gy = (vx_int16) srow2[x - 1] + (vx_int16) srow2[x + 1] - (vx_int16) srow0[x - 1] - (vx_int16) srow0[x + 1] + 2 * ((vx_int16) srow2[x] - (vx_int16) srow0[x]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int) dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 1] - (vx_int16)srow0[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow2[x - 1] + 2 * ((vx_int16)srow1[x + 1] - (vx_int16)srow1[x - 1]);
			vx_int16 Gy = (vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow0[x - 1] - (vx_int16)srow0[x + 1] + 2 * ((vx_int16)srow2[x] - (vx_int16)srow0[x]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
	    }
#endif

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}



// Using separable filter
//			-1	-2	0	2	1			1
//										4
//  Gx =								6
//										4
//										1

int HafCpu_CannySobel_U16_U8_5x5_L1NORM
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint16   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	)
{
	int x, y;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += 2 * dstride;		// don't care about border. start processing from row2
	pSrcImage += 2 * srcImageStrideInBytes;
#if ENABLE_MSA
	int prefixWidth = ((intptr_t)(pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int)dstWidth - prefixWidth) & 15;
	int alignedWidth = (int)dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16*)(pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 c6 = __builtin_msa_ldi_h(6);
	v8i16 s0, s1, s2, s3, s4, s5, s6, s7, s8;
#endif

	for (y = 2; y < (int)dstHeight - 2; y++)
	{
		const vx_uint8* srow0 = pSrcImage - 2 * srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow2 = pSrcImage;
		const vx_uint8* srow3 = pSrcImage + srcImageStrideInBytes;
		const vx_uint8* srow4 = pSrcImage + 2 * srcImageStrideInBytes;

		vx_uint16* drow = (vx_uint16*)pDstImage;

#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ (vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2];
			vx_int16 Gy = (vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2]
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		for (x = prefixWidth; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);
			s3 = __builtin_msa_ld_h((void *) (srow3 + x), 0);
			s4 = __builtin_msa_ld_h((void *) (srow4 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);
			s3 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s3);
			s4 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s4);

			v8i16 t0 = __builtin_msa_addv_h(__builtin_msa_slli_h(__builtin_msa_addv_h(s1, s3), 2),	__builtin_msa_mulv_h(s2, c6));
			t0 = __builtin_msa_addv_h(t0, __builtin_msa_addv_h(s0, s4));

			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_subv_h(s3, s1), 1);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_subv_h(s4, s0));

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);
		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 2), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 2), 0);

			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 2), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s6 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s7 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);
			s8 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 2), 0);

			v8i16 t0 = __builtin_msa_slli_h(__builtin_msa_subv_h(s2, s1), 1);
			t0= __builtin_msa_adds_s_h(t0, __builtin_msa_subv_h(s3, s0));
			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_addv_h(s5, s7), 2);

			s0 = __builtin_msa_mulv_h(s6, c6);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_addv_h(s4, s8));
			t1 = __builtin_msa_addv_h(t1, s0);
			t1 = __builtin_msa_subv_h((v8i16)zeromask, t1);

			// find magnitude
			s0 = __builtin_msa_add_a_h(t0, t1);

			t0 = HafCpu_FastAtan2_Canny_MSA(t0, t1);

			s0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(s0, 2), (v16u8) t0);

			// store magnitude and angle to destination
			 __builtin_msa_st_h(s0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ (vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2];
			vx_int16 Gy = (vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2]
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ ((vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2]);
			vx_int16 Gy = ((vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2])
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}

int HafCpu_CannySobel_U16_U8_7x7_L1NORM
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint16   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	)
{
	int x, y;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += 3 * dstride;		// don't care about border. start processing from row2
	pSrcImage += 3 * srcImageStrideInBytes;
#if ENABLE_MSA
	int prefixWidth = ((intptr_t)(pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int)dstWidth - prefixWidth) & 15;
	int alignedWidth = (int)dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16*)(pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 c5 = __builtin_msa_ldi_h(5);
	v8i16 c6 = __builtin_msa_ldi_h(6);
	v8i16 c15 = __builtin_msa_ldi_h(15);
	v8i16 c20 = __builtin_msa_ldi_h(20);
	v8i16 clamp = __builtin_msa_fill_h((int) 0x3FFF);
	v8i16 s0, s1, s2, s3, s4, s5, s6, s7, s8;
#endif

	for (y = 3; y < (int)dstHeight - 3; y++)
	{
		const vx_uint8* srow0 = pSrcImage - 3 * srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage - 2 * srcImageStrideInBytes;
		const vx_uint8* srow2 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow3 = pSrcImage;
		const vx_uint8* srow4 = pSrcImage + srcImageStrideInBytes;
		const vx_uint8* srow5 = pSrcImage + 2 * srcImageStrideInBytes;
		const vx_uint8* srow6 = pSrcImage + 3 * srcImageStrideInBytes;

		vx_uint16* drow = (vx_uint16*)pDstImage;
#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		for (x = prefixWidth; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);
			s3 = __builtin_msa_ld_h((void *) (srow3 + x), 0);
			s4 = __builtin_msa_ld_h((void *) (srow4 + x), 0);
			s5 = __builtin_msa_ld_h((void *) (srow5 + x), 0);
			s6 = __builtin_msa_ld_h((void *) (srow6 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);
			s3 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s3);
			s4 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s4);
			s5 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s5);
			s6 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s6);

			v8i16 t0 =  __builtin_msa_addv_h(__builtin_msa_mulv_h(__builtin_msa_addv_h(s1, s5), c6), __builtin_msa_mulv_h(s3, c20));
			v8i16 t2 =  __builtin_msa_mulv_h(__builtin_msa_addv_h(s2, s4), c15);

			t0 = __builtin_msa_addv_h(t0, __builtin_msa_addv_h(s0, s6));
			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_subv_h(s5, s1), 2);
			t0 = __builtin_msa_addv_h(t0, t2);

			t2 = __builtin_msa_mulv_h(__builtin_msa_subv_h(s4, s2), c5);
			t0 = __builtin_msa_srai_h(t0, 2);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_subv_h(s6, s0));
			t1 = __builtin_msa_addv_h(t1, t2);
			t1 = __builtin_msa_srai_h(t1, 2);

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);
		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 3), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 2), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 2), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 3), 0);

			v8i16 t0 = __builtin_msa_slli_h(__builtin_msa_subs_s_h(s4, s1), 2);
			v8i16 t1 = __builtin_msa_mulv_h(__builtin_msa_subs_s_h(s3, s2), c5);
			t0 = __builtin_msa_adds_s_h(t0, __builtin_msa_subs_s_h(s5, s0));
			t0 = __builtin_msa_adds_s_h(t0, t1);

			s0 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 3), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 2), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 2), 0);
			s6 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 3), 0);

			t1 = __builtin_msa_adds_s_h(__builtin_msa_mulv_h(__builtin_msa_addv_h(s1, s5), c6), __builtin_msa_mulv_h(s3, c20));
			v8i16 t2 = __builtin_msa_mulv_h(__builtin_msa_addv_h(s2, s4), c15);
			t1 = __builtin_msa_adds_s_h(t1, __builtin_msa_adds_s_h(s0, s6));
			t1 = __builtin_msa_adds_s_h(t1, t2);
			t1 = __builtin_msa_subs_s_h((v8i16) zeromask, t1);

			// find magnitude
			s0 = __builtin_msa_add_a_h(t0, t1);
			s0 = __builtin_msa_min_s_h(s0, clamp);

			t0 = HafCpu_FastAtan2_Canny_MSA(t0, t1);

			s0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(s0, 2), (v16u8) t0);

			// store magnitude and angle to destination
			 __builtin_msa_st_h(s0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = abs(Gx) + abs(Gy);
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}


int HafCpu_CannySuppThreshold_U8XY_U16_3x3
	(
		vx_uint32	       capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32	     * pxyStackTop,
		vx_uint32	       dstWidth,
		vx_uint32	       dstHeight,
		vx_uint8	     * pDst,
		vx_uint32	       dstStrideInBytes,
		vx_uint16	     * pSrc,
		vx_uint32	       srcStrideInBytes,
		vx_uint16		hyst_lower,
		vx_uint16		hyst_upper
	)
{
	// do minmax suppression: from Gx
	vx_uint32 sstride = srcStrideInBytes >> 1;
	ago_coord2d_ushort_t *pxyStack = xyStack;
	for (unsigned int y = 1; y < dstHeight - 1; y++)
	{
		vx_uint8 *pOut = pDst + y * dstStrideInBytes;
		vx_uint16 *pLocSrc = pSrc + y * sstride + 1;	// we are processing from 2nd row
		for (unsigned int x = 1; x < dstWidth - 1; x++, pLocSrc++)
		{
			vx_int32 edge;
			// get the Mag and angle
			int mag = (pLocSrc[0] >> 2);
			int ang = pLocSrc[0] & 3;
			int offset0 = n_offset[ang][0][1] * sstride + n_offset[ang][0][0];
			int offset1 = n_offset[ang][1][1] * sstride + n_offset[ang][1][0];
			edge = ((mag >(pLocSrc[offset0] >> 2)) && (mag >(pLocSrc[offset1] >> 2))) ? mag : 0;
			if (edge > hyst_upper)
			{
				pOut[x] = (vx_int8) 255;
				// add the cordinates to stacktop
				pxyStack->x = x;	// store x and y co-ordinates
				pxyStack->y = y;	// store x and y co-ordinates
				pxyStack++;
			}
			else if (edge <= hyst_lower)
			{
				pOut[x] = 0;
			}
			else pOut[x] = 127;
		}
	}
	*pxyStackTop = (vx_uint32) (pxyStack - xyStack);
	return AGO_SUCCESS;
}

int HafCpu_CannyEdgeTrace_U8_U8XY
	(
		vx_uint32	       dstWidth,
		vx_uint32	       dstHeight,
		vx_uint8	     * pDstImage,
		vx_uint32	       dstImageStrideInBytes,
		vx_uint32	       capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32	       xyStackTop
	)
{
	ago_coord2d_ushort_t *pxyStack = xyStack + xyStackTop;
	while (pxyStack != xyStack)
	{
			pxyStack--;
			vx_uint16 x = pxyStack->x;
			vx_uint16 y = pxyStack->y;
			// look at all the neighbors for strong edge value
		for (int i = 0; i < 8; i++)
		{
			const ago_coord2d_short_t offs = dir_offsets[i];
			vx_int16 x1 = x + offs.x;
			vx_int16 y1 = y + offs.y;
			vx_uint8 *pDst = pDstImage + y1 * dstImageStrideInBytes + x1;
			if (*pDst == 127)
			{
				*pDst |= 0x80;		// *pDst = 255
				*((unsigned *) pxyStack) = (y1 << 16) | x1;
				pxyStack++;
			}
		}
	}
	// go through the entire destination and convert all 127 to 0
#if ENABLE_MSA
	const v16u8 v127 = (v16u8) __builtin_msa_ldi_b((char) 127);
	for (unsigned int y = 0; y < dstHeight; y++)
	{
		v16u8 * src = (v16u8 *) pDstImage;
		vx_uint32 width = (dstWidth + 15) >> 4;
		for (unsigned int x = 0; x < width; x++)
		{
			v16u8 mask;
			v16u8 pixels = (v16u8) __builtin_msa_ld_b(src, 0);
			mask = (v16u8) __builtin_msa_ceq_b((v16i8) pixels,(v16i8) v127);
			mask =	__builtin_msa_xori_b( mask, 255);
			pixels = __builtin_msa_and_v(mask, pixels);
			__builtin_msa_st_b((v16i8) pixels, (void *) src++, 0);
		}
		pDstImage += dstImageStrideInBytes;
	}
#else
	for (unsigned int y = 0; y < dstHeight; y++)
	{
		vx_uint8 *src = (vx_uint8 *) pDstImage;
		for (unsigned int x = 0; x < dstWidth; x++)
		{
			vx_uint8 pixels = *src;
			if (pixels == 127)
				pixels = 0;
			*src++ = pixels;
		}
		pDstImage += dstImageStrideInBytes;
	}
#endif

	return AGO_SUCCESS;
}

int HafCpu_CannySobel_U16_U8_3x3_L2NORM
(
	vx_uint32     dstWidth,
	vx_uint32     dstHeight,
	vx_uint16   * pDstImage,
	vx_uint32     dstImageStrideInBytes,
	vx_uint8    * pSrcImage,
	vx_uint32     srcImageStrideInBytes,
	vx_uint8    * pLocalData
)
{
	int x, y;
	pSrcImage += srcImageStrideInBytes;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += dstride;		// don't care about border. start processing from row2
#if ENABLE_MSA
	int prefixWidth = ((intptr_t)(pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int)dstWidth - prefixWidth) & 15;
	int alignedWidth = (int)dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16*)(pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 s0, s1, s2, s3, s4;
#endif

	for (y = 1; y < (int)dstHeight - 1; y++)
	{
		const vx_uint8* srow0 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage;
		const vx_uint8* srow2 = pSrcImage + srcImageStrideInBytes;
		vx_uint16* drow = (vx_uint16*)pDstImage;

#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 1] - (vx_int16)srow0[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow2[x - 1] + 2 * ((vx_int16)srow1[x + 1] - (vx_int16)srow1[x - 1]);
			vx_int16 Gy = (vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow0[x - 1] - (vx_int16)srow0[x + 1] + 2 * ((vx_int16)srow2[x] - (vx_int16)srow0[x]);
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			Gy = ~Gy + 1;
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);

			v8i16 t0 = __builtin_msa_addv_h(__builtin_msa_addv_h(s0, s2), __builtin_msa_slli_h(s1, 1));
			v8i16 t1 = __builtin_msa_subv_h(s2, s0);

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);
		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);

			v8i16 t0 = __builtin_msa_subv_h(s1, s0);
			v8i16 t1 =  __builtin_msa_addv_h(__builtin_msa_addv_h(s2, s4),__builtin_msa_slli_h(s3, 1));
			t1 = __builtin_msa_subv_h((v8i16) zeromask, t1);

			s0 = __builtin_msa_mulv_h(t0, t0);
			s1 = __builtin_msa_mulv_h(t1, t1);

			// unpack to dwords for multiplication
			s2 = __builtin_msa_ilvl_h((v8i16) zeromask, s0);
			s0 = __builtin_msa_ilvr_h((v8i16) zeromask, s0);
			s3 = __builtin_msa_ilvl_h((v8i16) zeromask, s1);
			s1 = __builtin_msa_ilvr_h((v8i16) zeromask, s1);

			v4f32 f0 = __builtin_msa_ffint_s_w((v4i32) s0);
			v4f32 f1 = __builtin_msa_ffint_s_w((v4i32) s2);
			v4f32 f2 = __builtin_msa_ffint_s_w((v4i32) s1);
			v4f32 f3 = __builtin_msa_ffint_s_w((v4i32) s3);


			f0 = __builtin_msa_fadd_w(f0, f2);
			f1 = __builtin_msa_fadd_w(f1, f3);
			f0 = __builtin_msa_fsqrt_w(f0);
			f1 = __builtin_msa_fsqrt_w(f1);

			s1 = HafCpu_FastAtan2_Canny_MSA(t0, t1);

			t0 = (v8i16) __builtin_msa_ftint_s_w(f0);
			t1 = (v8i16) __builtin_msa_ftint_s_w(f1);

			// pack with signed saturation
			v4u32 temp0_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t0, 15);
			v4u32 temp1_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t1, 15);
			t0 = __builtin_msa_pckev_h((v8i16)temp1_u, (v8i16)temp0_u);

			// // store magnitude and angle to destination
			t0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(t0, 2), (v16u8) s1);
			 __builtin_msa_st_h(t0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 1] - (vx_int16)srow0[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow2[x - 1] + 2 * ((vx_int16)srow1[x + 1] - (vx_int16)srow1[x - 1]);
			vx_int16 Gy = (vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow0[x - 1] - (vx_int16)srow0[x + 1] + 2 * ((vx_int16)srow2[x] - (vx_int16)srow0[x]);
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			Gy = ~Gy + 1;
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 1] - (vx_int16)srow0[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow2[x - 1] + 2 * ((vx_int16)srow1[x + 1] - (vx_int16)srow1[x - 1]);
			vx_int16 Gy = (vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1] - (vx_int16)srow0[x - 1] - (vx_int16)srow0[x + 1] + 2 * ((vx_int16)srow2[x] - (vx_int16)srow0[x]);
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}

int HafCpu_CannySobel_U16_U8_5x5_L2NORM
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint16   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	)
{
	int x, y;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += 2 * dstride;		// don't care about border. start processing from row2
	pSrcImage += 2 * srcImageStrideInBytes;
#if ENABLE_MSA
	int prefixWidth = ((intptr_t)(pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int)dstWidth - prefixWidth) & 15;
	int alignedWidth = (int)dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16*)(pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 c6 = __builtin_msa_ldi_h(6);
	v8i16 s0, s1, s2, s3, s4, s5, s6, s7, s8;
#endif

	for (y = 2; y < (int)dstHeight - 2; y++)
	{
		const vx_uint8* srow0 = pSrcImage - 2 * srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow2 = pSrcImage;
		const vx_uint8* srow3 = pSrcImage + srcImageStrideInBytes;
		const vx_uint8* srow4 = pSrcImage + 2 * srcImageStrideInBytes;

		vx_uint16* drow = (vx_uint16*)pDstImage;

#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ (vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2];
			vx_int16 Gy = (vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2]
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		for (x = prefixWidth; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);
			s3 = __builtin_msa_ld_h((void *) (srow3 + x), 0);
			s4 = __builtin_msa_ld_h((void *) (srow4 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);
			s3 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s3);
			s4 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s4);

			v8i16 t0 = __builtin_msa_addv_h(__builtin_msa_slli_h(__builtin_msa_addv_h(s1, s3), 2),	__builtin_msa_mulv_h(s2, c6));
			t0 = __builtin_msa_addv_h(t0, __builtin_msa_addv_h(s0, s4));

			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_subv_h(s3, s1), 1);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_subv_h(s4, s0));

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);
		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 2), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 2), 0);

			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 2), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s6 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s7 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);
			s8 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 2), 0);

			v8i16 t0 = __builtin_msa_slli_h(__builtin_msa_subv_h(s2, s1), 1);
			t0= __builtin_msa_adds_s_h(t0, __builtin_msa_subv_h(s3, s0));
			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_addv_h(s5, s7), 2);

			s0 = __builtin_msa_mulv_h(s6, c6);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_addv_h(s4, s8));
			t1 = __builtin_msa_adds_s_h(t1, s0);
			t1 = __builtin_msa_subv_h((v8i16)zeromask, t1);

			s0 = __builtin_msa_ilvr_h(t1, t0);
			s2 = __builtin_msa_ilvl_h(t1, t0);

			s0 = (v8i16) __builtin_msa_dotp_s_w(s0, s0);
			s2 = (v8i16) __builtin_msa_dotp_s_w(s2, s2);

			v4f32 f0 = __builtin_msa_ffint_s_w((v4i32) s0);
			v4f32 f1 = __builtin_msa_ffint_s_w((v4i32) s2);

			f0 = __builtin_msa_fsqrt_w(f0);
			f1 = __builtin_msa_fsqrt_w(f1);

			s1 = HafCpu_FastAtan2_Canny_MSA(t0, t1);

			t0 = (v8i16) __builtin_msa_ftint_s_w(f0);
			t1 = (v8i16) __builtin_msa_ftint_s_w(f1);

			// pack with signed saturation
			v4u32 temp0_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t0, 15);
			v4u32 temp1_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t1, 15);
			t0 = __builtin_msa_pckev_h((v8i16)temp1_u, (v8i16)temp0_u);

			// store magnitude and angle to destination
			t0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(t0, 2), (v16u8) s1);
			 __builtin_msa_st_h(t0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ (vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2];
			vx_int16 Gy = (vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2]
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 2] + (2 * ((vx_int16)srow0[x + 1])) - (2 * ((vx_int16)srow0[x - 1])) - (vx_int16)srow0[x - 2]
				+ 4 * ((vx_int16)srow1[x + 2] + (2 * ((vx_int16)srow1[x + 1])) - (2 * ((vx_int16)srow1[x - 1])) - (vx_int16)srow1[x - 2])
				+ 6 * ((vx_int16)srow2[x + 2] + (2 * ((vx_int16)srow2[x + 1])) - (2 * ((vx_int16)srow2[x - 1])) - (vx_int16)srow2[x - 2])
				+ 4 * ((vx_int16)srow3[x + 2] + (2 * ((vx_int16)srow3[x + 1])) - (2 * ((vx_int16)srow3[x - 1])) - (vx_int16)srow3[x - 2])
				+ ((vx_int16)srow4[x + 2] + (2 * ((vx_int16)srow4[x + 1])) - (2 * ((vx_int16)srow4[x - 1])) - (vx_int16)srow4[x - 2]);
			vx_int16 Gy = ((vx_int16)srow4[x - 2] + (4 * (vx_int16)srow4[x - 1]) + (6 * (vx_int16)srow4[x]) + (4 * (vx_int16)srow4[x + 1]) + (vx_int16)srow4[x + 2])
				+ 2 * ((vx_int16)srow3[x - 2] + (4 * (vx_int16)srow3[x - 1]) + (6 * (vx_int16)srow3[x]) + (4 * (vx_int16)srow3[x + 1]) + (vx_int16)srow3[x + 2])
				- 2 * ((vx_int16)srow1[x - 2] + (4 * (vx_int16)srow1[x - 1]) + (6 * (vx_int16)srow1[x]) + (4 * (vx_int16)srow1[x + 1]) + (vx_int16)srow1[x + 2])
				- ((vx_int16)srow0[x - 2] + (4 * (vx_int16)srow0[x - 1]) + (6 * (vx_int16)srow0[x]) + (4 * (vx_int16)srow0[x + 1]) + (vx_int16)srow0[x + 2]);
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}

int HafCpu_CannySobel_U16_U8_7x7_L2NORM
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint16   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	)
{
	int x, y;
	vx_uint32 dstride = dstImageStrideInBytes >> 1;
	pDstImage += 3 * dstride;		// don't care about border. start processing from row2
	pSrcImage += 3 * srcImageStrideInBytes;
#if ENABLE_MSA
	int prefixWidth = ((intptr_t)(pDstImage)) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	int postfixWidth = ((int)dstWidth - prefixWidth) & 15;
	int alignedWidth = (int)dstWidth - prefixWidth - postfixWidth;
	vx_int16 *r0 = (vx_int16*)(pLocalData + 16);
	vx_int16 *r1 = r0 + ((dstWidth + 15) & ~15);
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v8i16 c5 = __builtin_msa_ldi_h(5);
	v8i16 c6 = __builtin_msa_ldi_h(6);
	v8i16 c15 = __builtin_msa_ldi_h(15);
	v8i16 c20 = __builtin_msa_ldi_h(20);
	v8i16 clamp = __builtin_msa_fill_h((int) 0x3FFF);
	v8i16 s0, s1, s2, s3, s4, s5, s6, s7, s8;
#endif

	for (y = 3; y < (int)dstHeight - 3; y++)
	{
		const vx_uint8* srow0 = pSrcImage - 3 * srcImageStrideInBytes;
		const vx_uint8* srow1 = pSrcImage - 2 * srcImageStrideInBytes;
		const vx_uint8* srow2 = pSrcImage - srcImageStrideInBytes;
		const vx_uint8* srow3 = pSrcImage;
		const vx_uint8* srow4 = pSrcImage + srcImageStrideInBytes;
		const vx_uint8* srow5 = pSrcImage + 2 * srcImageStrideInBytes;
		const vx_uint8* srow6 = pSrcImage + 3 * srcImageStrideInBytes;

		vx_uint16* drow = (vx_uint16*)pDstImage;
#if ENABLE_MSA
		for (x = 0; x < prefixWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}

		// do vertical convolution - MSA
		for (x = prefixWidth; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((void *) (srow0 + x), 0);
			s1 = __builtin_msa_ld_h((void *) (srow1 + x), 0);
			s2 = __builtin_msa_ld_h((void *) (srow2 + x), 0);
			s3 = __builtin_msa_ld_h((void *) (srow3 + x), 0);
			s4 = __builtin_msa_ld_h((void *) (srow4 + x), 0);
			s5 = __builtin_msa_ld_h((void *) (srow5 + x), 0);
			s6 = __builtin_msa_ld_h((void *) (srow6 + x), 0);

			s0 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s0);
			s1 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s1);
			s2 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s2);
			s3 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s3);
			s4 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s4);
			s5 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s5);
			s6 = (v8i16) __builtin_msa_ilvr_b(zeromask, (v16i8) s6);

			v8i16 t0 =  __builtin_msa_addv_h(__builtin_msa_mulv_h(__builtin_msa_addv_h(s1, s5), c6), __builtin_msa_mulv_h(s3, c20));
			v8i16 t2 =  __builtin_msa_mulv_h(__builtin_msa_addv_h(s2, s4), c15);

			t0 = __builtin_msa_addv_h(t0, __builtin_msa_addv_h(s0, s6));
			v8i16 t1 = __builtin_msa_slli_h(__builtin_msa_subv_h(s5, s1), 2);
			t0 = __builtin_msa_addv_h(t0, t2);

			t2 = __builtin_msa_mulv_h(__builtin_msa_subv_h(s4, s2), c5);
			t0 = __builtin_msa_srai_h(t0, 2);
			t1 = __builtin_msa_addv_h(t1, __builtin_msa_subv_h(s6, s0));
			t1 = __builtin_msa_addv_h(t1, t2);
			t1 = __builtin_msa_srai_h(t1, 2);

			__builtin_msa_st_h(t0, (void *) (r0 + x), 0);
			__builtin_msa_st_h(t1, (void *) (r1 + x), 0);
		}

		// do horizontal convolution, interleave the results and store them to dst - MSA
		x = prefixWidth;
		for (; x <= alignedWidth - 8; x += 8)
		{
			s0 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 3), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 2), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r0 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 1), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 2), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r0 + x + 3), 0);

			v8i16 t0 = __builtin_msa_slli_h(__builtin_msa_subs_s_h(s4, s1), 2);
			v8i16 t1 = __builtin_msa_mulv_h(__builtin_msa_subs_s_h(s3, s2), c5);
			t0 = __builtin_msa_adds_s_h(t0, __builtin_msa_subs_s_h(s5, s0));
			t0 = __builtin_msa_adds_s_h(t0, t1);

			s0 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 3), 0);
			s1 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 2), 0);
			s2 = __builtin_msa_ld_h((const v16u8 *) (r1 + x - 1), 0);
			s3 = __builtin_msa_ld_h((const v16u8 *) (r1 + x), 0);
			s4 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 1), 0);
			s5 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 2), 0);
			s6 = __builtin_msa_ld_h((const v16u8 *) (r1 + x + 3), 0);

			t1 = __builtin_msa_adds_s_h(__builtin_msa_mulv_h(__builtin_msa_addv_h(s1, s5), c6), __builtin_msa_mulv_h(s3, c20));
			v8i16 t2 = __builtin_msa_mulv_h(__builtin_msa_addv_h(s2, s4), c15);
			t1 = __builtin_msa_adds_s_h(t1, __builtin_msa_adds_s_h(s0, s6));
			t1 = __builtin_msa_adds_s_h(t1, t2);
			t1 = __builtin_msa_subs_s_h((v8i16) zeromask, t1);

			s0 = __builtin_msa_ilvr_h(t1, t0);
			s2 = __builtin_msa_ilvl_h(t1, t0);

			s0 = (v8i16) __builtin_msa_dotp_s_w(s0, s0);
			s2 = (v8i16) __builtin_msa_dotp_s_w(s2, s2);

			v4f32 f0 = __builtin_msa_ffint_s_w((v4i32) s0);
			v4f32 f1 = __builtin_msa_ffint_s_w((v4i32) s2);

			f0 = __builtin_msa_fsqrt_w(f0);
			f1 = __builtin_msa_fsqrt_w(f1);

			s1 = HafCpu_FastAtan2_Canny_MSA(t0, t1);

			t0 = (v8i16) __builtin_msa_ftint_s_w(f0);
			t1 = (v8i16) __builtin_msa_ftint_s_w(f1);

			// pack with signed saturation
			v4u32 temp0_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t0, 15);
			v4u32 temp1_u = (v4u32)__builtin_msa_sat_u_h((v8u16) t1, 15);
			t0 = __builtin_msa_pckev_h((v8i16)temp1_u, (v8i16)temp0_u);

			// store magnitude and angle to destination
			t0 = (v8i16) __builtin_msa_or_v((v16u8) __builtin_msa_slli_h(t0, 2), (v16u8) s1);
			 __builtin_msa_st_h(t0, (void *) (drow + x), 0);
		}

		for (x = alignedWidth + prefixWidth; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#else
		for (x = 0; x < (int)dstWidth; x++)
		{
			vx_int16 Gx = (vx_int16)srow0[x + 3] + (4 * (vx_int16)srow0[x + 2]) + (5 * (vx_int16)srow0[x + 1]) - (5 * (vx_int16)srow0[x - 1]) - (4 * (vx_int16)srow0[x - 2]) - (vx_int16)srow0[x - 3]
				+ 6 * ((vx_int16)srow1[x + 3] + (4 * (vx_int16)srow1[x + 2]) + (5 * (vx_int16)srow1[x + 1]) - (5 * (vx_int16)srow1[x - 1]) - (4 * (vx_int16)srow1[x - 2]) - (vx_int16)srow1[x - 3])
				+ 15 * ((vx_int16)srow2[x + 3] + (4 * (vx_int16)srow2[x + 2]) + (5 * (vx_int16)srow2[x + 1]) - (5 * (vx_int16)srow2[x - 1]) - (4 * (vx_int16)srow2[x - 2]) - (vx_int16)srow2[x - 3])
				+ 20 * ((vx_int16)srow3[x + 3] + (4 * (vx_int16)srow3[x + 2]) + (5 * (vx_int16)srow3[x + 1]) - (5 * (vx_int16)srow3[x - 1]) - (4 * (vx_int16)srow3[x - 2]) - (vx_int16)srow3[x - 3])
				+ 15 * ((vx_int16)srow4[x + 3] + (4 * (vx_int16)srow4[x + 2]) + (5 * (vx_int16)srow4[x + 1]) - (5 * (vx_int16)srow4[x - 1]) - (4 * (vx_int16)srow4[x - 2]) - (vx_int16)srow4[x - 3])
				+ 6 * ((vx_int16)srow5[x + 3] + (4 * (vx_int16)srow5[x + 2]) + (5 * (vx_int16)srow5[x + 1]) - (5 * (vx_int16)srow5[x - 1]) - (4 * (vx_int16)srow5[x - 2]) - (vx_int16)srow5[x - 3])
				+ (vx_int16)srow6[x + 3] + (4 * (vx_int16)srow6[x + 2]) + (5 * (vx_int16)srow6[x + 1]) - (5 * (vx_int16)srow6[x - 1]) - (4 * (vx_int16)srow6[x - 2]) - (vx_int16)srow6[x - 3];
			vx_int16 Gy = (vx_int16)srow6[x - 3] + (vx_int16)srow6[x + 3] + (6 * ((vx_int16)srow6[x - 2] + (vx_int16)srow6[x + 2])) + (15 * ((vx_int16)srow6[x - 1] + (vx_int16)srow6[x + 1])) + (20 * (vx_int16)srow6[x])
				+ 4 * ((vx_int16)srow5[x - 3] + (vx_int16)srow5[x + 3] + (6 * ((vx_int16)srow5[x - 2] + (vx_int16)srow5[x + 2])) + (15 * ((vx_int16)srow5[x - 1] + (vx_int16)srow5[x + 1])) + (20 * (vx_int16)srow5[x]))
				+ 5 * ((vx_int16)srow4[x - 3] + (vx_int16)srow4[x + 3] + (6 * ((vx_int16)srow4[x - 2] + (vx_int16)srow4[x + 2])) + (15 * ((vx_int16)srow4[x - 1] + (vx_int16)srow4[x + 1])) + (20 * (vx_int16)srow4[x]))
				- 5 * ((vx_int16)srow2[x - 3] + (vx_int16)srow2[x + 3] + (6 * ((vx_int16)srow2[x - 2] + (vx_int16)srow2[x + 2])) + (15 * ((vx_int16)srow2[x - 1] + (vx_int16)srow2[x + 1])) + (20 * (vx_int16)srow2[x]))
				- 4 * ((vx_int16)srow1[x - 3] + (vx_int16)srow1[x + 3] + (6 * ((vx_int16)srow1[x - 2] + (vx_int16)srow1[x + 2])) + (15 * ((vx_int16)srow1[x - 1] + (vx_int16)srow1[x + 1])) + (20 * (vx_int16)srow1[x]))
				- ((vx_int16)srow0[x - 3] + (vx_int16)srow0[x + 3] + (6 * ((vx_int16)srow0[x - 2] + (vx_int16)srow0[x + 2])) + (15 * ((vx_int16)srow0[x - 1] + (vx_int16)srow0[x + 1])) + (20 * (vx_int16)srow0[x]));
			Gx = Gx / 4;
			Gy = Gy / 4;
			Gy = ~Gy + 1;
			vx_int16 tmp = (vx_int16)sqrt((Gx*Gx) + (Gy*Gy));
			tmp <<= 2;
			tmp |= (HafCpu_FastAtan2_Canny(Gx, Gy) & 3);
			drow[x] = tmp;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstride;
	}
	return AGO_SUCCESS;
}
