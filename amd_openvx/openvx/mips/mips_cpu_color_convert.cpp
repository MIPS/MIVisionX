#include "ago_internal.h"
#include "mips_internal.h"

DECL_ALIGN(16) unsigned char dataColorConvert[16 * 26] ATTR_ALIGN(16) = {
	  1,   3,   5,   7,   9,  11,  13,  15, 255, 255, 255, 255, 255, 255, 255, 255,		// UYVY to IYUV - Y; UV12 to IUV - V (lower); NV21 to IYUV - U; UYVY to NV12 - Y; YUYV to NV12 - UV
	  0,   4,   8,  12, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// UYVY to IYUV - U
	  2,   6,  10,  14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// UYVY to IYUV - V
	  0,   2,   4,   6,   8,  10,  12,  14, 255, 255, 255, 255, 255, 255, 255, 255,		// YUYV to IYUV - Y; UV12 to IUV - U (lower); NV21 to IYUV - V; UYVY to NV12 - UV; YUYV to NV12 - Y
	  1,   5,   9,  13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// YUYV to IYUV - U
	  3,   7,  11,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// YUYV to IYUV - V
	  0,   0,   2,   2,   4,   4,   6,   6,   8,   8,  10,  10,  12,  12,  14,  14,		// UV12 to UV - U; NV21 to YUV4 - V
	  1,   1,   3,   3,   5,   5,   7,   7,   9,   9,  11,  11,  13,  13,  15,  15,		// VV12 to UV - V; NV21 to YUV4 - U
	  0,   1,   2,   4,   5,   6,   8,   9,  10,  12,  13,  14, 255, 255, 255, 255,		// RGBX to RGB - First 16 bytes of RGBX to first 16 bytes of RGB
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,   1,   2,   4,		// RGBX to RGB - Second 16 bytes of RGBX to first 16 bytes of RGB
	  5,   6,   8,   9,  10,  12,  13,  14, 255, 255, 255, 255, 255, 255, 255, 255,		// RGBX to RGB - Second 16 bytes of RGBX to second 16 bytes of RGB
	255, 255, 255, 255, 255, 255, 255, 255,   0,   1,   2,   4,   5,   6,   8,   9,		// RGBX to RGB - Third 16 bytes of RGBX to second 16 bytes of RGB
	 10,  12,  13,  14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// RGBX to RGB - Third 16 bytes of RGBX to third 16 bytes of RGB
	255, 255, 255, 255,   0,   1,   2,   4,   5,   6,   8,   9,  10,  12,  13,  14,		// RGBX to RGB - Fourth 16 bytes of RGBX to third 16 bytes of RGB
	  0,   1,   2, 255,   3,   4,   5, 255,   6,   7,   8, 255,   9,  10,  11, 255,		// RGB to RGBX - First 16 bytes of RGB to first 16 bytes of RGBX
	 12,  13,  14, 255,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// RGB to RGBX - First 16 bytes of RGB to second 16 bytes of RGBX
	255, 255, 255, 255, 255,   0,   1, 255,   2,   3,   4, 255,   5,   6,   7, 255,		// RGB to RGBX - Second 16 bytes of RGB to second 16 bytes of RGBX
	  8,   9,  10, 255,  11,  12,  13, 255,  14,  15, 255, 255, 255, 255, 255, 255,		// RGB to RGBX - Second 16 bytes of RGB to third 16 bytes of RGBX
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0, 255,   1,   2,   3, 255,		// RGB to RGBX - Third 16 bytes of RGB to third 16 bytes of RGBX
	  4,   5,   6, 255,   7,   8,   9, 255,  10,  11,  12, 255,  13,  14,  15, 255,		// RGB to RGBX - Third 16 bytes of RGB to fourth 16 bytes of RGBX
	  0,   0,   0, 255,   0,   0,   0, 255,   0,   0,   0, 255,   0,   0,   0, 255,		// RGB to RGBX - Mask to fill in 255 for X positions
	  0,   3,   6,   9,  12,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// RGB to single plane extraction
	  2,   5,   8,  11,  14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// RGB to single plane extraction
	  1,   4,   7,  10,  13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// RGB to single plane extraction
	255, 255, 255, 255, 255, 255, 255, 255,   1,   3,   5,   7,   9,  11,  13,  15,		// UV12 to IUV - V (upper)
	255, 255, 255, 255, 255, 255, 255, 255,   0,   2,   4,   6,   8,  10,  12,  14		// UV12 to IUV - U (upper)
};

int HafCpu_ColorConvert_Y_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstYImage,
		vx_uint32     dstYImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
#if ENABLE_MSA
	v16i8 *tbl = (v16i8 *) dataColorConvert;
	v16u8 R, G, B;
	v4u32 pixels0, pixels1, pixels2;
	v4f32 temp, Y;
	v4u32 temp0_u, temp1_u;

	v16i8 mask1 = __builtin_msa_ld_b(tbl + 21, 0);
	v16i8 mask2 = __builtin_msa_ld_b(tbl + 22, 0);
	v16i8 mask3 = __builtin_msa_ld_b(tbl + 23, 0);

	float wR[4] = {0.2126f, 0.2126f, 0.2126f, 0.2126f};
	float wG[4] = {0.7152f, 0.7152f, 0.7152f, 0.7152f};
	float wB[4] = {0.0722f, 0.0722f, 0.0722f, 0.0722f};

	v4f32 weights_R = (v4f32) __builtin_msa_ld_w(&wR, 0);
	v4f32 weights_G = (v4f32) __builtin_msa_ld_w(&wG, 0);
	v4f32 weights_B = (v4f32) __builtin_msa_ld_w(&wB, 0);

	v16i8 zero = __builtin_msa_ldi_b(0);

	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;
#endif
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 *pLocalSrc = pSrcImage;
		vx_uint8 *pLocalDst = pDstYImage;
#if ENABLE_MSA
		for (int width = 0; width < (alignedWidth >> 4); width++)
		{
			pixels0 = (v4u32) __builtin_msa_ld_b(pLocalSrc, 0);
			pixels1 = (v4u32) __builtin_msa_ld_b(pLocalSrc + 16, 0);
			pixels2 = (v4u32) __builtin_msa_ld_b(pLocalSrc + 32, 0);

			// 0 0 0 0 0 0 0 0 0 0 R5 R4 R3 R2 R1 R0
			R = (v16u8) __builtin_msa_vshf_b(mask1, (v16i8) pixels0, (v16i8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 0 G4 G3 G2 G1 G0
			G = (v16u8) __builtin_msa_vshf_b(mask3, (v16i8) pixels0, (v16i8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 0 B4 B3 B2 B1 B0
			B = (v16u8) __builtin_msa_vshf_b(mask2, (v16i8) pixels0, (v16i8) pixels0);

			// 0 0 0 0 0 0 0 0 0 0 0 0 R10 R9 R8 R7 R6
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask2, (v16i8) pixels1, (v16i8) pixels1);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 10);
			// 0 0 0 0 0 R10 R9 R8 R7 R6 R5 R4 R3 R2 R1 R0
			R = __builtin_msa_or_v(R, (v16u8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 G10 G9 G8 G7 G6 G5
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask1, (v16i8) pixels1, (v16i8) pixels1);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 11);
			// 0 0 0 0 0 G10 G9 G8 G7 G6 G5 G4 G3 G2 G1 G0
			G = __builtin_msa_or_v(G, (v16u8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 0 B9 B8 B7 B6 B5
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask3, (v16i8) pixels1, (v16i8) pixels1);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 11);
			// 0 0 0 0 0 0 B9 B8 B7 B6 B5 B4 B3 B2 B1 B0
			B = __builtin_msa_or_v(B, (v16u8) pixels0);

			// 0 0 0 0 0 0 0 0 0 0 0 R15 R14 R13 R12 R11
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask3, (v16i8) pixels2, (v16i8) pixels2);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 5);
			// R15 R14 R13 R12 R11 R10 R9 R8 R7 R6 R5 R4 R3 R2 R1 R0
			R = __builtin_msa_or_v(R, (v16u8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 0 G15 G14 G13 G12 G11
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask2, (v16i8) pixels2, (v16i8) pixels2);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 5);
			// G15 G14 G13 G12 G11 G10 G9 G8 G7 G6 G5 G4 G3 G2 G1 G0
			G = __builtin_msa_or_v(G, (v16u8) pixels0);
			// 0 0 0 0 0 0 0 0 0 0 B15 B14 B13 B12 B11 B10
			pixels0 = (v4u32) __builtin_msa_vshf_b(mask1, (v16i8) pixels2, (v16i8) pixels2);
			pixels0 = (v4u32) __builtin_msa_sldi_b((v16i8) pixels0, (v16i8) pixels0, 6);
			// B15 B14 B13 B12 B11 B10 B9 B8 B7 B6 B5 B4 B3 B2 B1 B0
			B = __builtin_msa_or_v(B, (v16u8) pixels0);

			// For pixels 0..3
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) R);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			// R0..R3
			temp = __builtin_msa_ffint_u_w(pixels2);
			Y = __builtin_msa_fmul_w(temp, weights_R);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) G);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// G0..G3
			temp = __builtin_msa_fmul_w(temp, weights_G);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) B);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// B0..B3
			temp = __builtin_msa_fmul_w(temp, weights_B);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels0 = (v4u32) __builtin_msa_ftrunc_u_w(Y);

			// For pixels 4..7
			R = (v16u8) __builtin_msa_sldi_b((v16i8) R, (v16i8) R, 4);
			G = (v16u8) __builtin_msa_sldi_b((v16i8) G, (v16i8) G, 4);
			B = (v16u8) __builtin_msa_sldi_b((v16i8) B, (v16i8) B, 4);

			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) R);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			// R4..R7
			temp = __builtin_msa_ffint_u_w(pixels2);
			Y = __builtin_msa_fmul_w(temp, weights_R);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) G);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// G4..G7
			temp = __builtin_msa_fmul_w(temp, weights_G);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) B);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// B4..B7
			temp = __builtin_msa_fmul_w(temp, weights_B);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels1 = __builtin_msa_ftrunc_u_w(Y);
			temp0_u = __builtin_msa_sat_u_w(pixels0, 15);
			temp1_u = __builtin_msa_sat_u_w(pixels1, 15);
			pixels0 = (v4u32) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			// For pixels 8..11
			R = (v16u8) __builtin_msa_sldi_b((v16i8) R, (v16i8) R, 4);
			G = (v16u8) __builtin_msa_sldi_b((v16i8) G, (v16i8) G, 4);
			B = (v16u8) __builtin_msa_sldi_b((v16i8) B, (v16i8) B, 4);

			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) R);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			// R8..R11
			temp = __builtin_msa_ffint_u_w(pixels2);
			Y = __builtin_msa_fmul_w(temp, weights_R);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) G);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// G8..G11
			temp = __builtin_msa_fmul_w(temp, weights_G);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) B);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// B8..B11
			temp = __builtin_msa_fmul_w(temp, weights_B);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels1 = __builtin_msa_ftrunc_u_w(Y);

			// For pixels 12..15
			R = (v16u8) __builtin_msa_sldi_b((v16i8) R, (v16i8) R, 4);
			G = (v16u8) __builtin_msa_sldi_b((v16i8) G, (v16i8) G, 4);
			B = (v16u8) __builtin_msa_sldi_b((v16i8) B, (v16i8) B, 4);

			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) R);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			// R12..R15
			temp = __builtin_msa_ffint_u_w(pixels2);
			Y = __builtin_msa_fmul_w(temp, weights_R);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) G);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// G12..G15
			temp = __builtin_msa_fmul_w(temp, weights_G);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels2 = (v4u32) __builtin_msa_ilvr_b(zero, (v16i8) B);
			pixels2 = (v4u32) __builtin_msa_ilvr_h((v8i16) zero, (v8i16) pixels2);
			temp = __builtin_msa_ffint_u_w(pixels2);
			// B12..B15
			temp = __builtin_msa_fmul_w(temp, weights_B);
			Y = __builtin_msa_fadd_w(Y, temp);
			pixels2 = __builtin_msa_ftrunc_u_w(Y);
			temp0_u = __builtin_msa_sat_u_w(pixels1, 15);
			temp1_u = __builtin_msa_sat_u_w(pixels2, 15);
			pixels1 = (v4u32) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

			temp0_u = (v4u32) __builtin_msa_sat_u_h((v8u16) pixels0, 7);
			temp1_u = (v4u32) __builtin_msa_sat_u_h((v8u16) pixels1, 7);
			pixels0 = (v4u32) __builtin_msa_pckev_b((v16i8) temp1_u, (v16i8) temp0_u);
			__builtin_msa_st_b((v16i8) pixels0, (void*) pLocalDst, 0);

			pLocalSrc += 48;
			pLocalDst += 16;
		}

		for (int width = 0; width < postfixWidth; width++)
		{
			float R = (float) *pLocalSrc++;
			float G = (float) *pLocalSrc++;
			float B = (float) *pLocalSrc++;

			*pLocalDst++ = (vx_uint8) ((R * 0.2126f) + (G * 0.7152f) + (B * 0.0722f));
		}
#else
		for (int width = 0; width < dstWidth; width++)
		{
			float R = (float) *pLocalSrc++;
			float G = (float) *pLocalSrc++;
			float B = (float) *pLocalSrc++;

			*pLocalDst++ = (vx_uint8) ((R * 0.2126f) + (G * 0.7152f) + (B * 0.0722f));
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstYImage += dstYImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGB_IYUV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcYImage,
		vx_uint32     srcYImageStrideInBytes,
		vx_uint8    * pSrcUImage,
		vx_uint32     srcUImageStrideInBytes,
		vx_uint8    * pSrcVImage,
		vx_uint32     srcVImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcY = pSrcYImage;
		vx_uint8 * pLocalSrcU = pSrcUImage;
		vx_uint8 * pLocalSrcV = pSrcVImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcY);
			Rpix = (float) (*pLocalSrcV++) - 128.0f;
			Bpix = (float) (*pLocalSrcU++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcY + 1));
			*(pLocalDst + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcY + srcYImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcY + srcYImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			pLocalSrcY += 2;
			pLocalDst += 6;
		}

		pSrcYImage += (srcYImageStrideInBytes + srcYImageStrideInBytes);
		pSrcUImage += srcUImageStrideInBytes;
		pSrcVImage += srcVImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGB_UYVY
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
	int alignedWidth = dstWidth & ~7;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 shufMask = {0, 1, 2, 4, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	v16i8 tempI, row;
	v4f32 Y0, Y1, U, V;
	v8u16 temp0_u, temp1_u;

	// BT709 conversion factors
	// x R G B, The most significant float is don't care
	v4f32 weights_U2RGB = {0.0f, -0.1873f, 1.8556f, 0.0f};
	// x R G B, The most significant float is don't care
	v4f32 weights_V2RGB = {1.5748f, -0.4681f, 0.0f, 0.0f};
	v4f32 const128 = {128.0f, 128.0f, 128.0f, 128.0f};
	v4u32 mask_1110 = {0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00};
#endif

	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

#if ENABLE_MSA
		for (int width = 0; width < alignedWidth; width += 8)
		{
			row = __builtin_msa_ld_b(pLocalSrc, 0);

			for (int i = 0; i < 4; i++)
			{
				tempI = __builtin_msa_vshf_b((v16i8) mask_1110, row, row);
				// U U U U
				U = __builtin_msa_ffint_s_w((v4i32) tempI);
				U = __builtin_msa_fsub_w(U, const128);

				row = __builtin_msa_sldi_b(row, row, 1);
				tempI = __builtin_msa_vshf_b((v16i8) mask_1110, row, row);
				// Y0 Y0 Y0 Y0
				Y0 = __builtin_msa_ffint_s_w((v4i32) tempI);

				row = __builtin_msa_sldi_b(row, row, 1);
				tempI = __builtin_msa_vshf_b((v16i8) mask_1110, row, row);
				// V V V V
				V = __builtin_msa_ffint_s_w((v4i32) tempI);
				V = __builtin_msa_fsub_w(V, const128);

				row = __builtin_msa_sldi_b(row, row, 1);
				tempI = __builtin_msa_vshf_b((v16i8) mask_1110, row, row);
				// Y1 Y1 Y1 Y1
				Y1 = __builtin_msa_ffint_s_w((v4i32) tempI);

				row = __builtin_msa_sldi_b(row, row, 1);
				U = __builtin_msa_fmul_w(U, weights_U2RGB);
				V = __builtin_msa_fmul_w(V, weights_V2RGB);

				// weights_U*U + weights_V*V
				U = __builtin_msa_fadd_w(U, V);

				// RGB for pixel 0
				Y0 = __builtin_msa_fadd_w(Y0, U);

				// RGB for pixel 1
				Y1 = __builtin_msa_fadd_w(Y1, U);

				// Convert RGB01 to U8
				temp0_u = __builtin_msa_sat_u_h((v8u16) __builtin_msa_ftrunc_u_w(Y0), 15);
				temp1_u = __builtin_msa_sat_u_h((v8u16) __builtin_msa_ftrunc_u_w(Y1), 15);
				tempI = (v16i8) __builtin_msa_pckev_h((v8i16) temp1_u, (v8i16) temp0_u);

				temp0_u = (v8u16)__builtin_msa_sat_u_h((v8u16) tempI, 7);
				tempI = (v16i8) __builtin_msa_pckev_b((v16i8) temp0_u, (v16i8) temp0_u);

				tempI = __builtin_msa_vshf_b((v16i8) shufMask, tempI, tempI);

				__builtin_msa_st_b((v16i8) tempI, (void *) (pLocalDst + 6 * i), 0);
			}

			pLocalSrc += 16;
			pLocalDst += 24;
		}

		for (int width = 0; width < postfixWidth; width += 2)
		{
			float Ypix1, Ypix2, Upix, Vpix, Rpix, Gpix, Bpix;
			Upix = (float) (*pLocalSrc++) - 128.0f;
			Ypix1 = (float) (*pLocalSrc++);
			Vpix = (float) (*pLocalSrc++) - 128.0f;
			Ypix2 = (float) (*pLocalSrc++);

			Rpix = fminf(fmaxf(Ypix1 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix1 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix1 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;

			Rpix = fminf(fmaxf(Ypix2 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix2 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix2 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
		}
#else // C
		for (int width = 0; width < dstWidth; width += 2)
		{
			float Ypix1, Ypix2, Upix, Vpix, Rpix, Gpix, Bpix;
			Upix = (float) (*pLocalSrc++) - 128.0f;
			Ypix1 = (float) (*pLocalSrc++);
			Vpix = (float) (*pLocalSrc++) - 128.0f;
			Ypix2 = (float) (*pLocalSrc++);

			Rpix = fminf(fmaxf(Ypix1 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix1 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix1 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;

			Rpix = fminf(fmaxf(Ypix2 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix2 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix2 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
		}
#endif
		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}

	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGBX_IYUV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcYImage,
		vx_uint32     srcYImageStrideInBytes,
		vx_uint8    * pSrcUImage,
		vx_uint32     srcUImageStrideInBytes,
		vx_uint8    * pSrcVImage,
		vx_uint32     srcVImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcY = pSrcYImage;
		vx_uint8 * pLocalSrcU = pSrcUImage;
		vx_uint8 * pLocalSrcV = pSrcVImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcY);
			Rpix = (float) (*pLocalSrcV++) - 128.0f;
			Bpix = (float) (*pLocalSrcU++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcY + 1));
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 7) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcY + srcYImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcY + srcYImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 7) = (vx_uint8) 255;

			pLocalSrcY += 2;
			pLocalDst += 8;
		}
		pSrcYImage += (srcYImageStrideInBytes + srcYImageStrideInBytes);
		pSrcUImage += srcUImageStrideInBytes;
		pSrcVImage += srcVImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGBX_YUYV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix1, Ypix2, Upix, Vpix, Rpix, Gpix, Bpix;
			Ypix1 = (float) (*pLocalSrc++);
			Upix = (float) (*pLocalSrc++) - 128.0f;
			Ypix2 = (float) (*pLocalSrc++);
			Vpix = (float) (*pLocalSrc++) - 128.0f;

			Rpix = fminf(fmaxf(Ypix1 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix1 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix1 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
			*pLocalDst++ = (vx_uint8) 255;

			Rpix = fminf(fmaxf(Ypix2 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix2 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix2 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
			*pLocalDst++ = (vx_uint8) 255;
		}

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGBX_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height++)
	{
		vx_uint8 * pLocalSrc = pSrcImage;
		vx_uint8 * pLocalDst = pDstImage;

		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix1, Ypix2, Upix, Vpix, Rpix, Gpix, Bpix;
			Upix = (float) (*pLocalSrc++) - 128.0f;
			Ypix1 = (float) (*pLocalSrc++);
			Vpix = (float) (*pLocalSrc++) - 128.0f;
			Ypix2 = (float) (*pLocalSrc++);

			Rpix = fminf(fmaxf(Ypix1 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix1 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix1 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
			*pLocalDst++ = (vx_uint8 )255;

			Rpix = fminf(fmaxf(Ypix2 + (Vpix * 1.5748f), 0.0f), 255.0f);
			Gpix = fminf(fmaxf(Ypix2 - (Upix * 0.1873f) - (Vpix * 0.4681f), 0.0f), 255.0f);
			Bpix = fminf(fmaxf(Ypix2 + (Upix * 1.8556f), 0.0f), 255.0f);

			*pLocalDst++ = (vx_uint8) Rpix;
			*pLocalDst++ = (vx_uint8) Gpix;
			*pLocalDst++ = (vx_uint8) Bpix;
			*pLocalDst++ = (vx_uint8) 255;
		}

		pSrcImage += srcImageStrideInBytes;
		pDstImage += dstImageStrideInBytes;
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGB_NV12
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcLumaImage,
		vx_uint32     srcLumaImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcLuma = pSrcLumaImage;
		vx_uint8 * pLocalSrcChroma = pSrcChromaImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcLuma);
			Bpix = (float) (*pLocalSrcChroma++) - 128.0f;
			Rpix = (float) (*pLocalSrcChroma++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + 1));
			*(pLocalDst + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			pLocalSrcLuma += 2;
			pLocalDst += 6;
		}
		pSrcLumaImage += (srcLumaImageStrideInBytes + srcLumaImageStrideInBytes);
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGBX_NV12
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcLumaImage,
		vx_uint32     srcLumaImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcLuma = pSrcLumaImage;
		vx_uint8 * pLocalSrcChroma = pSrcChromaImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcLuma);
			Bpix = (float) (*pLocalSrcChroma++) - 128.0f;
			Rpix = (float) (*pLocalSrcChroma++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + 1));
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 7) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 7) = (vx_uint8) 255;

			pLocalSrcLuma += 2;
			pLocalDst += 8;
		}
		pSrcLumaImage += (srcLumaImageStrideInBytes + srcLumaImageStrideInBytes);
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGB_NV21
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcLumaImage,
		vx_uint32     srcLumaImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcLuma = pSrcLumaImage;
		vx_uint8 * pLocalSrcChroma = pSrcChromaImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcLuma);
			Rpix = (float) (*pLocalSrcChroma++) - 128.0f;
			Bpix = (float) (*pLocalSrcChroma++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + 1));
			*(pLocalDst + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);

			pLocalSrcLuma += 2;
			pLocalDst += 6;
		}
		pSrcLumaImage += (srcLumaImageStrideInBytes + srcLumaImageStrideInBytes);
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_ColorConvert_RGBX_NV21
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcLumaImage,
		vx_uint32     srcLumaImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	for (int height = 0; height < (int) dstHeight; height += 2)
	{
		vx_uint8 * pLocalSrcLuma = pSrcLumaImage;
		vx_uint8 * pLocalSrcChroma = pSrcChromaImage;
		vx_uint8 * pLocalDst = pDstImage;

		// Processing two pixels at a time in a row
		for (int width = 0; width < (int) dstWidth; width += 2)
		{
			float Ypix, Rpix, Gpix, Bpix;

			Ypix = (float) (*pLocalSrcLuma);
			Rpix = (float) (*pLocalSrcChroma++) - 128.0f;
			Bpix = (float) (*pLocalSrcChroma++) - 128.0f;

			Gpix = (Bpix * 0.1873f) + (Rpix * 0.4681f);
			Rpix *= 1.5748f;
			Bpix *= 1.8556f;

			*pLocalDst = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + 1));
			*(pLocalDst + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + 7) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes));
			*(pLocalDst + dstImageStrideInBytes + 0) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 1) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 2) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 3) = (vx_uint8) 255;

			Ypix = (float) (*(pLocalSrcLuma + srcLumaImageStrideInBytes + 1));
			*(pLocalDst + dstImageStrideInBytes + 4) = (vx_uint8) fminf(fmaxf(Ypix + Rpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 5) = (vx_uint8) fminf(fmaxf(Ypix - Gpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 6) = (vx_uint8) fminf(fmaxf(Ypix + Bpix, 0.0f), 255.0f);
			*(pLocalDst + dstImageStrideInBytes + 7) = (vx_uint8) 255;

			pLocalSrcLuma += 2;
			pLocalDst += 8;
		}
		pSrcLumaImage += (srcLumaImageStrideInBytes + srcLumaImageStrideInBytes);
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstImage += (dstImageStrideInBytes + dstImageStrideInBytes);
	}
	return AGO_SUCCESS;
}

int HafCpu_FormatConvert_IUV_UV12
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	unsigned char *pLocalSrc, *pLocalDstU, *pLocalDstV;
#if ENABLE_MSA
	v16u8 *pLocalSrc_msa, *pLocalDstU_msa, *pLocalDstV_msa;
	v16u8 pixels0, pixels1, temp;

	int prefixWidth = intptr_t(pDstUImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif

	int height = (int) dstHeight;
	while (height)
	{
#if ENABLE_MSA
		pLocalSrc = (unsigned char *) pSrcChromaImage;
		pLocalDstU = (unsigned char *) pDstUImage;
		pLocalDstV = (unsigned char *) pDstVImage;

		for (int x = 0; x < prefixWidth; x++)
		{
			*pLocalDstU++ = *pLocalSrc++;
			*pLocalDstV++ = *pLocalSrc++;
		}
		pLocalSrc_msa = (v16u8 *) pLocalSrc;
		pLocalDstU_msa = (v16u8 *) pLocalDstU;
		pLocalDstV_msa = (v16u8 *) pLocalDstV;

		// 16 pixels processed at a time
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels0 = (v16u8) __builtin_msa_ld_b(pLocalSrc_msa++, 0);
			pixels1 = (v16u8) __builtin_msa_ld_b(pLocalSrc_msa++, 0);

			temp = (v16u8) __builtin_msa_pckev_b((v16i8) pixels1, (v16i8) pixels0);
			__builtin_msa_st_b((v16i8) temp, pLocalDstU_msa++, 0);

			temp = (v16u8) __builtin_msa_pckod_b((v16i8) pixels1, (v16i8) pixels0);
			__builtin_msa_st_b((v16i8) temp, pLocalDstV_msa++, 0);

			width--;
		}
		pLocalSrc = (unsigned char *) pLocalSrc_msa;
		pLocalDstU = (unsigned char *) pLocalDstU_msa;
		pLocalDstV = (unsigned char *) pLocalDstV_msa;

		for (int x = 0; x < postfixWidth; x++)
		{
			*pLocalDstU++ = *pLocalSrc++;
			*pLocalDstV++ = *pLocalSrc++;
		}
#else // C
		pLocalSrc = (unsigned char *) pSrcChromaImage;
		pLocalDstU = (unsigned char *) pDstUImage;
		pLocalDstV = (unsigned char *) pDstVImage;

		for (int x = 0; x < dstWidth; x++)
		{
			*pLocalDstU++ = *pLocalSrc++;
			*pLocalDstV++ = *pLocalSrc++;
		}
#endif
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstUImage += dstUImageStrideInBytes;
		pDstVImage += dstVImageStrideInBytes;
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_FormatConvert_UV12_IUV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstChromaImage,
		vx_uint32     dstChromaImageStrideInBytes,
		vx_uint8    * pSrcUImage,
		vx_uint32     srcUImageStrideInBytes,
		vx_uint8    * pSrcVImage,
		vx_uint32     srcVImageStrideInBytes
	)
{
	unsigned char *pLocalSrcU, *pLocalSrcV, *pLocalDst;
#if ENABLE_MSA
	int prefixWidth = intptr_t(pDstChromaImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);
	prefixWidth >>= 1; // 2 bytes = 1 pixel

	// 16 pixels processed at a time in MSA loop
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;

	v16i8 *pLocalSrcU_msa, *pLocalSrcV_msa, *pLocalDst_msa;
	v16i8 pixels_U, pixels_V, pixels_out;
#endif

	int height = (int) dstHeight;
	while (height)
	{
#if ENABLE_MSA
		pLocalSrcU = (unsigned char *) pSrcUImage;
		pLocalSrcV = (unsigned char *) pSrcVImage;
		pLocalDst = (unsigned char *) pDstChromaImage;

		for (int x = 0; x < prefixWidth; x++)
		{
			*pLocalDst++ = *pLocalSrcU++;
			*pLocalDst++ = *pLocalSrcV++;
		}

		pLocalSrcU_msa = (v16i8 *) pLocalSrcU;
		pLocalSrcV_msa = (v16i8 *) pLocalSrcV;
		pLocalDst_msa = (v16i8 *) pLocalDst;

		// Each inner loop writes 16 pixels of each chroma plane in destination buffer
		int width = (int) (dstWidth >> 4);
		while (width)
		{
			pixels_U = __builtin_msa_ld_b(pLocalSrcU_msa++, 0);
			pixels_V = __builtin_msa_ld_b(pLocalSrcV_msa++, 0);
			pixels_out = __builtin_msa_ilvr_b(pixels_V, pixels_U);
			pixels_U = __builtin_msa_ilvl_b(pixels_V, pixels_U);

			__builtin_msa_st_b(pixels_out, pLocalDst_msa++, 0);
			__builtin_msa_st_b(pixels_U, pLocalDst_msa++, 0);

			width--;
		}

		pLocalSrcU = (unsigned char *) pLocalSrcU_msa;
		pLocalSrcV = (unsigned char *) pLocalSrcV_msa;
		pLocalDst = (unsigned char *) pLocalDst_msa;

		for (int x = 0; x < postfixWidth; x++)
		{
			*pLocalDst++ = *pLocalSrcU++;
			*pLocalDst++ = *pLocalSrcV++;
		}
#else // C
		pLocalSrcU = (unsigned char *) pSrcUImage;
		pLocalSrcV = (unsigned char *) pSrcVImage;
		pLocalDst = (unsigned char *) pDstChromaImage;

		for (int x = 0; x < dstWidth; x++)
		{
			*pLocalDst++ = *pLocalSrcU++;
			*pLocalDst++ = *pLocalSrcV++;
		}
#endif
		pSrcUImage += srcUImageStrideInBytes;
		pSrcVImage += srcVImageStrideInBytes;
		pDstChromaImage += dstChromaImageStrideInBytes;
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_FormatConvert_UV_UV12
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcChromaImage,
		vx_uint32     srcChromaImageStrideInBytes
	)
{
	vx_uint8 *pLocalSrc, *pLocalDstUCurrentRow, *pLocalDstUNextRow, *pLocalDstVCurrentRow, *pLocalDstVNextRow;
#if ENABLE_MSA
	int alignedWidth = dstWidth & ~15;
	int postfixWidth = (int) dstWidth - alignedWidth;

	v16i8 * tbl = (v16i8 *) dataColorConvert;
	v16i8 pixels, U;
	v16i8 maskU = __builtin_msa_ld_b(tbl + 6, 0);
	v16i8 maskV = __builtin_msa_ld_b(tbl + 7, 0);
#endif

	// Each inner loop writes out two rows of dst buffer
	int height = (int) (dstHeight >> 1);

	while (height)
	{
#if ENABLE_MSA
		pLocalSrc = pSrcChromaImage;
		pLocalDstUCurrentRow = pDstUImage;
		pLocalDstUNextRow = pDstUImage + dstUImageStrideInBytes;
		pLocalDstVCurrentRow = pDstVImage;
		pLocalDstVNextRow = pDstVImage + dstVImageStrideInBytes;

		// Each inner loop iteration processess 16 output pixels
		int width = (int) (alignedWidth >> 4);
		while (width)
		{
			pixels = __builtin_msa_ld_b(pLocalSrc, 0);
			U = __builtin_msa_vshf_b(maskU, pixels, pixels);
			pixels = __builtin_msa_vshf_b(maskV, pixels, pixels);

			__builtin_msa_st_b(U, pLocalDstUCurrentRow, 0);
			__builtin_msa_st_b(U, pLocalDstUNextRow, 0);
			__builtin_msa_st_b(pixels, pLocalDstVCurrentRow, 0);
			__builtin_msa_st_b(pixels, pLocalDstVNextRow, 0);

			pLocalSrc += 16;
			pLocalDstUCurrentRow += 16;
			pLocalDstUNextRow += 16;
			pLocalDstVCurrentRow += 16;
			pLocalDstVNextRow += 16;
			width--;
		}

		for (int w = 0; w < postfixWidth; w += 2)
		{
			*pLocalDstUCurrentRow++ = *pLocalSrc;
			*pLocalDstUCurrentRow++ = *pLocalSrc;
			*pLocalDstUNextRow++ = *pLocalSrc;
			*pLocalDstUNextRow++ = *pLocalSrc++;

			*pLocalDstVCurrentRow++ = *pLocalSrc;
			*pLocalDstVCurrentRow++ = *pLocalSrc;
			*pLocalDstVNextRow++ = *pLocalSrc;
			*pLocalDstVNextRow++ = *pLocalSrc++;
		}
#else // C
		pLocalSrc = pSrcChromaImage;
		pLocalDstUCurrentRow = pDstUImage;
		pLocalDstUNextRow = pDstUImage + dstUImageStrideInBytes;
		pLocalDstVCurrentRow = pDstVImage;
		pLocalDstVNextRow = pDstVImage + dstVImageStrideInBytes;

		for (int w = 0; w < dstWidth; w += 2)
		{
			*pLocalDstUCurrentRow++ = *pLocalSrc;
			*pLocalDstUCurrentRow++ = *pLocalSrc;
			*pLocalDstUNextRow++ = *pLocalSrc;
			*pLocalDstUNextRow++ = *pLocalSrc++;

			*pLocalDstVCurrentRow++ = *pLocalSrc;
			*pLocalDstVCurrentRow++ = *pLocalSrc;
			*pLocalDstVNextRow++ = *pLocalSrc;
			*pLocalDstVNextRow++ = *pLocalSrc++;
		}
#endif
		pSrcChromaImage += srcChromaImageStrideInBytes;
		pDstUImage += (dstUImageStrideInBytes + dstUImageStrideInBytes);
		pDstVImage += (dstVImageStrideInBytes + dstVImageStrideInBytes);
		height--;
	}

	return AGO_SUCCESS;
}

int HafCpu_FormatConvert_IYUV_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstYImage,
		vx_uint32     dstYImageStrideInBytes,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	)
{
	unsigned char *pLocalSrc, *pLocalDstY, *pLocalDstU, *pLocalDstV;
	unsigned char *pLocalSrcNextRow, *pLocalDstYNextRow;
#if ENABLE_MSA
	v16i8 *tbl = (v16i8 *) dataColorConvert;
	v16i8 maskY = __builtin_msa_ld_b(tbl, 0);
	v16i8 maskU = __builtin_msa_ld_b(tbl + 1, 0);
	v16i8 maskV = __builtin_msa_ld_b(tbl + 2, 0);
	v16i8 pixels0, pixels1, pixels0_NextRow, pixels1_NextRow, temp0, temp1;

	int prefixWidth = intptr_t(pDstYImage) & 15;
	prefixWidth = (prefixWidth == 0) ? 0 : (16 - prefixWidth);

	// 16 pixels processed at a time
	int postfixWidth = ((int) dstWidth - prefixWidth) & 15;
	int alignedWidth = (int) dstWidth - prefixWidth - postfixWidth;
#endif

	int height = (int) dstHeight;
	while (height)
	{
#if ENABLE_MSA
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalSrcNextRow = (unsigned char *) pSrcImage + srcImageStrideInBytes;
		pLocalDstY = (unsigned char *) pDstYImage;
		pLocalDstYNextRow = (unsigned char *) pDstYImage + dstYImageStrideInBytes;
		pLocalDstU = (unsigned char *) pDstUImage;
		pLocalDstV = (unsigned char *) pDstVImage;

		for (int x = 0; x < prefixWidth; x++)
		{
			*pLocalDstU++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// U
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
			*pLocalDstV++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// V
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
		}

		// 16 pixels processed at a time
		int width = alignedWidth >> 4;
		while (width)
		{
			pixels0 = __builtin_msa_ld_b(pLocalSrc, 0);
			pixels1 = __builtin_msa_ld_b((pLocalSrc + 16), 0);
			pixels0_NextRow = __builtin_msa_ld_b(pLocalSrcNextRow, 0);
			pixels1_NextRow = __builtin_msa_ld_b((pLocalSrcNextRow + 16), 0);

			// Y plane, bytes 0..7
			temp0 = __builtin_msa_vshf_b(maskY, pixels0, pixels0);
			// Y plane, bytes 8..15
			temp1 = __builtin_msa_vshf_b(maskY, pixels1, pixels1);
			temp1 = __builtin_msa_sldi_b(temp1, temp1, 8);
			temp0 = (v16i8) __builtin_msa_or_v((v16u8) temp0, (v16u8) temp1);
			__builtin_msa_st_b(temp0, pLocalDstY, 0);

			// Y plane - next row, bytes 8..15
			temp1 = __builtin_msa_vshf_b(maskY, pixels1_NextRow, pixels1_NextRow);
			temp1 = __builtin_msa_sldi_b(temp1, temp1, 8);
			// Y plane - next row, bytes 0..7
			temp0 = __builtin_msa_vshf_b(maskY, pixels0_NextRow, pixels0_NextRow);
			temp0 = (v16i8) __builtin_msa_or_v((v16u8) temp0, (v16u8) temp1);
			__builtin_msa_st_b(temp0, pLocalDstYNextRow, 0);

			// U plane, intermideate bytes 4..7
			temp1 = __builtin_msa_vshf_b(maskU, pixels1, pixels1);
			// V plane, intermideate bytes 4..7
			pixels1 = __builtin_msa_vshf_b(maskV, pixels1, pixels1);
			temp1 = __builtin_msa_sldi_b(temp1, temp1, 12);
			pixels1 = __builtin_msa_sldi_b(pixels1, pixels1, 12);

			// U plane, intermideate bytes 0..3
			temp0 = __builtin_msa_vshf_b(maskU, pixels0, pixels0);
			// V plane, intermideate bytes 0..3
			pixels0 = __builtin_msa_vshf_b(maskV, pixels0, pixels0);
			// U plane, intermideate bytes 0..7
			temp0 = (v16i8) __builtin_msa_or_v((v16u8) temp0, (v16u8) temp1);
			// V plane, intermideate bytes 0..7
			pixels0 = (v16i8) __builtin_msa_or_v((v16u8) pixels0, (v16u8) pixels1);

			// U plane - next row, intermideate bytes 4..7
			temp1 = __builtin_msa_vshf_b(maskU, pixels1_NextRow, pixels1_NextRow);
			// V plane - next row, intermideate bytes 4..7
			pixels1_NextRow = __builtin_msa_vshf_b(maskV, pixels1_NextRow, pixels1_NextRow);
			temp1 = __builtin_msa_sldi_b(temp1, temp1, 12);
			pixels1_NextRow = __builtin_msa_sldi_b(pixels1_NextRow, pixels1_NextRow, 12);

			// U plane - next row, intermideate bytes 0..3
			pixels1 = __builtin_msa_vshf_b(maskU, pixels0_NextRow, pixels0_NextRow);
			// V plane - next row, intermideate bytes 0..3
			pixels0_NextRow = __builtin_msa_vshf_b(maskV, pixels0_NextRow, pixels0_NextRow);
			// U plane - next row, intermideate bytes 0..7
			temp1 = (v16i8) __builtin_msa_or_v((v16u8) temp1, (v16u8) pixels1);
			// V plane - next row, intermideate bytes 0..7
			pixels0_NextRow = (v16i8) __builtin_msa_or_v((v16u8) pixels0_NextRow, (v16u8) pixels1_NextRow);

			// U plane, bytes 0..7
			temp0 = (v16i8) __builtin_msa_ave_u_b((v16u8) temp0, (v16u8) temp1);
			*((long long *) pLocalDstU) = ((long long *) &temp0)[0];

			// V plane, bytes 0..7
			pixels0 = (v16i8) __builtin_msa_ave_u_b((v16u8) pixels0, (v16u8) pixels0_NextRow);
			*((long long *) pLocalDstV) = ((long long *) &pixels0)[0];

			pLocalSrc += 32;
			pLocalSrcNextRow += 32;
			pLocalDstY += 16;
			pLocalDstYNextRow += 16;
			pLocalDstU += 8;
			pLocalDstV += 8;
			width--;
		}

		for (int x = 0; x < postfixWidth; x++)
		{
			*pLocalDstU++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// U
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
			*pLocalDstV++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// V
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
		}
#else // C
		pLocalSrc = (unsigned char *) pSrcImage;
		pLocalSrcNextRow = (unsigned char *) pSrcImage + srcImageStrideInBytes;
		pLocalDstY = (unsigned char *) pDstYImage;
		pLocalDstYNextRow = (unsigned char *) pDstYImage + dstYImageStrideInBytes;
		pLocalDstU = (unsigned char *) pDstUImage;
		pLocalDstV = (unsigned char *) pDstVImage;

		for (int x = 0; x < dstWidth; x+=2)
		{
			*pLocalDstU++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// U
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
			*pLocalDstV++ = (*pLocalSrc++ + *pLocalSrcNextRow++) >> 1;	// V
			*pLocalDstY++ = *pLocalSrc++;					// Y
			*pLocalDstYNextRow++ = *pLocalSrcNextRow++;			// Y - next row
		}
#endif
		pSrcImage += (srcImageStrideInBytes + srcImageStrideInBytes);		// Advance by 2 rows
		pDstYImage += (dstYImageStrideInBytes + dstYImageStrideInBytes);	// Advance by 2 rows
		pDstUImage += dstUImageStrideInBytes;
		pDstVImage += dstVImageStrideInBytes;

		height -= 2;
	}

	return AGO_SUCCESS;
}
