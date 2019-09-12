#include "ago_internal.h"
#include "mips_internal.h"

int HafCpu_Not_U8_U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Not_U1_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Not_U1_U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Lut_U8_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLut
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Threshold_U8_U8_Binary
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      threshold
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Threshold_U8_U8_Range
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      lower,
		vx_uint8      upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Threshold_U1_U8_Range
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      lower,
		vx_uint8      upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ThresholdNot_U8_U8_Binary
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      threshold
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ThresholdNot_U8_U8_Range
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      lower,
		vx_uint8      upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ThresholdNot_U1_U8_Binary
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      threshold
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ThresholdNot_U1_U8_Range
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8      lower,
		vx_uint8      upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorDepth_U8_S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorDepth_U8_S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorDepth_S16_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int32      shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Add_U8_U8U8_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_U8_U8U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_U8_U8U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_U8_U8U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_U8_U8U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_And_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_And_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_And_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Or_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Or_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Or_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Or_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Or_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xor_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xor_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xor_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xor_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xor_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nand_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Nor_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U8_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U8_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U1_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U1_U8U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Xnor_U1_U1U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_AccumulateWeighted_U8_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_float32    alpha
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_U8U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_U8U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_U8U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_U8U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_U8U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Add_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Accumulate_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_S16U8_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16U8_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16U8_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16U8_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16U8_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_uint8    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_AccumulateSquared_S16_S16U8_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint32     shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_U8S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_U8S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Add_S16_S16S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_S16S16_Wrap
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sub_S16_S16S16_Sat
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16S16_Wrap_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16S16_Wrap_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16S16_Sat_Trunc
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Mul_S16_S16S16_Sat_Round
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_int16    * pSrcImage1,
		vx_uint32     srcImage1StrideInBytes,
		vx_int16    * pSrcImage2,
		vx_uint32     srcImage2StrideInBytes,
		vx_float32    scale
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Magnitude_S16_S16S16
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pMagImage,
		vx_uint32     magImageStrideInBytes,
		vx_int16    * pGxImage,
		vx_uint32     gxImageStrideInBytes,
		vx_int16    * pGyImage,
		vx_uint32     gyImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Phase_U8_S16S16
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pPhaseImage,
		vx_uint32     phaseImageStrideInBytes,
		vx_int16    * pGxImage,
		vx_uint32     gxImageStrideInBytes,
		vx_int16    * pGyImage,
		vx_uint32     gyImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MemSet_U24
	(
		vx_size       count,
		vx_uint8    * pDstBuf,
		vx_uint32     value
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_BufferCopyDisperseInDst
	(
		vx_uint32	  dstWidth,
		vx_uint32	  dstHeight,
		vx_uint32	  pixelSizeInBytes,
		vx_uint8	* pDstImage,
		vx_uint32	  dstImageStrideYInBytes,
		vx_uint32	  dstImageStrideXInBytes,
		vx_uint8	* pSrcImage,
		vx_uint32	  srcImageStrideYInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_BufferCopyDisperseInSrc
	(
		vx_uint32	  dstWidth,
		vx_uint32	  dstHeight,
		vx_uint32	  pixelSizeInBytes,
		vx_uint8	* pDstImage,
		vx_uint32	  dstImageStrideYInBytes,
		vx_uint8	* pSrcImage,
		vx_uint32	  srcImageStrideYInBytes,
		vx_uint32	  srcImageStrideXInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ChannelCopy_U8_U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ChannelCopy_U1_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ChannelCopy_U1_U1
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ChannelExtract_U8U8U8_U32
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage0,
		vx_uint8    * pDstImage1,
		vx_uint8    * pDstImage2,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ChannelExtract_U8U8U8U8_U32
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage0,
		vx_uint8    * pDstImage1,
		vx_uint8    * pDstImage2,
		vx_uint8    * pDstImage3,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGB_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGB_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGB_YUYV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGBX_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGBX_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_RGBX_YUYV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_YUV4_RGB
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_YUV4_RGBX
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IYUV_RGB
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IYUV_RGBX
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_FormatConvert_IYUV_YUYV
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_NV12_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstLumaImage,
		vx_uint32     dstLumaImageStrideInBytes,
		vx_uint8    * pDstChromaImage,
		vx_uint32     dstChromaImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_NV12_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstLumaImage,
		vx_uint32     dstLumaImageStrideInBytes,
		vx_uint8    * pDstChromaImage,
		vx_uint32     dstChromaImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_FormatConvert_NV12_UYVY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstLumaImage,
		vx_uint32     dstLumaImageStrideInBytes,
		vx_uint8    * pDstChromaImage,
		vx_uint32     dstChromaImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_FormatConvert_NV12_YUYV
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstLumaImage,
		vx_uint32     dstLumaImageStrideInBytes,
		vx_uint8    * pDstChromaImage,
		vx_uint32     dstChromaImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_Y_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstYImage,
		vx_uint32     dstYImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_U_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_U_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_V_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_V_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IU_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IU_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IV_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IV_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IUV_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_IUV_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstUImage,
		vx_uint32     dstUImageStrideInBytes,
		vx_uint8    * pDstVImage,
		vx_uint32     dstVImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_UV12_RGB
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImageChroma,
		vx_uint32     dstImageChromaStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ColorConvert_UV12_RGBX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImageChroma,
		vx_uint32     dstImageChromaStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ScaleGaussianHalf_U8_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8    * pLocalData
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Convolve_U8_U8_MxN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_uint32     convolutionWidth,
		vx_uint32     convolutionHeight,
		vx_int32      shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Convolve_S16_U8_MxN
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_int16    * convMatrix,
		vx_uint32     convolutionWidth,
		vx_uint32     convolutionHeight,
		vx_int32      shift
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_SobelMagnitude_S16_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstMagImage,
		vx_uint32     dstMagImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_SobelPhase_U8_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstPhaseImage,
		vx_uint32     dstPhaseImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8	* pScratch
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_SobelMagnitudePhase_S16U8_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstMagImage,
		vx_uint32     dstMagImageStrideInBytes,
		vx_uint8    * pDstPhaseImage,
		vx_uint32     dstPhaseImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sobel_S16_U8_3x3_GX
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstGxImage,
		vx_uint32     dstGxImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8	* pScratch
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Sobel_S16_U8_3x3_GY
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_int16    * pDstGyImage,
		vx_uint32     dstGyImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes,
		vx_uint8	* pScratch
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Dilate_U1_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Erode_U1_U8_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Dilate_U1_U1_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Erode_U1_U1_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Dilate_U8_U1_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Erode_U8_U1_3x3
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint8    * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_3x3_L1NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper,
		vx_uint8			 * pScratch
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_3x3_L2NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_5x5_L1NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_5x5_L2NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_7x7_L1NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannySobelSuppThreshold_U8XY_U8_7x7_L2NORM
	(
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[],
		vx_uint32            * pxyStackTop,
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDst,
		vx_uint32              dstStrideInBytes,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		vx_uint16               hyst_lower,
		vx_uint16               hyst_upper
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Remap_U8_U8_Nearest
	(
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDstImage,
		vx_uint32              dstImageStrideInBytes,
		vx_uint32              srcWidth,
		vx_uint32              srcHeight,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		ago_coord2d_ushort_t * pMap,
		vx_uint32              mapStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Remap_U8_U8_Nearest_Constant
	(
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDstImage,
		vx_uint32              dstImageStrideInBytes,
		vx_uint32              srcWidth,
		vx_uint32              srcHeight,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		ago_coord2d_ushort_t * pMap,
		vx_uint32              mapStrideInBytes,
		vx_uint8               border
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Remap_U8_U8_Bilinear
	(
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDstImage,
		vx_uint32              dstImageStrideInBytes,
		vx_uint32              srcWidth,
		vx_uint32              srcHeight,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		ago_coord2d_ushort_t * pMap,
		vx_uint32              mapStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Remap_U8_U8_Bilinear_Constant
	(
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDstImage,
		vx_uint32              dstImageStrideInBytes,
		vx_uint32              srcWidth,
		vx_uint32              srcHeight,
		vx_uint8             * pSrcImage,
		vx_uint32              srcImageStrideInBytes,
		ago_coord2d_ushort_t * pMap,
		vx_uint32              mapStrideInBytes,
		vx_uint8               border
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_WarpPerspective_U8_U8_Bilinear
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
		vx_uint8				 * pLocalData
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_WarpPerspective_U8_U8_Bilinear_Constant
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
		vx_uint8				 * pLocalData
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_ScaleImage_U8_U8_Nearest_Constant
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_FastCornerMerge_XY_XY
	(
		vx_uint32       capacityOfDstCorner,
		vx_keypoint_t   dstCorner[],
		vx_uint32     * pDstCornerCount,
		vx_uint32		numSrcCornerBuffers,
		vx_keypoint_t * pSrcCorners[],
		vx_uint32       numSrcCorners[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_CannyEdgeTrace_U8_U8
	(
		vx_uint32              dstWidth,
		vx_uint32              dstHeight,
		vx_uint8             * pDstImage,
		vx_uint32              dstImageStrideInBytes,
		vx_uint32              capacityOfXY,
		ago_coord2d_ushort_t   xyStack[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_IntegralImage_U32_U8
	(
		vx_uint32     dstWidth,
		vx_uint32     dstHeight,
		vx_uint32   * pDstImage,
		vx_uint32     dstImageStrideInBytes,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Histogram_DATA_U8
	(
		vx_uint32     dstHist[],
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
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
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MeanStdDev_DATA_U8
	(
		vx_float32  * pSum,
		vx_float32  * pSumOfSquared,
		vx_uint32     srcWidth,
		vx_uint32     srcHeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_Equalize_DATA_DATA
	(
		vx_uint8    * pLut,
		vx_uint32     numPartitions,
		vx_uint32   * pPartSrcHist[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_HistogramMerge_DATA_DATA
	(
		vx_uint32     dstHist[],
		vx_uint32     numPartitions,
		vx_uint32   * pPartSrcHist[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MeanStdDevMerge_DATA_DATA
	(
		vx_float32  * mean,
		vx_float32  * stddev,
		vx_uint32	  totalSampleCount,
		vx_uint32     numPartitions,
		vx_float32    partSum[],
		vx_float32    partSumOfSquared[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMax_DATA_U8
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     srcWidth,
		vx_uint32     srcWeight,
		vx_uint8    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMax_DATA_S16
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     srcWidth,
		vx_uint32     srcWeight,
		vx_int16    * pSrcImage,
		vx_uint32     srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxMerge_DATA_DATA
	(
		vx_int32    * pDstMinValue,
		vx_int32    * pDstMaxValue,
		vx_uint32     numDataPartitions,
		vx_int32      srcMinValue[],
		vx_int32      srcMaxValue[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_None_Count_Min
	(
		vx_uint32          * pMinLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_None_Count_Max
	(
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_None_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_Min_Count_Min
	(
		vx_uint32          * pMinLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_Min_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_Max_Count_Max
	(
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_Max_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_U8DATA_Loc_MinMax_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_uint8           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_None_Count_Min
	(
		vx_uint32          * pMinLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_None_Count_Max
	(
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_None_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_Min_Count_Min
	(
		vx_uint32          * pMinLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_Min_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_Max_Count_Max
	(
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_Max_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLoc_DATA_S16DATA_Loc_MinMax_Count_MinMax
	(
		vx_uint32          * pMinLocCount,
		vx_uint32          * pMaxLocCount,
		vx_uint32            capacityOfMinLocList,
		vx_coordinates2d_t   minLocList[],
		vx_uint32            capacityOfMaxLocList,
		vx_coordinates2d_t   maxLocList[],
		vx_int32           * pDstMinValue,
		vx_int32           * pDstMaxValue,
		vx_uint32            numDataPartitions,
		vx_int32             srcMinValue[],
		vx_int32             srcMaxValue[],
		vx_uint32            srcWidth,
		vx_uint32            srcHeight,
		vx_int16           * pSrcImage,
		vx_uint32            srcImageStrideInBytes
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}

int HafCpu_MinMaxLocMerge_DATA_DATA
	(
		vx_uint32          * pDstLocCount,
		vx_uint32            capacityOfDstLocList,
		vx_coordinates2d_t   dstLocList[],
		vx_uint32            numDataPartitions,
		vx_uint32            partLocCount[],
		vx_coordinates2d_t * partLocList[]
	){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}


// helper functions for phase
float HafCpu_FastAtan2_deg
(
	vx_int16	  Gx,
	vx_int16      Gy
){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}


float HafCpu_FastAtan2_rad
(
	vx_int16	  Gx,
	vx_int16      Gy
){
	return AGO_ERROR_HAFCPU_NOT_IMPLEMENTED;
}
