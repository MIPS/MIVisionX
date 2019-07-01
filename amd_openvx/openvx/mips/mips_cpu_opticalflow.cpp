#include "ago_internal.h"
#include "mips_internal.h"

/* Implements OpticalFlow pyramid algorithm */

typedef struct
{
	vx_float32 x;		      /*!< \brief The x coordinate. */
	vx_float32 y;		      /*!< \brief The y coordinate. */
} pt2f;

typedef struct
{
	vx_int32 x;		    /*!< \brief The x coordinate. */
	vx_int32 y;		    /*!< \brief The y coordinate. */
} pt2i;

static const int W_BITS = 14;
static const float FLT_SCALE = 1.f / (1 << 20);
static const float MinEugThreshold = 1.0e-04F;
static const float Epsilon = 1.0e-07F;

#define DESCALE(x, n) (((x) + (1 << ((n) - 1))) >> (n))

// helper function
static inline void pt_copy(pt2f &pt1, pt2f &pt2) { pt1.x = pt2.x; pt1.y = pt2.y; }

DECL_ALIGN(16) unsigned char shuffleMasks[16 * 26] ATTR_ALIGN(16) = {
	0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};

static void ComputeSharr
	(
		vx_uint32	dstImageStrideInBytes,
		vx_uint8	* dst,
		vx_uint32	srcWidth,
		vx_uint32	srcHeight,
		vx_uint32	srcImageStrideInBytes,
		vx_uint8	* src,
		vx_uint8	* pScharrScratch
	)
{
	unsigned int y, x;
	vx_uint16 *_tempBuf = (vx_uint16 *) pScharrScratch;
	vx_uint16 *trow0 = (vx_uint16 *) ALIGN16(_tempBuf + 1);
	vx_uint16 *trow1 = (vx_uint16 *) ALIGN16(trow0 + srcWidth + 2);

#if ENABLE_MSA
	v8i16 z = __builtin_msa_ldi_h(0);
	v8i16 c3 = __builtin_msa_ldi_h(3);
	v8i16 c10 = __builtin_msa_ldi_h(10);

	src += srcImageStrideInBytes;
	dst += dstImageStrideInBytes;
	for (y = 1; y < srcHeight - 1; y++)
	{
		const vx_uint8 *srow0 = y > 0 ? src - srcImageStrideInBytes : src;
		const vx_uint8 *srow1 = src;
		const vx_uint8 *srow2 = y < srcHeight - 1 ? src + srcImageStrideInBytes : src;
		vx_uint16 *drow = (vx_uint16 *) dst;

		// do vertical convolution
		x = 0;
		for (; x <= srcWidth - 8; x += 8)
		{
			v16i8 s0 = __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((srow0 + x), 0));
			v16i8 s1 = __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((srow1 + x), 0));
			v16i8 s2 = __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((srow2 + x), 0));
			v8i16 t0 = __builtin_msa_addv_h(__builtin_msa_mulv_h(__builtin_msa_addv_h((v8i16) s0, (v8i16) s2), c3),
											__builtin_msa_mulv_h((v8i16) s1, c10));
			v8i16 t1 = __builtin_msa_subv_h((v8i16) s2, (v8i16) s0);
			__builtin_msa_st_h(t0, (trow0 + x), 0);
			__builtin_msa_st_h(t1, (trow1 + x), 0);
		}
		// make border: is this really needed.
		//trow0[-1] = trow0[0]; trow0[srcWidth] = trow0[srcWidth - 1];
		//trow1[-1] = trow1[0]; trow1[srcWidth] = trow1[srcWidth - 1];

		// do horizontal convolution, interleave the results and store them to dst
		x = 0;
		for (; x <= srcWidth - 8; x += 8)
		{
			v16i8 s0 = __builtin_msa_ld_b((trow0 + x - 1), 0);
			v16i8 s1 = __builtin_msa_ld_b((trow0 + x + 1), 0);
			v16i8 s2 = __builtin_msa_ld_b((trow1 + x - 1), 0);
			v16i8 s3 = __builtin_msa_ld_b((trow1 + x), 0);
			v16i8 s4 = __builtin_msa_ld_b((trow1 + x + 1), 0);

			v8i16 t0 = __builtin_msa_subv_h((v8i16) s1, (v8i16) s0);
			v8i16 t1 = __builtin_msa_addv_h(__builtin_msa_mulv_h(__builtin_msa_addv_h((v8i16) s2, (v8i16) s4), c3),
											__builtin_msa_mulv_h((v8i16) s3, c10));
			v8i16 t2 = __builtin_msa_ilvr_h(t1, t0);
			t0 = __builtin_msa_ilvl_h(t1, t0);
			__builtin_msa_st_h(t2, (drow + x * 2), 0);
			__builtin_msa_st_h(t0, (drow + x * 2 + 8), 0);
		}
		src += srcImageStrideInBytes;
		dst += dstImageStrideInBytes;
	}
#else // C reference code for testing
	vx_int16 ops[] = {3, 10, 3, -3, -10, -3};
	src += srcImageStrideInBytes;
	dst += dstImageStrideInBytes;
	for (y = 1; y < srcHeight - 1; y++)
	{
		const vx_uint8 *srow0 = src - srcImageStrideInBytes;
		const vx_uint8 *srow1 = src;
		const vx_uint8 *srow2 = src + srcImageStrideInBytes;
		vx_int16 *drow = (vx_int16 *) dst;
		drow += 2;
		for (x = 1; x < srcWidth - 1; x++, drow += 2)
		{
			// calculate g_x
			drow[0] = (srow0[x + 1] * ops[0]) + (srow1[x + 1] * ops[1]) + (srow2[x + 1] * ops[2]) +
				  (srow0[x - 1] * ops[3]) + (srow1[x - 1] * ops[4]) + (srow2[x - 1] * ops[5]);
			drow[1] = (srow2[x - 1] * ops[0]) + (srow2[x] * ops[1]) + (srow2[x + 1] * ops[2]) +
				  (srow0[x - 1] * ops[3]) + (srow0[x] * ops[4]) + (srow0[x + 1] * ops[5]);
		}
		src += srcImageStrideInBytes;
		dst += dstImageStrideInBytes;
	}
#endif
}

int HafCpu_OpticalFlowPyrLK_XY_XY_Generic
	(
		vx_keypoint_t		newKeyPoint[],
		vx_float32			pyramidScale,
		vx_uint32			pyramidLevelCount,
		ago_pyramid_u8_t 	* oldPyramid,
		ago_pyramid_u8_t 	* newPyramid,
		vx_uint32			keyPointCount,
		vx_keypoint_t		oldKeyPoint[],
		vx_keypoint_t		newKeyPointEstimate[],
		vx_enum				termination,
		vx_float32			epsilon,
		vx_uint32			num_iterations,
		vx_bool				use_initial_estimate,
		vx_uint32			dataStrideInBytes,
		vx_uint8			* DataPtr,
		vx_int32			winsz
	)
{
	vx_size halfWin = (vx_size) (winsz >> 1);	//(winsz * 0.5f);
#if ENABLE_MSA
	v16i8 z = __builtin_msa_ldi_b(0);
	int qd_values[4] = {1 << (W_BITS - 1), 1 << (W_BITS - 1), 1 << (W_BITS - 1), 1 << (W_BITS - 1)};
	v4i32 qdelta_d = __builtin_msa_ld_w(&qd_values, 0);
	v4i32 qdelta = __builtin_msa_ldi_w(1 << (W_BITS - 5 - 1));
#endif
	// allocate matrix for I and dI
	vx_int16 Imat[256];				// enough to accomodate max win size of 15
	vx_int16 dIMat[256 * 2];
	vx_uint8 *pScharrScratch = DataPtr;
	vx_uint8 *pScratch = DataPtr + (oldPyramid[0].width + 2) * 4 + 64;
	ago_keypoint_t *pNextPtArray = (ago_keypoint_t *) (pScratch + (oldPyramid[0].width * oldPyramid[0].height * 4));

	for (int level = pyramidLevelCount - 1; level >= 0; level--)
	{
		int bBound;
		vx_uint32 dWidth = oldPyramid[level].width - 2;
		vx_uint32 dHeight = oldPyramid[level].height - 2;	// first and last row is not accounted
		vx_uint32 JWidth = newPyramid[level].width;
		vx_uint32 JHeight = newPyramid[level].height;
		vx_uint32 IStride = oldPyramid[level].strideInBytes, JStride = newPyramid[level].strideInBytes;
		vx_uint32 dStride = dataStrideInBytes >> 1;		//in #of elements
		vx_uint8 *SrcBase = oldPyramid[level].pImage;
		vx_uint8 *JBase = newPyramid[level].pImage;
		vx_int16 *DIBase = (vx_int16 *) pScratch;

		// calculate sharr derivatives Ix and Iy
		ComputeSharr(dataStrideInBytes, pScratch, oldPyramid[level].width, oldPyramid[level].height,
					 oldPyramid[level].strideInBytes, oldPyramid[level].pImage, pScharrScratch);
		float ptScale = (float) (pow(pyramidScale, level));

		// do the Lukas Kanade tracking for each feature point
		for (unsigned int pt = 0; pt < keyPointCount; pt++)
		{
			if (!oldKeyPoint[pt].tracking_status)
			{
				newKeyPoint[pt].x = oldKeyPoint[pt].x;
				newKeyPoint[pt].y = oldKeyPoint[pt].y;
				newKeyPoint[pt].strength = oldKeyPoint[pt].strength;
				newKeyPoint[pt].tracking_status = oldKeyPoint[pt].tracking_status;
				newKeyPoint[pt].scale = oldKeyPoint[pt].scale;
				newKeyPoint[pt].error = oldKeyPoint[pt].error;
				continue;
			}

			pt2f PrevPt, nextPt;
			bool bUseIE = false;
			PrevPt.x = oldKeyPoint[pt].x * ptScale;
			PrevPt.y = oldKeyPoint[pt].y * ptScale;
			if (level == pyramidLevelCount - 1)
			{
				if (use_initial_estimate)
				{
					nextPt.x = newKeyPointEstimate[pt].x * ptScale;
					nextPt.y = newKeyPointEstimate[pt].y * ptScale;
					bUseIE = true;
					newKeyPoint[pt].strength = newKeyPointEstimate[pt].strength;
					newKeyPoint[pt].tracking_status = newKeyPointEstimate[pt].tracking_status;
					newKeyPoint[pt].error = newKeyPointEstimate[pt].error;
				}
				else
				{
					pt_copy(nextPt, PrevPt);
					newKeyPoint[pt].tracking_status = oldKeyPoint[pt].tracking_status;
					newKeyPoint[pt].strength = oldKeyPoint[pt].strength;
				}
				pNextPtArray[pt].x = nextPt.x;
				pNextPtArray[pt].y = nextPt.y;
			}
			else
			{
				pNextPtArray[pt].x *= 2.0f;
				pNextPtArray[pt].y *= 2.0f;
				nextPt.x = pNextPtArray[pt].x;
				nextPt.y = pNextPtArray[pt].y;
			}

			if (!newKeyPoint[pt].tracking_status)
			{
				continue;
			}

			pt2i iprevPt, inextPt;
			PrevPt.x = PrevPt.x - halfWin;
			PrevPt.y = PrevPt.y - halfWin;
			nextPt.x = nextPt.x - halfWin;
			nextPt.y = nextPt.y - halfWin;

			iprevPt.x = (vx_int32) floor(PrevPt.x);
			iprevPt.y = (vx_int32) floor(PrevPt.y);
			// check if the point is out of bounds in the derivative image
			bBound = (iprevPt.x >> 31) | (iprevPt.x >= (vx_int32) (dWidth - winsz)) |
				 (iprevPt.y >> 31) | (iprevPt.y >= (vx_int32) (dHeight - winsz));
			if (bBound)
			{
				if (!level)
				{
					newKeyPoint[pt].x = (vx_int32) nextPt.x;
					newKeyPoint[pt].y = (vx_int32) nextPt.y;
					newKeyPoint[pt].tracking_status = 0;
					newKeyPoint[pt].error = 0;
				}
				continue;	// go to next point.
			}
			// calulate weights for interpolation
			float a = PrevPt.x - iprevPt.x;
			float b = PrevPt.y - iprevPt.y;
			float A11 = 0, A12 = 0, A22 = 0;
			int x, y;
			int iw00, iw01, iw10, iw11;
			if ((a == 0.0) && (b == 0.0))
			{
				// no need to do interpolation for the source and derivatives
				int x, y;
				for (y = 0; y < winsz; y++)
				{
					const unsigned char* src = SrcBase + (y + iprevPt.y) * IStride + iprevPt.x;
					const vx_int16* dsrc = DIBase + (y + iprevPt.y) * dStride + iprevPt.x * 2;

					vx_int16 *Iptr = &Imat[y * winsz];
					vx_int16 *dIptr = &dIMat[y * winsz * 2];
					x = 0;
#if ENABLE_MSA
					for (; x < winsz - 4; x += 4, dsrc += 8, dIptr += 8)
					{
						v16u8 v00, v01, v10, v11, v12;
						v16i8 *tbl = (v16i8 *) shuffleMasks;
						v16i8 mask0 = __builtin_msa_ld_b(tbl + 0, 0);
						v8i16 zero = __builtin_msa_ldi_h(0);
						int v00_array[4] = {*(const int *) (src + x), 0, 0, 0};
						v00 = (v16u8) __builtin_msa_ilvr_b(z, (v16i8) __builtin_msa_ld_w(v00_array, 0));
						v01 = (v16u8) __builtin_msa_ld_b(dsrc, 0);

						// iy3, iy2, ix3,ix2, iy1, iy0, ix1,ix0
						v10 = (v16u8) __builtin_msa_vshf_b(mask0, (v16i8) v01, (v16i8) v01);

						// iy3, iy2, iy1, iy0, ix3,ix2, ix1,ix0
						v10 = (v16u8) __builtin_msa_shf_w((v4i32) v10, 216);
						v11 = v10;
						// ix3,ix2, ix1,ix0, iy3, iy2, iy1, iy0
						v12 = (v16u8) __builtin_msa_shf_w((v4i32) v10, 78);
						v00 = (v16u8) __builtin_msa_slli_h((v8i16) v00, 5);

						// A121, A120
						v12 = (v16u8) __builtin_msa_dotp_s_w((v8i16) v12, (v8i16) v10);
						// A221, A220, A111, A110
						v10 = (v16u8) __builtin_msa_dotp_s_w((v8i16) v10, (v8i16) v11);

						A11 += (float) (((int32_t *) &v10)[0] + ((int32_t *) &v10)[1]);
						A22 += (float) (((int32_t *) &v10)[2] + ((int32_t *) &v10)[3]);
						A12 += (float) (((int32_t *) &v12)[0] + ((int32_t *) &v12)[1]);

						__builtin_msa_st_h((v8i16) v01, dIptr, 0);
						__builtin_msa_st_h((v8i16) v00, (Iptr + x), 0);
					}
					for (; x < winsz; x++, dsrc += 2, dIptr += 2)
					{
						int ival = (src[x] << 5);
						int ixval = dsrc[0];
						int iyval = dsrc[1];

						Iptr[x] = (short) ival;
						dIptr[0] = (short) ixval;
						dIptr[1] = (short) iyval;

						A11 += (float) (ixval * ixval);
						A12 += (float) (ixval * iyval);
						A22 += (float) (iyval * iyval);
					}
#else
					for (; x < winsz; x++, dsrc += 2, dIptr += 2)
					{
						int ival = (src[x] << 5);
						int ixval = dsrc[0];
						int iyval = dsrc[1];

						Iptr[x] = (short) ival;
						dIptr[0] = (short) ixval;
						dIptr[1] = (short) iyval;

						A11 += (float) (ixval * ixval);
						A12 += (float) (ixval * iyval);
						A22 += (float) (iyval * iyval);
					}
#endif
				}
				A11 *= FLT_SCALE;
				A12 *= FLT_SCALE;
				A22 *= FLT_SCALE;
			}
			else
			{
				int iw00 = (int) (((1.f - a) * (1.f - b) * (1 << W_BITS)) + 0.5);
				int iw01 = (int) ((a * (1.f - b) * (1 << W_BITS)) + 0.5);
				int iw10 = (int) (((1.f - a) * b * (1 << W_BITS)) + 0.5);
				int iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
#if ENABLE_MSA
				int qw0_values[4] = {iw00 + (iw01 << 16), iw00 + (iw01 << 16),
									iw00 + (iw01 << 16), iw00 + (iw01 << 16)};
				v4i32 qw0 = __builtin_msa_ld_w(&qw0_values, 0);
				int qw1_values[4] = {iw10 + (iw11 << 16), iw10 + (iw11 << 16),
									iw10 + (iw11 << 16), iw10 + (iw11 << 16)};
				v4i32 qw1 = __builtin_msa_ld_w(&qw1_values, 0);

				v4f32 qA11 = (v4f32) __builtin_msa_ldi_w(0);
				v4f32 qA12 = (v4f32) __builtin_msa_ldi_w(0);
				v4f32 qA22 = (v4f32) __builtin_msa_ldi_w(0);
#endif
				// extract the patch from the old image, compute covariation matrix of derivatives
				for (y = 0; y < winsz; y++)
				{
					const unsigned char* src = SrcBase + (y + iprevPt.y) * IStride + iprevPt.x;
					const vx_int16* dsrc = DIBase + (y + iprevPt.y) * dStride + iprevPt.x * 2;

					vx_int16* Iptr = &Imat[y * winsz];
					vx_int16* dIptr = &dIMat[y * winsz * 2];

					x = 0;
#if ENABLE_MSA
					for (; x <= winsz - 4; x += 4, dsrc += 4 * 2, dIptr += 4 * 2)
					{
						v16u8 v00, v01, v10, v11;
						v4u32 t0, t1;
						v8i16 zero = __builtin_msa_ldi_h(0);

						int v00_array[4] = {*(const int *) (src + x), 0, 0, 0};
						v00 = (v16u8) __builtin_msa_ilvr_b(z, (v16i8) __builtin_msa_ld_w(v00_array, 0));
						int v01_array[4] = {*(const int *) (src + x + 1), 0, 0, 0};
						v01 = (v16u8) __builtin_msa_ilvr_b(z, (v16i8) __builtin_msa_ld_w(v01_array, 0));
						int v10_array[4] = {*(const int *) (src + x + IStride), 0, 0, 0};
						v10 = (v16u8) __builtin_msa_ilvr_b(z, (v16i8) __builtin_msa_ld_w(v10_array, 0));
						int v11_array[4] = {*(const int *) (src + x + IStride + 1), 0, 0, 0};
						v11 = (v16u8) __builtin_msa_ilvr_b(z, (v16i8) __builtin_msa_ld_w(v11_array, 0));

						v8u16 temp11 = (v8u16) __builtin_msa_ilvr_h((v8i16) v01, (v8i16) v00);
						v8u16 temp21 = (v8u16) __builtin_msa_ilvr_h((v8i16) v11, (v8i16) v10);
						t0 = (v4u32) __builtin_msa_addv_w(__builtin_msa_dotp_s_w((v8i16) temp11, (v8i16) qw0),
														__builtin_msa_dotp_s_w((v8i16) temp21, (v8i16) qw1));

						t0 = (v4u32) __builtin_msa_srai_w(__builtin_msa_addv_w((v4i32) t0, qdelta), W_BITS - 5);

						v8i16 t0_s = __builtin_msa_sat_s_h((v8i16) t0, 15);
						v4i32 packed = (v4i32) __builtin_msa_pckev_h(t0_s, t0_s);
						*(long long *) (Iptr + x) = ((v2i64) packed)[0]; // store just the half

						v00 = (v16u8) __builtin_msa_ld_h(dsrc, 0);
						v01 = (v16u8) __builtin_msa_ld_h(dsrc + 2, 0);
						v10 = (v16u8) __builtin_msa_ld_h(dsrc + dStride, 0);
						v11 = (v16u8) __builtin_msa_ld_h(dsrc + dStride + 2, 0);

						temp11 = (v8u16) __builtin_msa_ilvr_h((v8i16) v01, (v8i16) v00);
						temp21 = (v8u16) __builtin_msa_ilvr_h((v8i16) v11, (v8i16) v10);
						t0 = (v4u32) __builtin_msa_addv_w(__builtin_msa_dotp_s_w((v8i16) temp11, (v8i16) qw0),
														__builtin_msa_dotp_s_w((v8i16) temp21, (v8i16) qw1));

						temp11 = (v8u16) __builtin_msa_ilvl_h((v8i16) v01, (v8i16) v00);
						temp21 = (v8u16) __builtin_msa_ilvl_h((v8i16) v11, (v8i16) v10);
						t1 = (v4u32) __builtin_msa_addv_w(__builtin_msa_dotp_s_w((v8i16) temp11, (v8i16) qw0),
														  __builtin_msa_dotp_s_w((v8i16) temp21, (v8i16) qw1));

						t0 = (v4u32) __builtin_msa_srai_w(__builtin_msa_addv_w((v4i32) t0, qdelta_d), W_BITS);
						t1 = (v4u32) __builtin_msa_srai_w(__builtin_msa_addv_w((v4i32) t1, qdelta_d), W_BITS);

						// Ix0 Iy0 Ix1 Iy1 ...
						t0_s = __builtin_msa_sat_s_h((v8i16) t0, 15);
						v8i16 t1_s = __builtin_msa_sat_s_h((v8i16) t1, 15);
						v00 = (v16u8) __builtin_msa_pckev_h(t1_s, t0_s);

						__builtin_msa_st_b((v16i8) v00, dIptr, 0);

						// Iy0 Iy1 Iy2 Iy3
						t0 = (v4u32) __builtin_msa_srai_w((v4i32) v00, 16);
						// Ix0 Ix1 Ix2 Ix3
						t1 = (v4u32) __builtin_msa_srai_w(__builtin_msa_slli_w((v4i32) v00, 16), 16);

						v4f32 fy = __builtin_msa_ffint_s_w((v4i32) t0);
						v4f32 fx = __builtin_msa_ffint_s_w((v4i32) t1);

						qA22 = __builtin_msa_fadd_w(qA22, __builtin_msa_fmul_w(fy, fy));
						qA12 = __builtin_msa_fadd_w(qA12, __builtin_msa_fmul_w(fx, fy));
						qA11 = __builtin_msa_fadd_w(qA11, __builtin_msa_fmul_w(fx, fx));
					}
					// do computation for remaining x if any
					for (; x < winsz; x++, dsrc += 2, dIptr += 2)
					{
						int ival = DESCALE(src[x] * iw00 + src[x + 1] * iw01 +
							   src[x + IStride] * iw10 + src[x + IStride + 1] * iw11, W_BITS - 5);
						int ixval = DESCALE(dsrc[0] * iw00 + dsrc[2] * iw01 +
							    dsrc[dStride] * iw10 + dsrc[dStride + 2] * iw11, W_BITS);
						int iyval = DESCALE(dsrc[1] * iw00 + dsrc[3] * iw01 + dsrc[dStride + 1] * iw10 +
							    dsrc[dStride + 3] * iw11, W_BITS);

						Iptr[x] = (short) ival;
						dIptr[0] = (short) ixval;
						dIptr[1] = (short) iyval;

						A11 += (float) (ixval * ixval);
						A12 += (float) (ixval * iyval);
						A22 += (float) (iyval * iyval);
					}
#else
					for (; x < winsz; x++, dsrc += 2, dIptr += 2)
					{
						int ival = DESCALE(src[x] * iw00 + src[x + 1] * iw01 +
							src[x + IStride] * iw10 + src[x + IStride + 1] * iw11, W_BITS - 5);
						int ixval = DESCALE(dsrc[0] * iw00 + dsrc[2] * iw01 +
							dsrc[dStride] * iw10 + dsrc[dStride + 2] * iw11, W_BITS);
						int iyval = DESCALE(dsrc[1] * iw00 + dsrc[3] * iw01 + dsrc[dStride + 1] * iw10 +
							dsrc[dStride + 3] * iw11, W_BITS);

						Iptr[x] = (short) ival;
						dIptr[0] = (short) ixval;
						dIptr[1] = (short) iyval;

						A11 += (float) (ixval * ixval);
						A12 += (float) (ixval * iyval);
						A22 += (float) (iyval * iyval);
					}
#endif
				}
#if ENABLE_MSA
				// add with MSA output
				if (winsz >= 4)
				{
					float DECL_ALIGN(16) A11buf[4] ATTR_ALIGN(16), A12buf[4] ATTR_ALIGN(16), A22buf[4] ATTR_ALIGN(16);
					__builtin_msa_st_w((v4i32) qA11, A11buf, 0);
					__builtin_msa_st_w((v4i32) qA12, A12buf, 0);
					__builtin_msa_st_w((v4i32) qA22, A22buf, 0);

					A11 += A11buf[0] + A11buf[1] + A11buf[2] + A11buf[3];
					A12 += A12buf[0] + A12buf[1] + A12buf[2] + A12buf[3];
					A22 += A22buf[0] + A22buf[1] + A22buf[2] + A22buf[3];
				}
#endif
				A11 *= FLT_SCALE;
				A12 *= FLT_SCALE;
				A22 *= FLT_SCALE;
			}

			float D = A11 * A22 - A12 * A12;
			float minEig = (A22 + A11 - std::sqrt((A11 - A22) * (A11 - A22) +
							4.f * A12 * A12)) / (2 * winsz * winsz);

			if (minEig < 1.0e-04F || D < 1.0e-07F)
			{
				if (!level)
				{
					newKeyPoint[pt].x = (vx_int32) nextPt.x;
					newKeyPoint[pt].y = (vx_int32) nextPt.y;
					newKeyPoint[pt].tracking_status = 0;
					newKeyPoint[pt].error = 0;
				}
				continue;
			}
			D = 1.f / D;
			float prevDelta_x = 0.f, prevDelta_y = 0.f;
			float delta_dx = 0.f, delta_dy = 0.f;
			unsigned int j = 0;
			while (j < num_iterations || termination == VX_TERM_CRITERIA_EPSILON)
			{
				inextPt.x = (vx_int32) floor(nextPt.x);
				inextPt.y = (vx_int32) floor(nextPt.y);
				bBound = (inextPt.x >> 31) | (inextPt.x >= (vx_int32) (JWidth - winsz)) |
					 (inextPt.y >> 31) | (inextPt.y >= (vx_int32) (JHeight - winsz));
				if (bBound)
				{
					if (!level)
					{
						newKeyPoint[pt].tracking_status = 0;
						newKeyPoint[pt].error = 0;
					}
					break;	// go to next point.
				}
				a = nextPt.x - inextPt.x;
				b = nextPt.y - inextPt.y;
				iw00 = (int) (((1.f - a) * (1.f - b) * (1 << W_BITS)) + 0.5);
				iw01 = (int) ((a * (1.f - b) * (1 << W_BITS)) + 0.5);
				iw10 = (int) (((1.f - a) * b * (1 << W_BITS)) + 0.5);
				iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
				double ib1 = 0, ib2 = 0;
				float b1, b2;
#if ENABLE_MSA
				int qw0_values[4] = {iw00 + (iw01 << 16), iw00 + (iw01 << 16),
									iw00 + (iw01 << 16), iw00 + (iw01 << 16)};
				v4u32 qw0 = (v4u32) __builtin_msa_ld_w(&qw0_values, 0);

				int qw1_values[4] = {iw10 + (iw11 << 16), iw10 + (iw11 << 16),
									iw10 + (iw11 << 16), iw10 + (iw11 << 16)};
				v4u32 qw1 = (v4u32) __builtin_msa_ld_w(&qw1_values, 0);

				v4f32 qb0 = (v4f32) __builtin_msa_ldi_w(0);
				v4f32 qb1 = (v4f32) __builtin_msa_ldi_w(0);
#endif
				for (y = 0; y < winsz; y++)
				{
					const unsigned char *Jptr = JBase + (y + inextPt.y) * JStride + inextPt.x;
					vx_int16 *Iptr = &Imat[y * winsz];
					vx_int16 *dIptr = &dIMat[y * winsz * 2];

					x = 0;
#if ENABLE_MSA
					for (; x <= winsz - 8; x += 8, dIptr += 8 * 2)
					{
						v8u16 diff0 = (v8u16) __builtin_msa_ld_h((Iptr + x), 0);
						v8u16 diff1;

						v16u8 v00 = (v16u8) __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((Jptr + x), 0));
						v16u8 v01 = (v16u8) __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((Jptr + x + 1), 0));
						v16u8 v10 = (v16u8) __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((Jptr + x + JStride), 0));
						v16u8 v11 = (v16u8) __builtin_msa_ilvr_b((v16i8) z, (v16i8) __builtin_msa_ld_d((Jptr + x + JStride + 1), 0));

						v8u16 temp11 = (v8u16) __builtin_msa_ilvr_h((v8i16) v01, (v8i16) v00);
						v8u16 temp21 = (v8u16) __builtin_msa_ilvr_h((v8i16) v11, (v8i16) v10);
						v4u32 t0 = (v4u32) __builtin_msa_addv_w(__builtin_msa_dotp_s_w((v8i16) temp11, (v8i16) qw0),
																__builtin_msa_dotp_s_w((v8i16) temp21, (v8i16) qw1));

						temp11 = (v8u16) __builtin_msa_ilvl_h((v8i16) v01, (v8i16) v00);
						temp21 = (v8u16) __builtin_msa_ilvl_h((v8i16) v11, (v8i16) v10);
						v4u32 t1 = (v4u32) __builtin_msa_addv_w(__builtin_msa_dotp_s_w((v8i16) temp11, (v8i16) qw0),
																__builtin_msa_dotp_s_w((v8i16) temp21, (v8i16) qw1));

						t0 = (v4u32) __builtin_msa_srai_w(__builtin_msa_addv_w((v4i32) t0, qdelta), W_BITS - 5);
						t1 = (v4u32) __builtin_msa_srai_w(__builtin_msa_addv_w((v4i32) t1, qdelta), W_BITS - 5);
						v8i16 t0_s = __builtin_msa_sat_s_h((v8i16) t0, 15);
						v8i16 t1_s = __builtin_msa_sat_s_h((v8i16) t1, 15);
						diff0 = (v8u16) __builtin_msa_subv_h((v8i16) __builtin_msa_pckev_h(t1_s, t0_s),
															(v8i16) __builtin_msa_sat_s_h((v8i16) diff0, 15));

						diff1 = (v8u16) __builtin_msa_ilvl_h((v8i16) diff0, (v8i16) diff0);
						// It0 It0 It1 It1 ...
						diff0 = (v8u16) __builtin_msa_ilvr_h((v8i16) diff0, (v8i16) diff0);

						// Ix0 Iy0 Ix1 Iy1 ...
						v00 = (v16u8) __builtin_msa_ld_h(dIptr, 0);
						v01 = (v16u8) __builtin_msa_ld_h((dIptr + 8), 0);
						v10 = (v16u8) __builtin_msa_mulv_h((v8i16) v00, (v8i16) diff0);

						v4i32 temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) z, (v8i16) v00);
						v4i32 temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) z, (v8i16) v00);
						v4i32 diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) z, (v8i16) diff0);
						v4i32 diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) z, (v8i16) diff0);
						temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
						temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
						v11 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

						v00 = (v16u8) __builtin_msa_ilvr_h((v8i16) v11, (v8i16) v10);
						v10 = (v16u8) __builtin_msa_ilvl_h((v8i16) v11, (v8i16) v10);

						qb0 = __builtin_msa_fadd_w(qb0, __builtin_msa_ffint_s_w((v4i32) v00));
						qb1 = __builtin_msa_fadd_w(qb1, __builtin_msa_ffint_s_w((v4i32) v10));

						v10 = (v16u8) __builtin_msa_mulv_h((v8i16) v01, (v8i16) diff1);

						temp_l = (v4i32) __builtin_msa_ilvl_h((v8i16) z, (v8i16) v01);
						temp_r = (v4i32) __builtin_msa_ilvr_h((v8i16) z, (v8i16) v01);
						diff_l = (v4i32) __builtin_msa_ilvl_h((v8i16) z, (v8i16) diff1);
						diff_r = (v4i32) __builtin_msa_ilvr_h((v8i16) z, (v8i16) diff1);
						temp_l = __builtin_msa_dotp_s_w((v8i16) temp_l, (v8i16) diff_l);
						temp_r = __builtin_msa_dotp_s_w((v8i16) temp_r, (v8i16) diff_r);
						v11 = (v16u8) __builtin_msa_pckod_h((v8i16) temp_l, (v8i16) temp_r);

						v00 = (v16u8) __builtin_msa_ilvr_h((v8i16) v11, (v8i16) v10);
						v10 = (v16u8) __builtin_msa_ilvl_h((v8i16) v11, (v8i16) v10);

						qb0 = __builtin_msa_fadd_w(qb0, __builtin_msa_ffint_s_w((v4i32) v00));
						qb1 = __builtin_msa_fadd_w(qb1, __builtin_msa_ffint_s_w((v4i32) v10));
					}
					for (; x < winsz; x++, dIptr += 2)
					{
						int diff = DESCALE(Jptr[x] * iw00 + Jptr[x + 1] * iw01 +
									Jptr[x + JStride] * iw10 + Jptr[x + JStride + 1] * iw11, W_BITS - 5);
						diff -= Iptr[x];
						ib1 += (float) (diff * dIptr[0]);
						ib2 += (float) (diff * dIptr[1]);
					}
#else
					for (; x < winsz; x++, dIptr += 2)
					{
						int diff = DESCALE(Jptr[x] * iw00 + Jptr[x + 1] * iw01 +
									Jptr[x + JStride] * iw10 + Jptr[x + JStride + 1] * iw11, W_BITS - 5);
						diff -= Iptr[x];
						ib1 += (float) (diff * dIptr[0]);
						ib2 += (float) (diff * dIptr[1]);
					}
#endif
				}
#if ENABLE_MSA
				if (winsz >= 8)
				{
					float DECL_ALIGN(16) bbuf[4] ATTR_ALIGN(16);
					__builtin_msa_st_w((v4i32) __builtin_msa_fadd_w(qb0, qb1), bbuf, 0);
					ib1 += bbuf[0] + bbuf[2];
					ib2 += bbuf[1] + bbuf[3];
				}
#endif
				b1 = (float) (ib1 * FLT_SCALE);
				b2 = (float) (ib2 * FLT_SCALE);
				// calculate delta
				float delta_x = (float) ((A12 * b2 - A22 * b1) * D);
				float delta_y = (float) ((A12 * b1 - A11 * b2) * D);
				// add to nextPt
				nextPt.x += delta_x;
				nextPt.y += delta_y;
				if ((delta_x * delta_x + delta_y * delta_y) <= epsilon &&
				    (termination == VX_TERM_CRITERIA_EPSILON || termination == VX_TERM_CRITERIA_BOTH))
				{
					break;
				}
				if (j > 0 && abs(delta_x + prevDelta_x) < 0.01 && abs(delta_y + prevDelta_y) < 0.01)
				{
					delta_dx = delta_x * 0.5f;
					delta_dy = delta_y * 0.5f;
					break;
				}
				prevDelta_x = delta_x;
				prevDelta_y = delta_y;
				j++;
			}
			if (!level)
			{
				newKeyPoint[pt].x = (vx_int32) (nextPt.x + halfWin - delta_dx + 0.5f);
				newKeyPoint[pt].y = (vx_int32) (nextPt.y + halfWin - delta_dy + 0.5f);
			}
			else
			{
				pNextPtArray[pt].x = (nextPt.x + halfWin - delta_dx);
				pNextPtArray[pt].y = (nextPt.y + halfWin - delta_dy);
			}
		}
	}
	return AGO_SUCCESS;
}
