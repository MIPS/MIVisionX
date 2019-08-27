#include "ago_internal.h"
#include "mips_internal.h"

#define FP_BITS		18
#define FP_MUL		(1 << FP_BITS)
#define FP_ROUND    (1<<17)

int HafCpu_ScaleImage_U8_U8_Area
	(
		vx_uint32            dstWidth,
		vx_uint32            dstHeight,
		vx_uint8           * pDstImage,
		vx_uint32            dstImageStrideInBytes,
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes,
		ago_scale_matrix_t * matrix
	)
{
	if (matrix->xscale == 1.0f && matrix->yscale == 1.0f)
	{
		vx_uint8 *pSrcB = pSrcImage + (dstHeight - 1) * srcImageStrideInBytes;
		// no scaling. Just do a copy from src to dst
		for (unsigned int y = 0; y < dstHeight; y++)
		{
			vx_uint8 *pSrc = pSrcImage + (int) (matrix->yoffset + y) * srcImageStrideInBytes + (int) matrix->xoffset;
			// clamp to boundary
			if (pSrc < pSrcImage) pSrc = pSrcImage;
			if (pSrc > pSrcB) pSrc = pSrcB;
			memcpy(pDstImage, pSrc, dstWidth);
			pDstImage += dstImageStrideInBytes;
		}
	}
	else if (matrix->xscale == 2.0f && matrix->yscale == 2.0f)
	{
#if ENABLE_MSA
		v16i8 zero = __builtin_msa_ldi_b(0);
		v8i16 delta2 = __builtin_msa_ldi_h(2);
		v8i16 masklow = __builtin_msa_ldi_h(0x00ff);
#endif
		vx_uint8 *pSrcB = pSrcImage + (srcHeight - 2) * srcImageStrideInBytes;
		// 2x2 image scaling
		for (unsigned int y = 0; y < dstHeight; y++)
		{
			vx_uint8 *S0 = pSrcImage + (int) (matrix->yoffset + (y * 2)) * srcImageStrideInBytes + (int) (matrix->xoffset);
			if (S0 < pSrcImage) S0 = pSrcImage;
			if (S0 > pSrcB) S0 = pSrcB;
			vx_uint8 *S1 = S0 + srcImageStrideInBytes;
			vx_uint8 *D = pDstImage;

#if ENABLE_MSA
			for (unsigned int dx = 0; dx <= dstWidth - 8; dx += 8, S0 += 16, S1 += 16, D += 8)
			{
				v16i8 r0 = __builtin_msa_ld_b((void *) S0, 0);
				v16i8 r1 = __builtin_msa_ld_b((void *) S1, 0);

				v16i8 s0 = (v16i8) __builtin_msa_addv_h(__builtin_msa_srli_h((v8i16) r0, 8), (v8i16) __builtin_msa_and_v((v16u8) r0, (v16u8) masklow));
				v16i8 s1 = (v16i8) __builtin_msa_addv_h(__builtin_msa_srli_h((v8i16) r1, 8), (v8i16) __builtin_msa_and_v((v16u8) r1, (v16u8) masklow));

				s0 = (v16i8) __builtin_msa_addv_h(__builtin_msa_addv_h((v8i16) s0, (v8i16) s1), (v8i16) delta2);

				v8u16 temp0_u = (v8u16)__builtin_msa_sat_u_b((v16u8) __builtin_msa_srli_h((v8i16) s0, 2), 7);
				s0 = (v16i8) __builtin_msa_pckev_b(zero, (v16i8) temp0_u);

				*(long long*) D = ((v2i64) s0)[0];
			}
#else
			for (unsigned int dx = 0; dx <= dstWidth - 1; dx++, S0++, S1++, D++)
			{
				unsigned short r0 = ((unsigned short *) &S0[dx])[0];
				unsigned short r1 = ((unsigned short *) &S1[dx])[0];

				unsigned short s0 = (r0 >> 8) + (r0 & 255);
				unsigned short s1 = (r1 >> 8) + (r1 & 255);

				s0 = s0 + s1 + 2;

				unsigned char t = (unsigned char) max(min((s0 >> 2), UINT8_MAX), 0);

				*(char*) D = (char) t;
			}
#endif
			pDstImage += dstImageStrideInBytes;
		}
	}
	else
	{
		int xinc, yinc, xoffs, yoffs, xpos, ypos, x, y;

		// Intermideate buffers to store results between horizontally filtered rows
		int alignWidth = (dstWidth + 15) & ~15;
		vx_uint16 *Xmap = (unsigned short *) ((vx_uint8 *) matrix + sizeof(ago_scale_matrix_t));
		vx_uint16 *Ymap = Xmap + alignWidth + 8;
#if ENABLE_MSA
		v16i8 z = __builtin_msa_ldi_b(0);
#endif

		// do generic area scaling

		// to convert to fixed point
		yinc = (int) (FP_MUL * matrix->yscale);
		xinc = (int) (FP_MUL * matrix->xscale);

		// to convert to fixed point
		yoffs = (int) (FP_MUL * matrix->yoffset);
		xoffs = (int) (FP_MUL * matrix->xoffset);

		int xscale = (int) (matrix->xscale + 0.5);
		int yscale = (int) (matrix->yscale + 0.5);
		float inv_scale = 1.0f / (xscale * yscale);
		int area_div = (int) (FP_MUL * inv_scale);
		vx_uint8 *src_b = pSrcImage + srcWidth * (srcHeight - 1);
		//int area_sz = (area + (1 << (FP_BITS - 1))) >> FP_BITS;

		// generate xmap;
		for (x = 0, xpos = xoffs; x <= (int) dstWidth; x++, xpos += xinc)
		{
			int xmap;
			xmap = ((xpos + FP_ROUND) >> FP_BITS);
			if (xmap >(int) (srcWidth - 1))
			{
				xmap = (srcWidth - 1);
			}
			if (xmap < 0) xmap = 0;
			Xmap[x] = (unsigned short) xmap;
		}
		for (y = 0, ypos = yoffs; y < (int) dstHeight; y++, ypos += yinc)
		{
			int ymap;
			ymap = ((ypos + FP_ROUND )>> FP_BITS);
			if (ymap >(int) (srcHeight - 1)){
				ymap = srcHeight - 1;
			}
			if (ymap < 0) ymap = 0;

			// compute vertical sum and store in intermediate buffer
			vx_uint8 *S0 = pSrcImage + (int) ymap * srcImageStrideInBytes;
			vx_uint8 *D = pDstImage;

#if ENABLE_MSA
			for (x = Xmap[0]; x <= (Xmap[dstWidth] - 7); x += 8)
			{
				v16i8 r0 = __builtin_msa_ilvr_b(z, __builtin_msa_ld_b((void *) (S0 + x), 0));
				vx_uint8 *S1 = S0 + srcImageStrideInBytes;
				for (int i = 1; i < yscale; i++)
				{
					if (S1 > src_b)
						S1 = src_b;
					v16i8 r1 = __builtin_msa_ilvr_b(z, __builtin_msa_ld_b((void *) (S1 + x), 0));
					r0 = (v16i8) __builtin_msa_addv_h((v8i16) r0, (v8i16) r1);
					S1 += srcImageStrideInBytes;
				}
				__builtin_msa_st_b(r0, (void *) &Ymap[x], 0);
			}
			// do horizontal scaling on intermediate buffer
			for (x = 0; x < (int) dstWidth; x++)
			{
				int x0 = Xmap[x];
				int x1 = x0 + xscale;
				int sum = Ymap[x0];
				while(++x0 < x1)
				{
					sum += Ymap[x0];
				};
				// divide sum by area and copy to dest
				*D++ = (vx_uint8) (((sum * area_div) + (1 << 15)) >> FP_BITS);

			}
#else
			for (x = Xmap[0]; x <= Xmap[dstWidth]; x++)
			{
				int r0 = S0[x];
				vx_uint8 *S1 = S0 + srcImageStrideInBytes;
				for (int i = 1; i < yscale; i++){
					if (S1 > src_b)
						S1 = src_b;
					int r1 = S1[x];
					r0 += r1;
					S1 += srcImageStrideInBytes;
				}
				Ymap[x] = r0;
			}

			// do horizontal scaling on intermediate buffer
			for (x = 0; x < (int) dstWidth; x++)
			{
				int x0 = Xmap[x];
				int x1 = x0 + xscale;
				int sum = Ymap[x0];
				while(++x0 < x1)
				{
					sum += Ymap[x0];
				};
				// divide sum by area and copy to dest
				*D++ = (vx_uint8) (((sum * area_div) + (1 << 15)) >> FP_BITS);

			}
#endif
			pDstImage += dstImageStrideInBytes;
		}
	}
	return AGO_SUCCESS;
}

int HafCpu_ScaleImage_U8_U8_Bilinear
	(
		vx_uint32	     dstWidth,
		vx_uint32	     dstHeight,
		vx_uint8	   * pDstImage,
		vx_uint32	     dstImageStrideInBytes,
		vx_uint32	     srcWidth,
		vx_uint32	     srcHeight,
		vx_uint8	   * pSrcImage,
		vx_uint32	     srcImageStrideInBytes,
		ago_scale_matrix_t * matrix
	)
{
	int xinc, yinc, xoffs, yoffs;
	unsigned char *pdst = pDstImage;

#if ENABLE_MSA
	v8i16 pp1, pp2;
	v8i16 mask = __builtin_msa_ldi_h((short) 0xff);
	v8i16 round = __builtin_msa_ldi_h((short) 0x80);

	// nearest multiple of 8
	unsigned int newDstWidth = dstWidth & ~7;
#endif

	// to convert to fixed point
	yinc = (int) (FP_MUL * matrix->yscale);
	xinc = (int) (FP_MUL * matrix->xscale);

	// to convert to fixed point
	yoffs = (int) (FP_MUL * matrix->yoffset);
	xoffs = (int) (FP_MUL * matrix->xoffset);

	int alignW = (dstWidth + 15) & ~15;
	unsigned short *Xmap = (unsigned short *) ((vx_uint8 *) matrix + sizeof(AgoConfigScaleMatrix));
	unsigned short *Xfrac = Xmap + alignW;
	unsigned short *One_min_xf = Xfrac + alignW;

	int xpos = xoffs;
	for (unsigned int x = 0; x < dstWidth; x++, xpos += xinc)
	{
		int xf;
		int xmap = (xpos >> FP_BITS);
		if (xmap >= (int) (srcWidth - 1)){
			Xmap[x] = (unsigned short) (srcWidth - 1);
		}
		Xmap[x] = (xmap < 0) ? 0: (unsigned short) xmap;
		xf = ((xpos & 0x3ffff) + 0x200) >> 10;
		Xfrac[x] = xf;
		One_min_xf[x] = (0x100 - xf);
	}

	for (int y = 0, ypos = yoffs; y < (int) dstHeight; y++, ypos += yinc)
	{
		unsigned int x = 0;
		vx_uint8 *pSrc1, *pSrc2;
		int ym, yf, one_min_yf;

		ym = (ypos >> FP_BITS);
		yf = ((ypos & 0x3ffff) + 0x200) >> 10;
		one_min_yf = (0x100 - yf);
		yoffs = ym * srcImageStrideInBytes;
		if (ym < 0){
			ym = yoffs = 0;
			pSrc1 = pSrc2 = pSrcImage;
		}
		else if (ym >= (int) (srcHeight - 1)){
			ym = srcHeight - 1;
			pSrc1 = pSrc2 = pSrcImage + ym * srcImageStrideInBytes;
		}
		else
		{
			pSrc1 = pSrcImage + ym * srcImageStrideInBytes;
			pSrc2 = pSrc1 + srcImageStrideInBytes;
		}

#if ENABLE_MSA
		v8i16 rmsa0, rmsa7;

		rmsa0 = __builtin_msa_fill_h((unsigned short) one_min_yf);
		rmsa7 = __builtin_msa_fill_h((unsigned short) yf);

		for (; x < newDstWidth; x += 8)
		{
			v8i16 mapxy, rmsa1, rmsa2, rmsa3, rmsa4;

			// mapped table [srcx7...src_x3, src_x2, src_x1, src_x0]
			mapxy = __builtin_msa_ld_h((void *) &Xmap[x], 0);

/*
	On ISAs older then r6 accessing 2B (unsigned short) will cause unaligned access.
	For this function unaligned access was heavily affecting the performance,
	so workaround was developed to use read 1B at the time and then combine it to short.
*/
#if __mips_isa_rev < 6
			v16u8 pp1r = (v16u8) pp1;
			v16u8 pp2r = (v16u8) pp2;
#endif
			// load pixels for mapxy
			for (int xx = 0; xx < 8; xx++)
			{
#if __mips_isa_rev < 6
				unsigned char temp1 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				unsigned char temp2 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[1];
				pp1r[2 * xx] = temp1;
				pp1r[2 * xx + 1] = temp2;

				temp1 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
				temp2 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[1];
				pp2r[2 * xx] = temp1;
				pp2r[2 * xx + 1] = temp2;
#else
				pp1[xx] = ((unsigned short *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				pp2[xx] = ((unsigned short *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
#endif
			}
#if __mips_isa_rev < 6
			pp1 = (v8i16) pp1r;
			pp2 = (v8i16) pp2r;
#endif
			rmsa1 = (v8i16) __builtin_msa_and_v((v16u8) pp1, (v16u8) mask);
			pp1 = __builtin_msa_srli_h(pp1, 8);

			rmsa4 = (v8i16) __builtin_msa_and_v((v16u8) pp2, (v16u8) mask);
			pp2 = __builtin_msa_srli_h(pp2, 8);

			rmsa2 = __builtin_msa_ld_h((void *) &Xfrac[x], 0);
			rmsa3 = __builtin_msa_ld_h((void *) &One_min_xf[x], 0);

			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa3);
			pp1 = __builtin_msa_mulv_h(pp1, rmsa2);
			rmsa1 = __builtin_msa_addv_h(rmsa1, pp1);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);
			rmsa1 = __builtin_msa_srli_h(rmsa1, 8);

			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa3);
			pp2 = __builtin_msa_mulv_h(pp2, rmsa2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, pp2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, round);
			rmsa4 = __builtin_msa_srli_h(rmsa4, 8);

			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa0);
			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa7);
			rmsa1 = __builtin_msa_addv_h(rmsa1, rmsa4);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);
			rmsa1 = __builtin_msa_srli_h(rmsa1, 8);
			v16i8 temp = (v16i8)__builtin_msa_sat_u_b((v16u8) rmsa1, 7);
			rmsa1 = (v8i16) __builtin_msa_pckev_b(temp, temp);

			*(long long*) (pDstImage + x) = ((v2i64) rmsa1)[0];
		}

		for (x = newDstWidth; x < dstWidth; x++) {
			const unsigned char *p0 = pSrc1 + Xmap[x];
			const unsigned char *p1 = pSrc2 + Xmap[x];
			pDstImage[x] = ((One_min_xf[x] * one_min_yf*p0[0]) + (Xfrac[x] * one_min_yf*p0[1]) + (One_min_xf[x] * yf*p1[0]) + (Xfrac[x] * yf*p1[1]) + 0x8000) >> 16;
		}
#else
		for (x = 0; x < dstWidth; x++) {
			const unsigned char *p0 = pSrc1 + Xmap[x];
			const unsigned char *p1 = pSrc2 + Xmap[x];
			pDstImage[x] = ((One_min_xf[x] * one_min_yf*p0[0]) + (Xfrac[x] * one_min_yf*p0[1]) + (One_min_xf[x] * yf*p1[0]) + (Xfrac[x] * yf*p1[1]) + 0x8000) >> 16;
		}
#endif
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ScaleImage_U8_U8_Bilinear_Constant
	(
		vx_uint32            dstWidth,
		vx_uint32            dstHeight,
		vx_uint8           * pDstImage,
		vx_uint32            dstImageStrideInBytes,
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes,
		ago_scale_matrix_t * matrix,
		vx_uint8             border
	)
{
	int xinc, yinc, xoffs, yoffs;
	unsigned char *pdst = pDstImage;

#if ENABLE_MSA
	v8i16 pp1, pp2;
	v8i16 mask = __builtin_msa_ldi_h((short) 0xff);
	v8i16 round = __builtin_msa_ldi_h((short) 0x80);
#endif

	unsigned int sline = srcImageStrideInBytes;
	unsigned char *pSrcLast = pSrcImage + (srcImageStrideInBytes * (srcWidth - 1));

	// to convert to fixed point
	yinc = (int) (FP_MUL * matrix->yscale);
	xinc = (int) (FP_MUL * matrix->xscale);

	// to convert to fixed point
	yoffs = (int) (FP_MUL * matrix->yoffset);
	xoffs = (int) (FP_MUL * matrix->xoffset);

	int alignW = (dstWidth + 15) & ~15;
	unsigned short *Xmap = (unsigned short *) ((vx_uint8 *) matrix + sizeof(AgoConfigScaleMatrix));
	unsigned short *Xfrac = Xmap + alignW;
	unsigned short *One_min_xf = Xfrac + alignW;

	vx_uint8 *pSrcBorder = (vx_uint8 *) (One_min_xf + alignW);
	memset(pSrcBorder, border, srcWidth);

	int xpos = xoffs;
	vx_uint32 newDstWidth = 0;
	for (unsigned int x = 0; x < dstWidth; x++, xpos += xinc)
	{
		int xf;
		int xmap = (xpos >> FP_BITS);
		if (xmap >= (int) (srcWidth - 1))
		{
			if (!newDstWidth)
				newDstWidth = x - 1;
			Xmap[x] = (unsigned short) (srcWidth - 1);
		}
		else
		{
			Xmap[x] = (xmap < 0) ? 0 : (unsigned short) xmap;
		}
		xf = ((xpos & 0x3ffff) + 0x200) >> 10;
		Xfrac[x] = xf;
		One_min_xf[x] = (0x100 - xf);
	}
	if (dstWidth & 7)
	{
		// nearest multiple of 8
		newDstWidth &= ~7;
	}
	for (int y = 0, ypos = yoffs; y < (int) dstHeight; y++, ypos += yinc)
	{
		unsigned int x = 0;
		int ym, yf, one_min_yf;
		unsigned int yoffs;
		vx_uint8 *pSrc1, *pSrc2;

		ym = (ypos >> FP_BITS);
		yf = ((ypos & 0x3ffff) + 0x200) >> 10;
		one_min_yf = (0x100 - yf);
		if (ym < 0)
		{
			ym = yoffs = 0;
			pSrc1 = pSrcBorder;
			pSrc2 = pSrcImage;
		}
		else if (ym >= (int) (srcHeight - 1))
		{
			ym = srcHeight - 1;
			pSrc1 = pSrcImage + ym * srcImageStrideInBytes;
			pSrc2 = pSrcBorder;
			yoffs = ym * srcImageStrideInBytes;
		}
		else
		{
			pSrc1 = pSrcImage + ym * srcImageStrideInBytes;
			pSrc2 = pSrc1 + srcImageStrideInBytes;
			yoffs = ym * srcImageStrideInBytes;
		}
#if ENABLE_MSA
		v8i16 rmsa0, rmsa7;
		rmsa0 = __builtin_msa_fill_h((unsigned short) one_min_yf);
		rmsa7 = __builtin_msa_fill_h((unsigned short) yf);

		for (; x < newDstWidth; x += 8)
		{
			v8i16 mapxy, rmsa1, rmsa2, rmsa3, rmsa4;
			// mapped table [srcx7...src_x3,src_x2,src_x1,src_x0]
			mapxy = __builtin_msa_ld_h((void *) &Xmap[x], 0);

/*
	On ISAs older then r6 accessing 2B (unsigned short) will cause unaligned access.
	For this function unaligned access was heavily affecting the performance,
	so workaround was developed to use read 1B at the time and then combine it to short.
*/
#if __mips_isa_rev < 6
			v16u8 pp1r = (v16u8) pp1;
			v16u8 pp2r = (v16u8) pp2;
#endif
			// load pixels for mapxy
			for (int xx = 0; xx < 8; xx++)
			{
#if __mips_isa_rev < 6
				unsigned char temp1 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				unsigned char temp2 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[1];
				pp1r[2 * xx] = temp1;
				pp1r[2 * xx + 1] = temp2;

				temp1 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
				temp2 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[1];
				pp2r[2 * xx] = temp1;
				pp2r[2 * xx + 1] = temp2;
#else
				pp1[xx] = ((unsigned short *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				pp2[xx] = ((unsigned short *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
#endif
			}
#if __mips_isa_rev < 6
			pp1 = (v8i16) pp1r;
			pp2 = (v8i16) pp2r;
#endif
			// unpack src for p1 and p2
			rmsa1 = (v8i16) __builtin_msa_and_v((v16u8) pp1, (v16u8) mask);
			pp1 = __builtin_msa_srli_h(pp1, 8);

			// unpack pp2 for p3 and p4
			rmsa4 = (v8i16) __builtin_msa_and_v((v16u8) pp2, (v16u8) mask);
			pp2 = __builtin_msa_srli_h(pp2, 8);

			// load xf and 1-xf
			rmsa2 = __builtin_msa_ld_h((void *) &Xfrac[x], 0);
			rmsa3 = __builtin_msa_ld_h((void *) &One_min_xf[x], 0);

			// t1 = (unsigned char) ((ione_minus_x *p1 + ifraction_x *p2) >> FW_WEIGHT);
			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa3);
			pp1 = __builtin_msa_mulv_h(pp1, rmsa2);
			rmsa1 = __builtin_msa_addv_h(rmsa1, pp1);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);
			rmsa1 = __builtin_msa_srli_h(rmsa1, 8);

			// t2 = (unsigned char) ((ione_minus_x *p3 + ifraction_x *p4) >> FW_WEIGHT);
			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa3);
			pp2 = __builtin_msa_mulv_h(pp2, rmsa2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, pp2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, round);
			rmsa4 = __builtin_msa_srli_h(rmsa4, 8);

			// *(pDst + x + y*dstStep) = (unsigned char) ((ione_minus_y *t1 + ifraction_y * t2) >> FW_WEIGHT)
			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa0);
			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa7);
			rmsa1 = __builtin_msa_addv_h(rmsa1, rmsa4);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);

			rmsa1 = (v8i16) __builtin_msa_pckod_b((v16i8) rmsa1, (v16i8) rmsa1);

			*(long long*) (pDstImage + x) = ((v2i64) rmsa1)[0];
		}
		// todo: if (upscale; recompute x=0, x=dwidth-1)
		if (matrix->xscale < 1)
		{
			unsigned int p0, p1, p2, p3;
			p0 = border;
			p1 = (ypos >> 8) < 0 ? border : pSrc1[0];
			p2 = border;
			p3 = pSrc2[0];
			pDstImage[0] = ((One_min_xf[0] * one_min_yf * p0) + (Xfrac[0] * one_min_yf * p1) + (One_min_xf[0] * yf * p2) + (Xfrac[0] * yf * p3) + 0x8000) >> 16;
		}
		x = newDstWidth;
		while (x < dstWidth)
		{
			unsigned int p0, p1, p2, p3;
			const unsigned char *p = pSrc1 + Xmap[x];
			p0 = p[0];
			p1 = (Xmap[x] < (srcWidth - 1)) ? p[1] : border;
			p = pSrc2 + Xmap[x];
			p2 = p[0];
			p3 = (Xmap[x] < (srcWidth - 1)) ? p[1] : border;
			pDstImage[x] = ((One_min_xf[x] * one_min_yf * p0) + (Xfrac[x] * one_min_yf * p1) + (One_min_xf[x] * yf * p2) + (Xfrac[x] * yf * p3) + 0x8000) >> 16;
			x++;
		}
#else // C
		while (x < dstWidth)
		{
			unsigned int p0, p1, p2, p3;
			const unsigned char *p = pSrc1 + Xmap[x];
			p0 = p[0];
			p1 = (Xmap[x] < (srcWidth - 1)) ? p[1] : border;
			p = pSrc2 + Xmap[x];
			p2 = p[0];
			p3 = (Xmap[x] < (srcWidth - 1)) ? p[1] : border;
			pDstImage[x] = ((One_min_xf[x] * one_min_yf * p0) + (Xfrac[x] * one_min_yf * p1) + (One_min_xf[x] * yf * p2) + (Xfrac[x] * yf * p3) + 0x8000) >> 16;
			x++;
		}
		// todo: if (upscale; recompute x=0, x=dwidth-1)
		if (matrix->xscale < 1)
		{
			unsigned int p0, p1, p2, p3;
			p0 = border;
			p1 = (ypos >> 8) < 0 ? border : pSrc1[0];
			p2 = border;
			p3 = pSrc2[0];
			pDstImage[0] = ((One_min_xf[0] * one_min_yf * p0) + (Xfrac[0] * one_min_yf * p1) + (One_min_xf[0] * yf * p2) + (Xfrac[0] * yf * p3) + 0x8000) >> 16;
		}
#endif
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ScaleImage_U8_U8_Bilinear_Replicate
	(
		vx_uint32            dstWidth,
		vx_uint32            dstHeight,
		vx_uint8           * pDstImage,
		vx_uint32            dstImageStrideInBytes,
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes,
		ago_scale_matrix_t * matrix
	)
{
	int xinc, yinc, xoffs, yoffs;
	unsigned char *pdst = pDstImage;

	// to convert to fixed point
	yinc = (int) (FP_MUL * matrix->yscale);
	xinc = (int) (FP_MUL * matrix->xscale);

	// to convert to fixed point
	yoffs = (int) (FP_MUL * matrix->yoffset);
	xoffs = (int) (FP_MUL * matrix->xoffset);

	int alignW = (dstWidth + 15) & ~15;

	unsigned short *Xmap = (unsigned short *) ((vx_uint8 *) matrix + sizeof(AgoConfigScaleMatrix));
	unsigned short *Xfrac = Xmap + alignW;
	unsigned short *One_min_xf = Xfrac + alignW;

	int xpos = xoffs;
	vx_uint32 newDstWidth = 0;

	for (unsigned int x = 0; x < dstWidth; x++, xpos += xinc)
	{
		int xf;
		int xmap = (xpos >> FP_BITS);
		if (xmap >= (int) (srcWidth - 1)){
			if (!newDstWidth)
				newDstWidth = x - 1;
			Xmap[x] = (unsigned short) (srcWidth - 1);
		}
		else {
			Xmap[x] = (xmap < 0) ? 0 : (unsigned short) xmap;
		}
		xf = ((xpos & 0x3ffff) + 0x200) >> 10;
		Xfrac[x] = xf;
		One_min_xf[x] = (0x100 - xf);
	}
	if (dstWidth & 7)
	{
		// nearest multiple of 8
		newDstWidth &= ~7;
	}

#if ENABLE_MSA
	v8i16 pp1, pp2;
	v8i16 mask = __builtin_msa_ldi_h((short) 0xff);
	v8i16 round = __builtin_msa_ldi_h((short) 0x80);
#endif
	for (int y = 0, ypos = yoffs; y < (int) dstHeight; y++, ypos += yinc)
	{
		unsigned int x = 0;
		int ym, yf, one_min_yf;
		unsigned int yoffs;
		vx_uint8 *pSrc1, *pSrc2;

		ym = (ypos >> FP_BITS);
		yf = ((ypos & 0x3ffff) + 0x200) >> 10;
		one_min_yf = (0x100 - yf);
		yoffs = ym * srcImageStrideInBytes;
		if (ym < 0)
		{
			ym = yoffs = 0;
			pSrc1 = pSrc2 = pSrcImage;
		}
		else if (ym >= (int) (srcHeight - 1))
		{
			ym = srcHeight - 1;
			pSrc1 = pSrc2 = pSrcImage + ym * srcImageStrideInBytes;
		}
		else
		{
			pSrc1 = pSrcImage + ym * srcImageStrideInBytes;
			pSrc2 = pSrc1 + srcImageStrideInBytes;
		}
#if ENABLE_MSA
		v8i16 rmsa0, rmsa7;
		rmsa0 = __builtin_msa_fill_h((unsigned short) one_min_yf);
		rmsa7 = __builtin_msa_fill_h((unsigned short) yf);

		for (; x < newDstWidth; x += 8)
		{
			v8i16 mapxy, rmsa1, rmsa2, rmsa3, rmsa4;

			// mapped table [srcx7...src_x3,src_x2,src_x1,src_x0]
			mapxy = __builtin_msa_ld_h((void *) &Xmap[x], 0);

/*
	On ISAs older then r6 accessing 2B (unsigned short) will cause unaligned access.
	For this function unaligned access was heavily affecting the performance,
	so workaround was developed to use read 1B at the time and then combine it to short.
*/
#if __mips_isa_rev < 6
			v16u8 pp1r = (v16u8) pp1;
			v16u8 pp2r = (v16u8) pp2;
#endif
			// load pixels for mapxy
			for (int xx = 0; xx < 8; xx++)
			{
#if __mips_isa_rev < 6
				unsigned char temp1 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				unsigned char temp2 = ((unsigned char *) &pSrc1[((int16_t *) &mapxy)[xx]])[1];
				pp1r[2 * xx] = temp1;
				pp1r[2 * xx + 1] = temp2;

				temp1 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
				temp2 = ((unsigned char *) &pSrc2[((int16_t *) &mapxy)[xx]])[1];
				pp2r[2 * xx] = temp1;
				pp2r[2 * xx + 1] = temp2;
#else
				pp1[xx] = ((unsigned short *) &pSrc1[((int16_t *) &mapxy)[xx]])[0];
				pp2[xx] = ((unsigned short *) &pSrc2[((int16_t *) &mapxy)[xx]])[0];
#endif
			}
#if __mips_isa_rev < 6
			pp1 = (v8i16) pp1r;
			pp2 = (v8i16) pp2r;
#endif
			// unpack src for p1 and p2
			rmsa1 = (v8i16) __builtin_msa_and_v((v16u8) pp1, (v16u8) mask);
			pp1 = __builtin_msa_srli_h(pp1, 8);

			// unpack pp2 for p3 and p4
			rmsa4 = (v8i16) __builtin_msa_and_v((v16u8) pp2, (v16u8) mask);
			pp2 = __builtin_msa_srli_h(pp2, 8);

			// load xf and 1-xf
			rmsa2 = __builtin_msa_ld_h((void *) &Xfrac[x], 0);
			rmsa3 = __builtin_msa_ld_h((void *) &One_min_xf[x], 0);

			// t1 = (unsigned char) ((ione_minus_x *p1 + ifraction_x *p2) >> FW_WEIGHT);
			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa3);
			pp1 = __builtin_msa_mulv_h(pp1, rmsa2);
			rmsa1 = __builtin_msa_addv_h(rmsa1, pp1);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);
			rmsa1 = __builtin_msa_srli_h(rmsa1, 8);

			// t2 = (unsigned char) ((ione_minus_x *p3 + ifraction_x *p4) >> FW_WEIGHT);
			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa3);
			pp2 = __builtin_msa_mulv_h(pp2, rmsa2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, pp2);
			rmsa4 = __builtin_msa_addv_h(rmsa4, round);
			rmsa4 = __builtin_msa_srli_h(rmsa4, 8);

			// *(pDst + x + y*dstStep) = (unsigned char) ((ione_minus_y *t1 + ifraction_y * t2) >> FW_WEIGHT)
			rmsa1 = __builtin_msa_mulv_h(rmsa1, rmsa0);
			rmsa4 = __builtin_msa_mulv_h(rmsa4, rmsa7);
			rmsa1 = __builtin_msa_addv_h(rmsa1, rmsa4);
			rmsa1 = __builtin_msa_addv_h(rmsa1, round);

			rmsa1 = (v8i16) __builtin_msa_pckod_b((v16i8) rmsa1, (v16i8) rmsa1);

			*(long long*) (pDstImage + x) = ((v2i64) rmsa1)[0];
		}
		// todo: if (upscale; recompute x=0, x=dwidth-1)
		if (matrix->xscale < 1){
			unsigned int p0, p1, p2, p3;
			p0 = p1 = pSrc1[0];
			p2 = p3 = pSrc2[0];
			pDstImage[0] = ((One_min_xf[0] * one_min_yf * p0) +
							(Xfrac[0] * one_min_yf * p1) +
							(One_min_xf[0] * yf * p2) +
							(Xfrac[0] * yf * p3) + 0x8000) >> 16;
		}
		x = newDstWidth;
		while (x < dstWidth){
			unsigned int p0, p1, p2, p3;
			const unsigned char *p = pSrc1 + Xmap[x];
			p0 = p[0];
			p1 = (Xmap[x] < (srcWidth - 1)) ? p[1] : p0;
			p = pSrc2 + Xmap[x];
			p2 = p[0];
			p3 = (Xmap[x] < (srcWidth - 1)) ? p[1]: p2;
			pDstImage[x] = ((One_min_xf[x] * one_min_yf * p0) +
							(Xfrac[x] * one_min_yf * p1) +
							(One_min_xf[x] * yf * p2) +
							(Xfrac[x] * yf * p3) + 0x8000) >> 16;
			x++;
		}
#else
		while (x < dstWidth){
			unsigned int p0, p1, p2, p3;
			const unsigned char *p = pSrc1 + Xmap[x];
			p0 = p[0];
			p1 = (Xmap[x] < (srcWidth - 1)) ? p[1] : p0;
			p = pSrc2 + Xmap[x];
			p2 = p[0];
			p3 = (Xmap[x] < (srcWidth - 1)) ? p[1]: p2;
			pDstImage[x] = ((One_min_xf[x] * one_min_yf * p0) +
							(Xfrac[x] * one_min_yf * p1) +
							(One_min_xf[x] * yf * p2) +
							(Xfrac[x] * yf * p3) + 0x8000) >> 16;
			x++;
		}
		// todo: if (upscale; recompute x=0, x=dwidth-1)
		if (matrix->xscale < 1){
			unsigned int p0, p1, p2, p3;
			p0 = p1 = pSrc1[0];
			p2 = p3 = pSrc2[0];
			pDstImage[0] = ((One_min_xf[0] * one_min_yf * p0) +
							(Xfrac[0] * one_min_yf * p1) +
							(One_min_xf[0] * yf * p2) +
							(Xfrac[0] * yf * p3) + 0x8000) >> 16;
		}
#endif
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ScaleImage_U8_U8_Nearest
	(
		vx_uint32	     dstWidth,
		vx_uint32	     dstHeight,
		vx_uint8	   * pDstImage,
		vx_uint32	     dstImageStrideInBytes,
		vx_uint32	     srcWidth,
		vx_uint32	     srcHeight,
		vx_uint8	   * pSrcImage,
		vx_uint32	     srcImageStrideInBytes,
		ago_scale_matrix_t * matrix
	)
{
	int xinc, yinc, ypos, xpos, yoffs, xoffs;// , newDstHeight, newDstWidth;

	// precompute Xmap and Ymap based on scale factors
	unsigned short *Xmap = (unsigned short *) ((vx_uint8 *) matrix + sizeof(AgoConfigScaleMatrix));
	unsigned short *Ymap = Xmap + ((dstWidth + 15) & ~15);
	unsigned int x, y;

	yinc = (int) (FP_MUL * matrix->yscale);		// to convert to fixed point
	xinc = (int) (FP_MUL * matrix->xscale);
	yoffs = (int) (FP_MUL * matrix->yoffset);	// to convert to fixed point
	xoffs = (int) (FP_MUL * matrix->xoffset);

	// generate ymap;
	for (y = 0, ypos = yoffs; y < (int) dstHeight; y++, ypos += yinc)
	{
		int ymap;
		ymap = (ypos >> FP_BITS);
		if (ymap > (int) (srcHeight - 1)){
			ymap = srcHeight - 1;
		}
		if (ymap < 0) ymap = 0;
		Ymap[y] = (unsigned short) ymap;
	}

	// generate xmap;
	for (x = 0, xpos = xoffs; x < (int) dstWidth; x++, xpos += xinc)
	{
		int xmap;
		xmap = (xpos >> FP_BITS);
		if (xmap > (int) (srcWidth - 1)){
			xmap = (srcWidth - 1);
		}
		if (xmap < 0) xmap = 0;
		Xmap[x] = (unsigned short) xmap;
	}

	// now do the scaling
#if ENABLE_MSA
	if (dstWidth >= 16)
	{
		v8i16 zeromask = __builtin_msa_ldi_h(0);
		v8i16 signmask;

		for (y = 0; y < dstHeight; y++)
		{
			unsigned int yadd = Ymap[y] * srcImageStrideInBytes;
			v4i32 syint = __builtin_msa_fill_w(yadd);
			unsigned int *pdst = (unsigned int *) pDstImage;
			for (x = 0; x <= (dstWidth - 16); x += 16)
			{
				v16u8 mapx0, mapx1, mapx2, mapx3;
				mapx0 = (v16u8) __builtin_msa_ld_b((void *) &Xmap[x], 0);
				mapx1 = (v16u8) __builtin_msa_ld_b((void *) &Xmap[x + 8], 0);
				mapx2 = (v16u8) __builtin_msa_ilvl_h(zeromask, (v8i16) mapx0);

				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) mapx0, 0);
				mapx0 = (v16u8) __builtin_msa_ilvr_h(signmask, (v8i16) mapx0);

				mapx3 = (v16u8) __builtin_msa_ilvl_h(zeromask, (v8i16) mapx1);
				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) mapx1, 0);
				mapx1 = (v16u8) __builtin_msa_ilvr_h(signmask, (v8i16) mapx1);

				mapx0 = (v16u8) __builtin_msa_addv_w((v4i32) mapx0, syint);
				mapx2 = (v16u8) __builtin_msa_addv_w((v4i32) mapx2, syint);
				mapx1 = (v16u8) __builtin_msa_addv_w((v4i32) mapx1, syint);
				mapx3 = (v16u8) __builtin_msa_addv_w((v4i32) mapx3, syint);

				// copy to dst
				*pdst++ = pSrcImage[((int32_t *) &mapx0)[0]] | (pSrcImage[((int32_t *) &mapx0)[1]] << 8) |
					(pSrcImage[((int32_t *) &mapx0)[2]] << 16) | (pSrcImage[((int32_t *) &mapx0)[3]] << 24);

				*pdst++ = pSrcImage[((int32_t *) &mapx2)[0]] | (pSrcImage[((int32_t *) &mapx2)[1]] << 8) |
					(pSrcImage[((int32_t *) &mapx2)[2]] << 16) | (pSrcImage[((int32_t *) &mapx2)[3]] << 24);

				*pdst++ = pSrcImage[((int32_t *) &mapx1)[0]] | (pSrcImage[((int32_t *) &mapx1)[1]] << 8) |
					(pSrcImage[((int32_t *) &mapx1)[2]] << 16) | (pSrcImage[((int32_t *) &mapx1)[3]] << 24);

				*pdst++ = pSrcImage[((int32_t *) &mapx3)[0]] | (pSrcImage[((int32_t *) &mapx3)[1]] << 8) |
					(pSrcImage[((int32_t *) &mapx3)[2]] << 16) | (pSrcImage[((int32_t *) &mapx3)[3]] << 24);

			}
			for (; x < dstWidth; x++)
				pDstImage[x] = pSrcImage[Xmap[x] + yadd];

			pDstImage += dstImageStrideInBytes;
		}
	}
	else
	{
		for (y = 0; y < dstHeight; y++)
		{
			unsigned int yadd = Ymap[y] * srcImageStrideInBytes;
			x = 0;
			for (; x < dstWidth; x++)
				pDstImage[x] = pSrcImage[Xmap[x] + yadd];
			pDstImage += dstImageStrideInBytes;
		}
	}
#else
	for (y = 0; y < dstHeight; y++)
	{
		unsigned int yadd = Ymap[y] * srcImageStrideInBytes;
		x = 0;
		for (; x < dstWidth; x++)
			pDstImage[x] = pSrcImage[Xmap[x] + yadd];
		pDstImage += dstImageStrideInBytes;
	}
#endif
	return AGO_SUCCESS;
}

// upsample 2x2 (used for 4:2:0 to 4:4:4 conversion)
int HafCpu_ScaleUp2x2_U8_U8
	(
		vx_uint32	     dstWidth,
		vx_uint32	     dstHeight,
		vx_uint8	   * pDstImage,
		vx_uint32	     dstImageStrideInBytes,
		vx_uint8	   * pSrcImage,
		vx_uint32	     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	v16i8 pixels1, pixels2;
#endif
	unsigned char *pchDst = (unsigned char*) pDstImage;
	unsigned char *pchDstlast = (unsigned char*) pDstImage + dstHeight * dstImageStrideInBytes;

	while (pchDst < pchDstlast)
	{
#if ENABLE_MSA
		v16i8 *src = (v16i8*) pSrcImage;
		v16i8 *dst = (v16i8*) pchDst;
		v16i8 *dstNext = (v16i8*) (pchDst + dstImageStrideInBytes);
		v16i8 *dstlast = dst + (dstWidth >> 4);

		bool fallbackToC = false;

		// Fallback to C if image width is not divisible by 16.
		// Or if image width is smaller then 16px.
		if ((dstWidth & 15) > 0)
			fallbackToC = true;

		// Fallback to C if image width requires odd number of vectors to process it.
		// Do not process the last, odd, vector with MSA.
		if (((dstWidth >> 4) % 2) == 1)
		{
			dstlast--;
			fallbackToC = true;
		}

		while (dst < dstlast)
		{
			// src (0-15)
			pixels1 = __builtin_msa_ld_b((void *) src++, 0);
			// dst (0-15)
			pixels2 = __builtin_msa_ilvr_b(pixels1, pixels1);
			// dst (16-31)
			pixels1 = __builtin_msa_ilvl_b(pixels1, pixels1);

			__builtin_msa_st_b(pixels2, (void *) dst++, 0);
			__builtin_msa_st_b(pixels1, (void *) dst++, 0);
			__builtin_msa_st_b(pixels2, (void *) dstNext++, 0);
			__builtin_msa_st_b(pixels1, (void *) dstNext++, 0);
		}

		if (fallbackToC)
		{
			unsigned char *src = (unsigned char *) pSrcImage;
			unsigned char *dst = (unsigned char *) pchDst;
			unsigned char *dstNext = (unsigned char *) (pchDst + dstImageStrideInBytes);
			unsigned char *dstLast = dst + dstWidth - 1;

			while (dst < dstLast)
			{
				*dst = *src;
				*dst++;
				*dst = *src;
				*dst++;

				*dstNext = *src;
				*dstNext++;
				*dstNext = *src++;
				*dstNext++;
			}
		}
#else
		unsigned char *src = (unsigned char *) pSrcImage;
		unsigned char *dst = (unsigned char *) pchDst;
		unsigned char *dstNext = (unsigned char *) (pchDst + dstImageStrideInBytes);
		unsigned char *dstLast = dst + dstWidth - 1;
		while (dst < dstLast)
		{
			*dst = *src;
			*dst++;
			*dst = *src;
			*dst++;

			*dstNext = *src;
			*dstNext++;
			*dstNext = *src++;
			*dstNext++;
		}
#endif
		pchDst += (dstImageStrideInBytes * 2);
		pSrcImage += srcImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_WarpAffine_U8_U8_Nearest
	(
		vx_uint32	     dstWidth,
		vx_uint32	     dstHeight,
		vx_uint8	   * pDstImage,
		vx_uint32	     dstImageStrideInBytes,
		vx_uint32	     srcWidth,
		vx_uint32	     srcHeight,
		vx_uint8	   * pSrcImage,
		vx_uint32	     srcImageStrideInBytes,
		ago_affine_matrix_t * matrix,
		vx_uint8	   * pLocalData
	)
{
	const float r00 = matrix->matrix[0][0];
	const float r10 = matrix->matrix[0][1];
	const float r01 = matrix->matrix[1][0];
	const float r11 = matrix->matrix[1][1];
	const float const1 = matrix->matrix[2][0];
	const float const2 = matrix->matrix[2][1];

#if ENABLE_MSA
	v4f32 ymap, xmap, ydest, xdest;
	v16i8 zeromask = __builtin_msa_ldi_b(0);

	v4i32 sX = __builtin_msa_fill_w(srcWidth);
	v4i32 sY = __builtin_msa_fill_w(srcHeight);

	const v4f32 srcbx = __builtin_msa_ffint_s_w(sX);
	const v4f32 srcby = __builtin_msa_ffint_s_w(sY);

	v4i32 srcb = __builtin_msa_fill_w((srcHeight * srcImageStrideInBytes) - 1);
	v4i32 src_s = __builtin_msa_fill_w(srcImageStrideInBytes);
#endif

	// check if all mapped pixels are valid or not
	bool bBoder = (const1 < 0) | (const2 < 0) | (const1 >= srcWidth) | (const2 >= srcHeight);
	// check for (dstWidth, 0)
	float x1 = (r00 * dstWidth + const1);
	float y1 = (r10 * dstWidth + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);
	// check for (0, dstHeight)
	x1 = (r01 * dstHeight + const1);
	y1 = (r11 * dstHeight + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);
	// check for (dstWidth, dstHeight)
	x1 = (r00 * dstWidth + r01 * dstHeight + const1);
	y1 = (r10 * dstWidth + r11 * dstHeight + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);

	unsigned int x, y;
	float *r00_x, *r10_x;
	r00_x = (float*) pLocalData;
	r10_x = r00_x + dstWidth;
	for (x = 0; x < dstWidth; x++){
		r00_x[x] = r00 * x;
		r10_x[x] = r10 * x;
	}
	y = 0;

	if (bBoder){
		while (y < dstHeight)
		{

			unsigned int x = 0;
			vx_float32 yC1 = y * r01 + const1;
			vx_float32 yC2 = y * r11 + const2;

#if ENABLE_MSA
			unsigned int *dst = (unsigned int *) pDstImage;

			// calculate (y*m[0][1] + m[0][2]) for x and y
			xdest = (v4f32){yC1, yC1, yC1, yC1};
			ydest = (v4f32){yC2, yC2, yC2, yC2};

#else
			unsigned char *dst = (unsigned char *) pDstImage;
#endif
			while (x < dstWidth)
			{
#if ENABLE_MSA
				v4i32 xpels, ypels;
				// read x into xpel
				xmap = (v4f32) __builtin_msa_ld_w((void *) &r00_x[x], 0);
				// xf = dst[x3, x2, x1, x0]
				xmap = __builtin_msa_fadd_w(xmap, xdest);
				ymap = (v4f32) __builtin_msa_ld_w((void *) &r10_x[x], 0);
				// ymap <- r10*x + ty
				ymap = __builtin_msa_fadd_w(ymap, ydest);

				v16u8 mask = (v16u8) __builtin_msa_fcle_w((v4f32) {0,0,0,0}, xmap);
				v16u8 fclt = (v16u8) __builtin_msa_fclt_w(xmap, srcbx);
				mask = __builtin_msa_and_v(mask, fclt);

				fclt = (v16u8) __builtin_msa_fclt_w((v4f32) {0,0,0,0}, ymap);
				mask = __builtin_msa_and_v(mask, fclt);

				fclt = (v16u8) __builtin_msa_fclt_w(ymap, srcby);
				mask = __builtin_msa_and_v(mask, fclt);

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				// multiply ydest*srcImageStrideInBytes
				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// maddv is not used due to bug in gcc version 8.2.0 (Debian 8.2.0-14)
				// in this gcc version the order of the parameters is wrong
				// ypels = (v4u32) __builtin_msa_maddv_w( (v4i32) xpels, src_s, (v4i32) ypels);

				// check if the values exceed boundary and clamp it to boundary :: need to do this to avoid memory access violations
				ypels = __builtin_msa_min_s_w(ypels, srcb);
				ypels = __builtin_msa_max_s_w(ypels, (v4i32) zeromask);

				xpels = (v4i32){pSrcImage[ypels[0]], pSrcImage[ypels[1]], pSrcImage[ypels[2]], pSrcImage[ypels[3]]};


				xpels = (v4i32) __builtin_msa_and_v((v16u8) xpels, mask);

				// convert to unsigned char and write to dst
				xpels = (v4i32) __builtin_msa_pckev_h((v8i16) zeromask, (v8i16) xpels);
				xpels = (v4i32) __builtin_msa_pckev_b(zeromask, (v16i8) xpels);

				*dst++ = __builtin_msa_copy_u_w(xpels, 0);
				x += 4;
#else	// C
				vx_float32 xmap = r00_x[x] + yC1;
				vx_float32 ymap = r10_x[x] + yC2;

				vx_int32 mask = (FLT_MIN <= xmap ? INT32_MAX : INT32_MIN) & (xmap < (float) srcWidth ? INT32_MAX : INT32_MIN) &
					(FLT_MIN < ymap ? INT32_MAX : INT32_MIN) & (ymap < (float) srcHeight ? INT32_MAX : INT32_MIN);

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				ypels = (ypels < ((srcHeight * srcImageStrideInBytes) - 1)) ? ypels : ((srcHeight * srcImageStrideInBytes) - 1);
				ypels = (ypels > 0) ? ypels : 0;

				vx_int32 xpels = pSrcImage[ypels] & mask;
				unsigned short *pok = (unsigned short *) &xpels;

				*dst++ = pok[0];
				++x;
#endif
			}
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	else
	{
		while (y < dstHeight)
		{
			unsigned int x = 0;
			vx_float32 yC1 = y * r01 + const1;
			vx_float32 yC2 = y * r11 + const2;
#if ENABLE_MSA
			// calculate (y*m[0][1] + m[0][2]) for x and y
			xdest = (v4f32){yC1, yC1, yC1, yC1};
			ydest = (v4f32){yC2, yC2, yC2, yC2};
#endif
			unsigned char *dst = (unsigned char *) pDstImage;

			while (x < dstWidth)
			{
#if ENABLE_MSA
				v4i32 xpels, ypels;
				// read x into xpel
				xmap = (v4f32) __builtin_msa_ld_w((void *) &r00_x[x], 0);
				// xf = dst[x3, x2, x1, x0]
				xmap = __builtin_msa_fadd_w(xmap, xdest);

				ymap = (v4f32) __builtin_msa_ld_w((void *) &r10_x[x], 0);
				// ymap <- r10*x + ty
				ymap = __builtin_msa_fadd_w(ymap, ydest);

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				// multiply ydest*srcImageStrideInBytes
				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// maddv is not used due to bug in gcc version 8.2.0 (Debian 8.2.0-14)
				// in this gcc version the order of the parameters is wrong
				// ypels = (v4u32) __builtin_msa_maddv_w( (v4i32) xpels, src_s, (v4i32) ypels);

				*dst++ = pSrcImage[ypels[0]];
				*dst++ = pSrcImage[ypels[1]];
				*dst++ = pSrcImage[ypels[2]];
				*dst++ = pSrcImage[ypels[3]];

				x += 4;
#else	// C
				vx_float32 xmap = r00_x[x] + yC1;
				vx_float32 ymap = r10_x[x] + yC2;

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				*dst++ = *((unsigned char *) &pSrcImage[ypels]);
				x += 1;
#endif
			}
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	return AGO_SUCCESS;
}

int HafCpu_WarpAffine_U8_U8_Nearest_Constant
	(
		vx_uint32	     dstWidth,
		vx_uint32	     dstHeight,
		vx_uint8	   * pDstImage,
		vx_uint32	     dstImageStrideInBytes,
		vx_uint32	     srcWidth,
		vx_uint32	     srcHeight,
		vx_uint8	   * pSrcImage,
		vx_uint32	     srcImageStrideInBytes,
		ago_affine_matrix_t * matrix,
		vx_uint8	     border,
		vx_uint8	   * pLocalData
	)
{
	const float r00 = matrix->matrix[0][0];
	const float r10 = matrix->matrix[0][1];
	const float r01 = matrix->matrix[1][0];
	const float r11 = matrix->matrix[1][1];
	const float const1 = matrix->matrix[2][0];
	const float const2 = matrix->matrix[2][1];
	const unsigned int u32_border = border | (border << 8) | (border << 16) | (border << 24);
	int pb[4] = {(int) border, 0, 0, 0};

#if ENABLE_MSA
	v4f32 ymap, xmap, ydest, xdest;
	v16i8 zeromask = __builtin_msa_ldi_b(0);
	v16u8 ones = (v16u8) __builtin_msa_fill_b(1);

	v4i32 sX = __builtin_msa_fill_w(srcWidth);
	v4i32 sY = __builtin_msa_fill_w(srcHeight);

	const v4f32 srcbx = __builtin_msa_ffint_s_w(sX);
	const v4f32 srcby = __builtin_msa_ffint_s_w(sY);

	v4i32 srcb = __builtin_msa_fill_w((srcHeight * srcImageStrideInBytes) - 1);
	v4i32 src_s = __builtin_msa_fill_w(srcImageStrideInBytes);

	v4i32 pborder = __builtin_msa_ld_w((void *) &pb, 0);
	pborder = __builtin_msa_shf_w(pborder, 0);
#endif

	// check if all mapped pixels are valid or not
	bool bBoder = (const1 < 0) | (const2 < 0) | (const1 >= srcWidth) | (const2 >= srcHeight);
	// check for (dstWidth, 0)
	float x1 = (r00 * dstWidth + const1);
	float y1 = (r10 * dstWidth + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);
	// check for (0, dstHeight)
	x1 = (r01 * dstHeight + const1);
	y1 = (r11 * dstHeight + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);
	// check for (dstWidth, dstHeight)
	x1 = (r00 * dstWidth + r01 * dstHeight + const1);
	y1 = (r10 * dstWidth + r11 * dstHeight + const2);
	bBoder |= (x1 < 0) | (y1 < 0) | (x1 >= srcWidth) | (y1 >= srcHeight);

	unsigned int x, y;
	float *r00_x = (float*) pLocalData;
	float *r10_x = r00_x + dstWidth;
	for (x = 0; x < dstWidth; x++)
	{
		r00_x[x] = r00 * x;
		r10_x[x] = r10 * x;
	}
	y = 0;

	if (bBoder){
		while (y < dstHeight)
		{
			unsigned int x = 0;
			vx_float32 yC1 = y * r01 + const1;
			vx_float32 yC2 = y * r11 + const2;

#if ENABLE_MSA
			unsigned int *dst = (unsigned int *) pDstImage;

			xdest = (v4f32){yC1, yC1, yC1, yC1};
			ydest = (v4f32){yC2, yC2, yC2, yC2};
#else
			unsigned char *dst = (unsigned char *) pDstImage;
#endif
			while (x < dstWidth)
			{
#if ENABLE_MSA
				v4i32 xpels, ypels;
				// read x into xpel
				xmap = (v4f32) __builtin_msa_ld_w((void *) &r00_x[x], 0);
				// xf = dst[x3, x2, x1, x0]
				xmap = __builtin_msa_fadd_w(xmap, xdest);
				ymap = (v4f32) __builtin_msa_ld_w((void *) &r10_x[x], 0);
				// ymap <- r10*x + ty
				ymap = __builtin_msa_fadd_w(ymap, ydest);

				v16u8 mask = (v16u8) __builtin_msa_fcle_w((v4f32) {0,0,0,0}, xmap);
				v16u8 fclt = (v16u8) __builtin_msa_fclt_w(xmap, srcbx);
				mask = __builtin_msa_and_v(mask, fclt);

				fclt = (v16u8) __builtin_msa_fclt_w((v4f32) zeromask, ymap);
				mask = __builtin_msa_and_v(mask, fclt);

				fclt = (v16u8) __builtin_msa_fclt_w(ymap, srcby);
				mask = __builtin_msa_and_v(mask, fclt);

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// maddv is not used due to bug in gcc version 8.2.0 (Debian 8.2.0-14)
				// in this gcc version the order of the parameters is wrong
				// ypels = (v4u32) __builtin_msa_maddv_w( (v4i32) xpels, src_s, (v4i32) ypels);

				// check if the values exceed boundary and clamp it to boundary :: need to do this to avoid memory access violations
				ypels = __builtin_msa_min_s_w(ypels, srcb);

				// check if the values exceed boundary and clamp it to boundary
				xpels = (v4i32){pSrcImage[ypels[0]], pSrcImage[ypels[1]], pSrcImage[ypels[2]], pSrcImage[ypels[3]]};

				// mask for boundary: boundary pixels will substituted with xero
				xpels = (v4i32) __builtin_msa_and_v((v16u8) xpels, mask);

				// combined result
				v16u8 temp = __builtin_msa_nor_v((v16u8) mask, (v16u8) mask);
				temp = __builtin_msa_and_v(temp, (v16u8) pborder);
				xpels = (v4i32) __builtin_msa_or_v((v16u8) xpels, temp);

				// convert to unsigned char and write to dst
				xpels = (v4i32) __builtin_msa_pckev_h((v8i16) zeromask, (v8i16) xpels);
				xpels = (v4i32) __builtin_msa_pckev_b(zeromask, (v16i8) xpels);

				*dst++ = __builtin_msa_copy_u_w(xpels, 0);

				x += 4;
#else	// C
				vx_float32 xmap = r00_x[x] + yC1;
				vx_float32 ymap = r10_x[x] + yC2;

				vx_int32 mask = (FLT_MIN <= xmap ? INT32_MAX : INT32_MIN) & (xmap < (float) srcWidth ? INT32_MAX : INT32_MIN) &
					(FLT_MIN < ymap ? INT32_MAX : INT32_MIN) & (ymap < (float) srcHeight ? INT32_MAX : INT32_MIN);

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				ypels = (ypels < ((srcHeight * srcImageStrideInBytes) - 1)) ? ypels : ((srcHeight * srcImageStrideInBytes) - 1);
				ypels = (ypels > 0) ? ypels : 0;

				vx_int32 xpels = (pSrcImage[ypels] & mask) | (~mask & border);

				unsigned short *pok = (unsigned short *) &xpels;
				*dst++ = pok[0];
				++x;
#endif
			}
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	else
	{
		while (y < dstHeight)
		{
			unsigned int x = 0;
			vx_float32 yC1 = y * r01 + const1;
			vx_float32 yC2 = y * r11 + const2;

#if ENABLE_MSA
			// calculate (y*m[0][1] + m[0][2]) for x and y
			xdest = (v4f32){yC1, yC1, yC1, yC1};
			ydest = (v4f32){yC2, yC2, yC2, yC2};
#endif
			unsigned char *dst = (unsigned char *) pDstImage;

			while (x < dstWidth)
			{
#if ENABLE_MSA
				v4i32 xpels, ypels;
				// read x into xpel
				xmap = (v4f32) __builtin_msa_ld_w((void *) &r00_x[x], 0);
				// xf = dst[x3, x2, x1, x0]
				xmap = __builtin_msa_fadd_w(xmap, xdest);
				ymap = (v4f32) __builtin_msa_ld_w((void *) &r10_x[x], 0);
				// ymap <- r10*x + ty
				ymap = __builtin_msa_fadd_w(ymap, ydest);

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// maddv is not used due to bug in gcc version 8.2.0 (Debian 8.2.0-14)
				// in this gcc version the order of the parameters is wrong
				// ypels = (v4u32) __builtin_msa_maddv_w( (v4i32) xpels, src_s, (v4i32) ypels);

				*dst++ = pSrcImage[ypels[0]];
				*dst++ = pSrcImage[ypels[1]];
				*dst++ = pSrcImage[ypels[2]];
				*dst++ = pSrcImage[ypels[3]];

				x += 4;
#else	 // C
				vx_float32 xmap = r00_x[x] + yC1;
				vx_float32 ymap = r10_x[x] + yC2;

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				*dst++ = *((unsigned char *) &pSrcImage[ypels]);

				x += 1;
#endif
			}
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	return AGO_SUCCESS;
}

int HafCpu_WarpPerspective_U8_U8_Nearest
	(
		vx_uint32			dstWidth,
		vx_uint32			dstHeight,
		vx_uint8			* pDstImage,
		vx_uint32			dstImageStrideInBytes,
		vx_uint32			srcWidth,
		vx_uint32			srcHeight,
		vx_uint8			* pSrcImage,
		vx_uint32			srcImageStrideInBytes,
		ago_perspective_matrix_t	* matrix,
		vx_uint8			* pLocalData
	)
{
	return HafCpu_WarpPerspective_U8_U8_Nearest_Constant(dstWidth, dstHeight, pDstImage, dstImageStrideInBytes, srcWidth,
		srcHeight, pSrcImage, srcImageStrideInBytes, matrix, (unsigned char)0, pLocalData);
}

// This alogorithm implements Constant Denominator method described in
// " A Novel Architechture for real time sprite decoding".
// The idea is to do perpective warping along the lines of constant divisor..
// The number of floating point divisions are reduced from O(Nsqared) to O(N)
/*
	forward mapping:
		x' = (ax+by+c)/(gx+hy+1)
		y' = (dx+ey+f)/(gx+hy+1)
	backward mapping:
		x  = ((hf-e)x'+(b-hc)y'+(ec-bf))/(eg-dh)x'+(ah-bg)y'+(db-ae))
		y  = ((d-fg)x'+(cg-a)y'+(af-dc))/(eg-dh)x'+(ah-bg)y'+(db-ae))

*/
int HafCpu_WarpPerspective_U8_U8_Nearest_Constant
	(
		vx_uint32                  dstWidth,
		vx_uint32                  dstHeight,
		vx_uint8                 * pDstImage,
		vx_uint32                  dstImageStrideInBytes,
		vx_uint32                  srcWidth,
		vx_uint32                  srcHeight,
		vx_uint8                 * pSrcImage,
		vx_uint32                  srcImageStrideInBytes,
		ago_perspective_matrix_t * matrix,
		vx_uint8                   border,
		vx_uint8		* pLocalData
	)
{
	// calculate inverse mapping coefficients for x and y
	const float a = matrix->matrix[0][0];
	const float d = matrix->matrix[0][1];
	const float g = matrix->matrix[0][2];
	const float b = matrix->matrix[1][0];
	const float e = matrix->matrix[1][1];
	const float h = matrix->matrix[1][2];
	const float c = matrix->matrix[2][0];
	const float f = matrix->matrix[2][1];
	const float i = matrix->matrix[2][2];

	// can't assume if end points in the warped image is within boundary, all the warped image is within boundary
	bool bBoder = 1;

#if ENABLE_MSA
	v16u8 mask;
	v4f32 xdest, ydest, zdest;

	v4i32 zeromask = __builtin_msa_ldi_w(0);
	v4i32 one = __builtin_msa_ldi_w(1);
	v4f32 oneFloat = __builtin_msa_ffint_s_w(one);

	v4i32 pborder = __builtin_msa_fill_w((int) border);

	v4i32 srcb = __builtin_msa_fill_w((int) ((srcHeight * srcImageStrideInBytes) - 1));
	v4i32 src_s = __builtin_msa_fill_w((int) srcImageStrideInBytes);

	v4f32 srcbx = __builtin_msa_ffint_s_w(__builtin_msa_fill_w(srcWidth));
	v4f32 srcby = __builtin_msa_ffint_s_w(__builtin_msa_fill_w(srcHeight));
#endif

	unsigned int x;
	float *A_x = (float *) pLocalData;
	float *D_x = (float *) ALIGN16(A_x + dstWidth);
	float *G_x = (float *) ALIGN16(D_x + dstWidth);

	for (x = 0; x < dstWidth; x++)
	{
		A_x[x] = a * x;
		D_x[x] = d * x;
		G_x[x] = g * x;			// (eg - dh)
	}

	unsigned int y = 0;
	// do the plain vanilla version with floating point division in inner_loop
	if (bBoder)
	{
		while (y < dstHeight)
		{
			x = 0;

			const float v1 = y * b + c;
			const float v2 = y * e + f;
			const float v3 = y * h + i;

#if ENABLE_MSA
			unsigned int *dst = (unsigned int *) pDstImage;

			xdest = (v4f32) {v1, v1, v1, v1};
			ydest = (v4f32) {v2, v2, v2, v2};
			zdest = (v4f32) {v3, v3, v3, v3};

			while (x < dstWidth)
			{
				v4f32 xmap, ymap, zmap;
				v4i32 xpels, ypels;

				zmap = (v4f32) __builtin_msa_ld_w((void *) &G_x[x], 0);
				xmap = (v4f32) __builtin_msa_ld_w((void *) &A_x[x], 0);

				zmap = __builtin_msa_fadd_w(zmap, zdest);
				ymap = (v4f32) __builtin_msa_ld_w((void *) &D_x[x], 0);

				zmap = __builtin_msa_fdiv_w(oneFloat, zmap);
				xmap = __builtin_msa_fadd_w(xmap, xdest);
				ymap = __builtin_msa_fadd_w(ymap, ydest);

				xmap = __builtin_msa_fmul_w(xmap, zmap);
				ymap = __builtin_msa_fmul_w(ymap, zmap);

				mask = (v16u8) __builtin_msa_fcle_w((v4f32) zeromask, xmap);
				mask = __builtin_msa_and_v(mask, (v16u8) __builtin_msa_fclt_w(xmap, srcbx));
				mask = __builtin_msa_and_v(mask, (v16u8) __builtin_msa_fclt_w((v4f32) zeromask, ymap));
				mask = __builtin_msa_and_v(mask, (v16u8) __builtin_msa_fclt_w(ymap, srcby));

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				// multiply ydest*srcImageStrideInBytes
				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// check if the values exceed boundary and clamp it to boundary :: need to do this to avoid memory access violations
				ypels = __builtin_msa_min_s_w(ypels, srcb);
				ypels = __builtin_msa_max_s_w(ypels, zeromask);

				// check if the values exceed boundary and clamp it to boundary
				xpels = (v4i32) {pSrcImage[ypels[0]], pSrcImage[ypels[1]], pSrcImage[ypels[2]], pSrcImage[ypels[3]]};
				// mask for boundary
				xpels = (v4i32) __builtin_msa_and_v((v16u8) xpels, (v16u8) mask);

				// combined result
				v16u8 temp = __builtin_msa_nor_v((v16u8) mask, (v16u8) mask);
				temp = __builtin_msa_and_v(temp, (v16u8) pborder);
				xpels = (v4i32) __builtin_msa_or_v((v16u8) xpels, temp);

				// convert to unsigned char and write to dst
				xpels = (v4i32) __builtin_msa_pckev_h((v8i16) zeromask, (v8i16) xpels);
				xpels = (v4i32) __builtin_msa_pckev_b((v16i8) zeromask, (v16i8) xpels);

				*dst++ = __builtin_msa_copy_u_w(xpels, 0);
				x += 4;
			}
#else // C
			unsigned char *dst = (unsigned char *) pDstImage;
			while (x < dstWidth)
			{
				vx_float32 zmap = 1.0f / (G_x[x] + v3);
				vx_float32 xmap = (A_x[x] + v1) * zmap;
				vx_float32 ymap = (D_x[x] + v2) * zmap;

				vx_int32 mask = (FLT_MIN <= xmap ? INT32_MAX : INT32_MIN) & (xmap < (float) srcWidth ? INT32_MAX : INT32_MIN) &
				(FLT_MIN < ymap ? INT32_MAX : INT32_MIN) & (ymap < (float) srcHeight ? INT32_MAX : INT32_MIN);

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				ypels = (ypels < ((srcHeight * srcImageStrideInBytes) - 1)) ? ypels : ((srcHeight * srcImageStrideInBytes) - 1);
				ypels = (ypels > 0) ? ypels : 0;

				vx_int32 xpels = (pSrcImage[ypels] & mask) | (~mask & border);

				unsigned char *pok = (unsigned char *) &xpels;

				*dst++ = pok[0];
				x++;
			}
#endif
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	else
	{
		while (y < dstHeight)
		{
			x = 0;

			const float v1 = y * b + c;
			const float v2 = y * e + f;
			const float v3 = y * h + i;
#if ENABLE_MSA
			unsigned int *dst = (unsigned int *) pDstImage;

			xdest = (v4f32) {v1, v1, v1, v1};
			ydest = (v4f32) {v2, v2, v2, v2};
			zdest = (v4f32) {v3, v3, v3, v3};

			while (x < dstWidth)
			{
				v4f32 xmap, ymap, zmap;
				v4i32 xpels, ypels;

				zmap = (v4f32) __builtin_msa_ld_w((void *) &G_x[x], 0);
				xmap = (v4f32) __builtin_msa_ld_w((void *) &A_x[x], 0);
				zmap = __builtin_msa_fadd_w(zmap, zdest);
				ymap = (v4f32) __builtin_msa_ld_w((void *) &D_x[x], 0);
				zmap = __builtin_msa_fdiv_w(oneFloat, zmap);
				xmap = __builtin_msa_fadd_w(xmap, xdest);
				ymap = __builtin_msa_fadd_w(ymap, ydest);
				xmap = __builtin_msa_fmul_w(xmap, zmap);
				ymap = __builtin_msa_fmul_w(ymap, zmap);

				// convert to integer with rounding towards zero
				xpels = __builtin_msa_ftrunc_s_w(xmap);
				ypels = __builtin_msa_ftrunc_s_w(ymap);

				// multiply ydest*srcImageStrideInBytes
				ypels = __builtin_msa_mulv_w(ypels, src_s);
				// pixel location at src for dst image.
				ypels = __builtin_msa_addv_w(ypels, xpels);

				// check if the values exceed boundary and clamp it to boundary
				xpels = (v4i32) {pSrcImage[ypels[0]], pSrcImage[ypels[1]], pSrcImage[ypels[2]], pSrcImage[ypels[3]]};

				// convert to unsigned char and write to dst
				xpels = (v4i32) __builtin_msa_pckev_h((v8i16) zeromask, (v8i16) xpels);
				xpels = (v4i32) __builtin_msa_pckev_b((v16i8) zeromask, (v16i8) xpels);

				*dst++ = __builtin_msa_copy_u_w(xpels, 0);

				x += 4;
			}
#else // C
			unsigned char *dst = (unsigned char *) pDstImage;
			while (x < dstWidth)
			{
				vx_float32 zmap = 1.0f / (G_x[x] + v3);
				vx_float32 xmap = (A_x[x] + v1) * zmap;
				vx_float32 ymap = (D_x[x] + v2) * zmap;

				vx_int32 ypels = (vx_int32) ymap * srcImageStrideInBytes + (vx_int32) xmap;

				*dst++ = *((unsigned char *) &pSrcImage[ypels]);

				++x;
			}
#endif
			y++;
			pDstImage += dstImageStrideInBytes;
		}
	}
	return AGO_SUCCESS;
}