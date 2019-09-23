#include "ago_internal.h"
#include "mips_internal.h"

#if ENABLE_MSA
static inline void Count_MSA(v16i8 pixels, v16i8 thresh, v4i32 *count, v16u8 onemask)
{
	v8i16 temp16;
	v4i32 temp32;

	v16i8 cmpout = __builtin_msa_clt_s_b(thresh, pixels);
	cmpout = (v16i8) __builtin_msa_and_v((v16u8) cmpout, onemask);
	cmpout = __builtin_msa_asub_s_b(cmpout, (v16i8) onemask);
	temp16 = __builtin_msa_hadd_s_h(cmpout, cmpout);
	temp32 = __builtin_msa_hadd_s_w(temp16, temp16);
	cmpout = (v16i8) __builtin_msa_hadd_s_d(temp32, temp32);
	*count = __builtin_msa_addv_w(*count, (v4i32) cmpout);
}
static inline void Count_2_MSA(v16i8 pixels, v16i8 thresh, v4i32 *count, v16u8 onemask, int slide)
{
	v8i16 temp16;
	v4i32 temp32;

	v16i8 cmpout = __builtin_msa_clt_s_b(thresh, pixels);
	cmpout = (v16i8) __builtin_msa_and_v((v16u8) cmpout, onemask);
	cmpout = __builtin_msa_asub_s_b(cmpout, (v16i8) onemask);

	temp16 = __builtin_msa_hadd_s_h(cmpout, cmpout);
	temp32 = __builtin_msa_hadd_s_w(temp16, temp16);
	cmpout = (v16i8) __builtin_msa_hadd_s_d(temp32, temp32);
	cmpout = (v16i8) __builtin_msa_slli_d((v2i64) cmpout, slide);
	*count = __builtin_msa_addv_w(*count, (v4i32) cmpout);
}
#else
static inline void Count_C(vx_int8 pixel, char thresh, int count[2], char i, char j)
{
	vx_int8 cmpout = (pixel > thresh) ? 255 : 0;
	cmpout &= 1;
	cmpout = abs(cmpout - 1);

	if(i < 8)
	{
		i++;
	}
	else
	{
		i = 1;
		j = (j == 0) ? 1 : 0;
	}
	count[j] += cmpout;
}
static inline void Count_2_C(vx_int8 pixel, char thresh, int count[8], char &i, char index)
{
	vx_int8 cmpout = (pixel > thresh) ? 255 : 0;
	cmpout &= 1;
	cmpout = abs(cmpout - 1);

	if(i < 8)
	{
		count[index] += cmpout;
	}
	else
	{
		count[index + 4] += cmpout;
	}
	i++;
	if(i == 16)
		i = 0;
}
#endif

static int HafCpu_Histogram1Threshold_DATA_U8
	(
		vx_uint32     dstHist[],
		vx_uint8      distThreshold,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	// offset: to convert the range from 0..255 to -128..127, because SSE does not have compare instructions for unsigned bytes
	// thresh: source threshold in -128..127 range
#if ENABLE_MSA
	v16i8 *src_msa;

	v16i8 offset = __builtin_msa_ldi_b((char) 0x80);
	v16i8 thresh = __builtin_msa_fill_b((char) ((distThreshold - 1) ^ 0x80));
	v16u8 onemask = (v16u8) __builtin_msa_ldi_b(1);

	// process one pixel row at a time that counts "pixel < srcThreshold"
	v4i32 count = __builtin_msa_ldi_w((char) 0);
#else // C
	char offset = (char) 0x80;
	char thresh = (char) ((distThreshold - 1) ^ 0x80);
	int count[2] = {0, 0};
#endif

	vx_uint8 * srcRow = pSrcImage;
	vx_uint32 width = (srcWidth + 15) >> 4;

	for (unsigned int y = 0; y < srcHeight; y++)
	{
#if ENABLE_MSA
		src_msa = (v16i8 *) srcRow;

		for (unsigned int x = 0; x < width; x++)
		{
			v16i8 pixels = __builtin_msa_ld_b((void *) src_msa++, 0);
			pixels = (v16i8) __builtin_msa_xor_v((v16u8) pixels, (v16u8) offset);

			Count_MSA(pixels, thresh, &count, onemask);
		}
#else // C
		char i = 0, j = 0;
		for (unsigned int x = 0; x < srcWidth; x++)
		{
			vx_int8 pixel = *((vx_int8 *) &srcRow[x]);
			pixel = pixel ^ offset;

			Count_C(pixel, thresh, count, i, j);
		}
#endif
		srcRow += srcImageStrideInBytes;
	}
	// extract histogram from count
#if ENABLE_MSA
	dstHist[0] = ((unsigned int *) &count)[0] + ((unsigned int *) &count)[2];
#else
	dstHist[0] = count[0] + count[1];
#endif
	dstHist[1] = srcWidth * srcHeight - dstHist[0];

	return AGO_SUCCESS;
}

static int HafCpu_Histogram3Thresholds_DATA_U8
	(
		vx_uint32     dstHist[],
		vx_uint8      distThreshold0,
		vx_uint8      distThreshold1,
		vx_uint8      distThreshold2,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	vx_uint32 width = (srcWidth + 15) >> 4;
	vx_uint8 * srcRow = pSrcImage;
	// offset: to convert the range from 0..255 to -128..127, because SSE does not have compare instructions for unsigned bytes
	// thresh: source threshold in -128..127 range
#if ENABLE_MSA
	v16i8 *src_msa;

	v16i8 offset = __builtin_msa_fill_b((char) 0x80);
	v16i8 T0 = __builtin_msa_fill_b((char) ((distThreshold0 - 1) ^ 0x80));
	v16i8 T1 = __builtin_msa_fill_b((char) ((distThreshold1 - 1) ^ 0x80));
	v16i8 T2 = __builtin_msa_fill_b((char) ((distThreshold2 - 1) ^ 0x80));
	v16u8 onemask = (v16u8) __builtin_msa_ldi_b(1);

	// process one pixel row at a time that counts "pixel < srcThreshold"
	v4i32 count0 = __builtin_msa_fill_w((char) 0);
	v4i32 count1 = __builtin_msa_fill_w((char) 0);
	v4i32 count2 = __builtin_msa_fill_w((char) 0);
#else // C
	char offset = (char) 0x80;
	char T0 = (char) ((distThreshold0 - 1) ^ 0x80);
	char T1 = (char) ((distThreshold1 - 1) ^ 0x80);
	char T2 = (char) ((distThreshold2 - 1) ^ 0x80);

	int count0[2] = {0, 0};
	int count1[2] = {0, 0};
	int count2[2] = {0, 0};
#endif

	for (unsigned int y = 0; y < srcHeight; y++)
	{
#if ENABLE_MSA
		src_msa = (v16i8 *) srcRow;

		for (unsigned int x = 0; x < width; x++)
		{
			v16i8 pixels = __builtin_msa_ld_b((void *) src_msa++, 0);
			pixels = (v16i8) __builtin_msa_xor_v((v16u8) pixels, (v16u8) offset);

			Count_MSA(pixels, T0, &count0, onemask);
			Count_MSA(pixels, T1, &count1, onemask);
			Count_MSA(pixels, T2, &count2, onemask);
		}
#else // C
		char i0 = 0, j0 = 0;
		char i1 = 0, j1 = 0;
		char i2 = 0, j2 = 0;
		for (unsigned int x = 0; x < srcWidth; x++)
		{
			vx_int8 pixel = *((vx_int8 *) &srcRow[x]);
			pixel = pixel ^ offset;

			Count_C(pixel, T0, count0, i0, j0);
			Count_C(pixel, T1, count1, i1, j1);
			Count_C(pixel, T2, count2, i2, j2);
		}
#endif
		srcRow += srcImageStrideInBytes;
	}

	// extract histogram from count: special case needed when T1 == T2
#if ENABLE_MSA
	dstHist[0] = ((unsigned int *) &count0)[0] + ((unsigned int *) &count0)[2];
	dstHist[1] = ((unsigned int *) &count1)[0] + ((unsigned int *) &count1)[2] - dstHist[0];
	dstHist[2] = ((unsigned int *) &count2)[0] + ((unsigned int *) &count2)[2] - dstHist[0] - dstHist[1];
#else
	dstHist[0] = count0[0] + count0[1];
	dstHist[1] = count1[0] + count1[1] - dstHist[0];
	dstHist[2] = count2[0] + count2[1] - dstHist[0] - dstHist[1];
#endif
	dstHist[3] = srcWidth * srcHeight - dstHist[0] - dstHist[1] - dstHist[2];

	if (((char *) &T1)[0] == ((char *) &T2)[0]) {
		dstHist[2] = dstHist[3];
		dstHist[3] = 0;
	}

	return AGO_SUCCESS;
}

// Implementation of these functions fails the CTS, so these cases will be covered by HafCpu_Histogram_DATA_U8
#if 0
static int HafCpu_Histogram8Bins_DATA_U8
	(
		vx_uint32   * dstHist,
		vx_uint8      distOffset,
		vx_uint8      distWindow,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	// offset: to convert the range from 0..255 to -128..127, because SSE does not have compare instructions for unsigned bytes
	// thresh: source threshold in -128..127 range
#if ENABLE_MSA
	v16i8 offset = __builtin_msa_ldi_b((char) 0x80);
	v16i8 T0 = __builtin_msa_fill_b((char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80));
	v16i8 dT = __builtin_msa_fill_b((char) distWindow);
	v16u8 onemask = (v16u8) __builtin_msa_ldi_b((char) 1);
#else // C
	char offset = (char) 0x80;
	char T0 = (char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80);
	char dT = (char) distWindow;
	char onemask = (char) 1;
#endif
	// process one pixel row at a time that counts "pixel < srcThreshold"
	vx_uint32 count[9] = { 0 };
	vx_uint8 * srcRow = pSrcImage;
	vx_uint32 width = (srcWidth + 15) >> 4;

	for (unsigned int y = 0; y < srcHeight; y++)
	{
#if ENABLE_MSA
		v16i8 * src = (v16i8 *) srcRow;
		v16i8 count0 = __builtin_msa_ldi_b((char) 0);
		v16i8 count1 = __builtin_msa_ldi_b((char) 0);
		v16i8 count2 = __builtin_msa_ldi_b((char) 0);

		for (unsigned int x = 0; x < width; x++)
		{
			v16i8 pixels = __builtin_msa_ld_b((void *) src++, 0);
			pixels = (v16i8) __builtin_msa_xor_v((v16u8) pixels, (v16u8) offset);
			v16i8 cmpout, Tnext = T0;

			// 0..3
			Count_MSA(pixels, Tnext, (v4i32*) &count0, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 48);

			// 4..7
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count1, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 48);

			// 8..9
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count2, onemask);
		}
#else	// C
		int count0[8] = {0};
		int count1[8] = {0};
		int count2[8] = {0};

		char i00 = 0, i01 = 0, i02 = 0, i03 = 0;
		char i10 = 0, i11 = 0, i12 = 0, i13 = 0;
		char i20 = 0;

		vx_int8 Tnext;
		for (unsigned int x = 0; x < srcWidth; x++)
		{
			vx_int8 pixel = *((vx_int8 *) &srcRow[x]);
			pixel = pixel ^ offset;

			// 0..3
			Tnext = T0;
			Count_2_C(pixel, Tnext, count0, i00, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i01, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i02, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i03, 3);

			// 4..7
			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i10, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i11, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i12, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i13, 3);

			// 8..9
			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i20, 0);
		}
#endif
		srcRow += srcImageStrideInBytes;

		// move counts from count0..2 into count[]
#if ENABLE_MSA
		for (int i = 0; i < 4; i++) {
			count[ 0 + i] += ((short *) &count0)[i] + ((short *) &count0)[4 + i];
			count[ 4 + i] += ((short *) &count1)[i] + ((short *) &count1)[4 + i];
		}
		count[8 + 0] += ((short *) &count2)[0] + ((short *) &count2)[4 + 0];
#else
		for (int i = 0; i < 4; i++)
		{
			count[0 + i] += count0[i] + count0[4 + i];
			count[4 + i] += count1[i] + count1[4 + i];
		}
		count[8 + 0] += count2[0] + count2[4 + 0];
#endif
	}

	// extract histogram from count
	if (distOffset == 0)
	{
		vx_uint32 last = (distWindow >= 32) ? srcWidth * srcHeight : count[7];
		for (int i = 6; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
		}
		dstHist[0] = last;
		for (int i = 1; i < 8; i++)
			dstHist[i] = count[i - 1];
	}
	else
	{
		vx_uint32 last = (distOffset + distWindow * 8 - 1 > 255) ? srcWidth * srcHeight : count[8];
		for (int i = 7; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
			dstHist[i] = count[i];
		}
	}

	return AGO_SUCCESS;
}

static int HafCpu_Histogram9Bins_DATA_U8
	(
		vx_uint32   * dstHist,
		vx_uint8      distOffset,
		vx_uint8      distWindow,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	// offset: to convert the range from 0..255 to -128..127, because SSE does not have compare instructions for unsigned bytes
	// thresh: source threshold in -128..127 range
#if ENABLE_MSA
	v16i8 offset = __builtin_msa_ldi_b((char) 0x80);
	v16i8 T0 = __builtin_msa_fill_b((char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80));
	v16i8 dT = __builtin_msa_fill_b((char) distWindow);
	v16u8 onemask = (v16u8) __builtin_msa_ldi_b((char) 1);
#else // C
	char offset = (char) 0x80;
	char T0 = (char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80);
	char dT = (char) distWindow;
	char onemask = (char) 1;
#endif
	// process one pixel row at a time that counts "pixel < srcThreshold"
	vx_uint32 count[10] = { 0 };
	vx_uint8 * srcRow = pSrcImage;
	vx_uint32 width = (srcWidth + 15) >> 4;

	for (unsigned int y = 0; y < srcHeight; y++)
	{
#if ENABLE_MSA
		v16i8 *src = (v16i8 *) srcRow;
		v4i32 count0 = __builtin_msa_ldi_w((char) 0);
		v4i32 count1 = __builtin_msa_ldi_w((char) 0);
		v4i32 count2 = __builtin_msa_ldi_w((char) 0);

		for (unsigned int x = 0; x < width; x++)
		{
			v16i8 pixels = __builtin_msa_ld_b((void *) src++, 0);
			pixels = (v16i8) __builtin_msa_xor_v((v16u8) pixels, (v16u8) offset);

			v16i8 cmpout;
			v16i8 Tnext = T0;

			// 0..3
			Count_MSA(pixels, Tnext, (v4i32*) &count0, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 48);

			// 4..7
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count1, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 48);

			// 8..9
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count2, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count2, onemask, 16);
		}
#else	// C
		int count0[8] = {0};
		int count1[8] = {0};
		int count2[8] = {0};

		char i00 = 0, i01 = 0, i02 = 0, i03 = 0;
		char i10 = 0, i11 = 0, i12 = 0, i13 = 0;
		char i20 = 0, i21 = 0;

		vx_int8 Tnext;
		for (unsigned int x = 0; x < srcWidth; x++)
		{
			vx_int8 pixel = *((vx_int8 *) &srcRow[x]);
			pixel = pixel ^ offset;

			// 0..3
			Tnext = T0;
			Count_2_C(pixel, Tnext, count0, i00, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i01, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i02, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i03, 3);

			// 4..7
			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i10, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i11, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i12, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i13, 3);

			// 8..9
			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i20, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i21, 1);
		}
#endif
		srcRow += srcImageStrideInBytes;

		// move counts from count0..2 into count[]
#if ENABLE_MSA
		for (int i = 0; i < 4; i++)
		{
			count[0 + i] += ((short *) &count0)[i] + ((short *) &count0)[4 + i];
			count[4 + i] += ((short *) &count1)[i] + ((short *) &count1)[4 + i];
		}
		count[8 + 0] += ((short *) &count2)[0] + ((short *) &count2)[4 + 0];
		count[8 + 1] += ((short *) &count2)[1] + ((short *) &count2)[4 + 1];
#else
		for (int i = 0; i < 4; i++)
		{
			count[0 + i] += count0[i] + count0[4 + i];
			count[4 + i] += count1[i] + count1[4 + i];
		}
		count[8 + 0] += count2[0] + count2[4 + 0];
		count[8 + 1] += count2[1] + count2[4 + 1];
#endif
	}

	// extract histogram from count
	if (distOffset == 0)
	{
		vx_uint32 last = (distWindow >= 29) ? srcWidth * srcHeight : count[8];
		for (int i = 7; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
		}
		dstHist[0] = last;
		for (int i = 1; i < 9; i++)
			dstHist[i] = count[i - 1];
	}
	else
	{
		vx_uint32 last = (distOffset + distWindow * 9 - 1 > 255) ? srcWidth * srcHeight : count[9];
		for (int i = 8; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
			dstHist[i] = count[i];
		}
	}
	return AGO_SUCCESS;
}
#endif

static int HafCpu_Histogram16Bins_DATA_U8
	(
		vx_uint32   * dstHist,
		vx_uint8      distOffset,
		vx_uint8      distWindow,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	// offset: to convert the range from 0..255 to -128..127, because SSE does not have compare instructions for unsigned bytes
	// thresh: source threshold in -128..127 range
#if ENABLE_MSA
	v16i8 offset = __builtin_msa_ldi_b((char) 0x80);
	v16i8 T0 = __builtin_msa_fill_b((char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80));
	v16i8 dT = __builtin_msa_fill_b((char) distWindow);
	v16u8 onemask = (v16u8) __builtin_msa_ldi_b((char) 1);
#else // C
	char offset = (char) 0x80;
	char T0 = (char) (((distOffset ? distOffset : distWindow) - 1) ^ 0x80);
	char dT = (char) distWindow;
#endif
	// process one pixel row at a time that counts "pixel < srcThreshold"
	vx_uint32 count[16] = { 0 };
	vx_uint8 * srcRow = pSrcImage;
	vx_uint32 width = (srcWidth + 15) >> 4;

	for (unsigned int y = 0; y < srcHeight; y++)
	{
#if ENABLE_MSA
		v16i8 * src = (v16i8 *) srcRow;

		v16i8 count0 = __builtin_msa_ldi_b((char) 0);
		v16i8 count1 = __builtin_msa_ldi_b((char) 0);
		v16i8 count2 = __builtin_msa_ldi_b((char) 0);
		v16i8 count3 = __builtin_msa_ldi_b((char) 0);
		for (unsigned int x = 0; x < width; x++)
		{
			v16i8 pixels = __builtin_msa_ld_b((void *) src++, 0);
			pixels = (v16i8) __builtin_msa_xor_v((v16u8) pixels, (v16u8) offset);
			v16i8 cmpout, Tnext = T0;

			// 0..3
			Count_MSA(pixels, Tnext, (v4i32*) &count0, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count0, onemask, 48);

			// 4..7
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count1, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count1, onemask, 48);

			// 8..11
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count2, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count2, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count2, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count2, onemask, 48);

			// 12..15
			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_MSA(pixels, Tnext, (v4i32*) &count3, onemask);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count3, onemask, 16);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count3, onemask, 32);

			Tnext = __builtin_msa_addv_b(Tnext, dT);
			Count_2_MSA(pixels, Tnext, (v4i32*) &count3, onemask, 48);
		}
#else	// C
		int count0[8] = {0};
		int count1[8] = {0};
		int count2[8] = {0};
		int count3[8] = {0};

		char i00 = 0, i01 = 0, i02 = 0, i03 = 0;
		char i10 = 0, i11 = 0, i12 = 0, i13 = 0;
		char i20 = 0, i21 = 0, i22 = 0, i23 = 0;
		char i30 = 0, i31 = 0, i32 = 0, i33 = 0;

		vx_int8 Tnext;
		for (unsigned int x = 0; x < srcWidth; x++)
		{
			vx_int8 pixel = *((vx_int8 *) &srcRow[x]);
			pixel = pixel ^ offset;

			// 0..3
			Tnext = T0;
			Count_2_C(pixel, Tnext, count0, i00, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i01, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i02, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count0, i03, 3);

			// 4..7
			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i10, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i11, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i12, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count1, i13, 3);

			// 8..11
			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i20, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i21, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i22, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count2, i23, 3);

			// 11..15
			Tnext += dT;
			Count_2_C(pixel, Tnext, count3, i30, 0);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count3, i31, 1);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count3, i32, 2);

			Tnext += dT;
			Count_2_C(pixel, Tnext, count3, i33, 3);
		}
#endif
		srcRow += srcImageStrideInBytes;

		// move counts from count0..2 into count[]
#if ENABLE_MSA
		for (int i = 0; i < 4; i++) {
			count[ 0 + i] += ((short *) &count0)[i] + ((short *) &count0)[4 + i];
			count[ 4 + i] += ((short *) &count1)[i] + ((short *) &count1)[4 + i];
			count[ 8 + i] += ((short *) &count2)[i] + ((short *) &count2)[4 + i];
			count[12 + i] += ((short *) &count3)[i] + ((short *) &count3)[4 + i];
		}
#else
		for (int i = 0; i < 4; i++) {
			count[ 0 + i] += count0[i] + count0[4 + i];
			count[ 4 + i] += count1[i] + count1[4 + i];
			count[ 8 + i] += count2[i] + count2[4 + i];
			count[12 + i] += count3[i] + count3[4 + i];
		}
#endif
	}

	// extract histogram from count
	if (distOffset == 0)
	{
		vx_uint32 last = (distWindow >= 16) ? srcWidth * srcHeight : count[15];
		for (int i = 14; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
		}
		dstHist[0] = last;
		for (int i = 1; i < 16; i++)
			dstHist[i] = count[i - 1];
	}
	else
	{
		vx_uint32 last = srcWidth * srcHeight;
		for (int i = 15; i >= 0; i--)
		{
			count[i] = last - count[i];
			last -= count[i];
			dstHist[i] = count[i];
		}
	}
	return AGO_SUCCESS;
}

int HafCpu_HistogramFixedBins_DATA_U8
	(
		vx_uint32     dstHist[],
		vx_uint32     distBinCount,
		vx_uint32     distOffset,
		vx_uint32     distRange,
		vx_uint32     distWindow,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	int status = AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;

	// compute number of split points in [0..255] range to compute the histogram
	vx_int32 numSplits = (distBinCount - 1) + ((distOffset > 0) ? 1 : 0) + (((distOffset + distRange) < 256) ? 1 : 0);

	// Use general code if width is not multiple of 8 or the buffer is unaligned
	bool useGeneral = (srcWidth & 7) || (((intptr_t) pSrcImage) & 15);

	if ((numSplits < 1 && distBinCount > 1) || (distBinCount == 0)) return status;

	if (numSplits <= 3 && !useGeneral)
	{
		if (numSplits == 0)
		{
			dstHist[0] = srcWidth * srcHeight;
			status = VX_SUCCESS;
		}
		else if (numSplits == 1)
		{
			vx_uint32 hist[2];
			status = HafCpu_Histogram1Threshold_DATA_U8(hist, distOffset ? distOffset : distWindow, srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);
			if (distBinCount == 1)
			{
				dstHist[0] = hist[distOffset > 0 ? 1 : 0];
			}
			else
			{
				dstHist[0] = hist[0];
				dstHist[1] = hist[1];
			}
		}
		else
		{
			// compute thresholds (split-points)
			vx_uint8 thresh[3], tlast = 0;
			vx_uint32 split = 0;
			if (distOffset > 0)
				tlast = thresh[split++] = distOffset;
			for (vx_uint32 bin = 1; bin < distBinCount; bin++)
				tlast = thresh[split++] = tlast + distWindow;
			if (split < 3)
			{
				if (((int) distOffset + distRange) < 256)
					tlast = thresh[split++] = tlast + distWindow;
				while (split < 3)
					thresh[split++] = tlast;
			}
			vx_uint32 count[4];
			status = HafCpu_Histogram3Thresholds_DATA_U8(count, thresh[0], thresh[1], thresh[2], srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);

			if (!status)
			{
				for (vx_uint32 i = 0; i < distBinCount; i++)
				{
					dstHist[i] = count[i + (distOffset ? 1 : 0)];
				}
			}
		}
	}
	// Implementation of these functions fails the CTS, so these cases will be covered by HafCpu_Histogram_DATA_U8
#if 0
	else if (distBinCount == 8 && !useGeneral)
	{
		status = HafCpu_Histogram8Bins_DATA_U8(dstHist, distOffset, distWindow, srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);
	}
	else if (distBinCount == 9 && !useGeneral)
	{
		status = HafCpu_Histogram9Bins_DATA_U8(dstHist, distOffset, distWindow, srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);
	}
#endif
	else if (distBinCount == 16 && numSplits <= 16 && !useGeneral)
	{
		status = HafCpu_Histogram16Bins_DATA_U8(dstHist, distOffset, distWindow, srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);
	}
	else
	{
		// use general 256-bin histogram
		vx_uint32 histTmp[256];
		status = HafCpu_Histogram_DATA_U8(histTmp, srcWidth, srcHeight, pSrcImage, srcImageStrideInBytes);
		if (!status)
		{
			// convert [256] histogram into [numbins]
			if (distWindow == 1)
			{
				memcpy(dstHist, &histTmp[distOffset], distBinCount * sizeof(vx_uint32));
			}
			else
			{
				for( vx_uint32 i = distOffset; i < distOffset + distRange; i++ )
				{
					int j = (i - distOffset) * distBinCount / distRange;
					dstHist[j] = (int32_t) (dstHist[j] + histTmp[i]);
				}
			}
		}
	}
	return status;
}
