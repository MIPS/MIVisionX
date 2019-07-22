#include "ago_internal.h"
#include "mips_internal.h"

#define FP_BITS		18
#define FP_MUL		(1 << FP_BITS)

int HafCpu_ScaleImage_U8_U8_Nearest
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
		v16i8 zeromask = __builtin_msa_ldi_b(0);
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
				mapx2 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) mapx0);

				signmask = (v8i16) __builtin_msa_clti_s_h((v8i16) mapx0, 0);
				mapx0 = (v16u8) __builtin_msa_ilvr_h(signmask, (v8i16) mapx0);

				mapx3 = (v16u8) __builtin_msa_ilvl_h((v8i16) zeromask, (v8i16) mapx1);
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
vx_uint32     dstWidth,
vx_uint32     dstHeight,
vx_uint8    * pDstImage,
vx_uint32     dstImageStrideInBytes,
vx_uint8    * pSrcImage,
vx_uint32     srcImageStrideInBytes
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
