#include "ago_internal.h"

VX_API_ENTRY vx_node VX_API_CALL vxNonLinearFilterNode(vx_graph graph, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_scalar func = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &function);

    vx_reference params[] = {
        (vx_reference)func,
        (vx_reference)input,
        (vx_reference)mask,
        (vx_reference)output,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
        VX_KERNEL_NON_LINEAR_FILTER,
        params,
        dimof(params));


    vxReleaseScalar(&func);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxBilateralFilterNode(vx_graph graph, vx_tensor src, vx_int32 diameter, vx_float32
    sigmaSpace, vx_float32 sigmaValues, vx_tensor dst)
{
    vx_scalar dia = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &diameter);
    vx_scalar sSpa = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &sigmaSpace);
    vx_scalar sVal = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &sigmaValues);
    vx_reference params[] = {
            (vx_reference)src,
            (vx_reference)dia,
            (vx_reference)sSpa,
            (vx_reference)sVal,
            (vx_reference)dst,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
                                           VX_KERNEL_BILATERAL_FILTER,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&dia);
    vxReleaseScalar(&sSpa);
    vxReleaseScalar(&sVal);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxScalarOperationNode(vx_graph graph, vx_enum operation, vx_scalar a, vx_scalar b, vx_scalar output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar op = vxCreateScalar(context, VX_TYPE_ENUM, &operation);
    vx_reference params[] = {
            (vx_reference)op,
            (vx_reference)a,
            (vx_reference)b,
            (vx_reference)output,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
                                           VX_KERNEL_SCALAR_OPERATION,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&op);
    return node;
    */
    return NULL;

}

VX_API_ENTRY vx_node VX_API_CALL vxHoughLinesPNode(vx_graph graph, vx_image input, const vx_hough_lines_p_t *params_hough_lines, vx_array lines_array, vx_scalar num_lines)
{
    vx_array params_hough_lines_array = vxCreateArray(vxGetContext((vx_reference)graph), VX_TYPE_HOUGH_LINES_PARAMS, 1);
    vxAddArrayItems(params_hough_lines_array, 1, params_hough_lines, sizeof(vx_hough_lines_p_t));
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)params_hough_lines_array,
        (vx_reference)lines_array,
        (vx_reference)num_lines,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
        VX_KERNEL_HOUGH_LINES_P,
        params,
        dimof(params));
    vxReleaseArray(&params_hough_lines_array);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxMatchTemplateNode(vx_graph graph, vx_image src, vx_image templateImage, vx_enum matchingMethod, vx_image output)
{
    vx_scalar scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &matchingMethod);
    vx_reference params[] = {
            (vx_reference)src,
            (vx_reference)templateImage,
            (vx_reference)scalar,
            (vx_reference)output,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
                                           VX_KERNEL_MATCH_TEMPLATE,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&scalar);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxNonMaxSuppressionNode(vx_graph graph, vx_image input, vx_image mask, vx_int32 win_size, vx_image output)
{
    vx_scalar wsize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &win_size);
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)mask,
        (vx_reference)wsize,
        (vx_reference)output,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
        VX_KERNEL_NON_MAX_SUPPRESSION,
        params,
        dimof(params));
    vxReleaseScalar(&wsize);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTableLookupNode(vx_graph graph, vx_tensor input, vx_lut lut, vx_tensor output)
{
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)lut,
        (vx_reference)output,
    };

    //return vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_TABLE_LOOKUP, params, dimof(params));
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorMatrixMultiplyNode(vx_graph graph, vx_tensor input1, vx_tensor input2, vx_tensor input3,
    const vx_tensor_matrix_multiply_params_t *matrix_multiply_params, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar transpose_src1 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input1);
    vx_scalar transpose_src2 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input2);
    vx_scalar transpose_src3 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input3);
    /*
    vx_reference params[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)input3,
        (vx_reference)transpose_src1,
        (vx_reference)transpose_src2,
        (vx_reference)transpose_src3,
        (vx_reference)output,
    };
    vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_MATRIX_MULTIPLY, params, dimof(params));

    vxReleaseScalar(&transpose_src1);
    vxReleaseScalar(&transpose_src2);
    vxReleaseScalar(&transpose_src3);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorMultiplyNode(vx_graph graph, vx_tensor input1, vx_tensor input2, vx_scalar scale, vx_enum overflow_policy,
        vx_enum rounding_policy, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar spolicy = vxCreateScalar(context, VX_TYPE_ENUM, &overflow_policy);
    vx_scalar rpolicy = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    vx_reference params[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)scale,
        (vx_reference)spolicy,
        (vx_reference)rpolicy,
        (vx_reference)output,
    };

    vx_node node = NULL;
    /*
    node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_MULTIPLY, params, dimof(params));

    vxReleaseScalar(&spolicy);
    vxReleaseScalar(&rpolicy);
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorSubtractNode(vx_graph graph, vx_tensor input1, vx_tensor input2, vx_enum policy, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar spolicy = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)spolicy,
        (vx_reference)output,
    };

    vx_node node = NULL;
    /*
    node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_SUBTRACT, params, dimof(params));
    */
    vxReleaseScalar(&spolicy);
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorAddNode(vx_graph graph, vx_tensor input1, vx_tensor input2, vx_enum policy, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar spolicy = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    vx_reference params[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)spolicy,
        (vx_reference)output,
    };

    vx_node node = NULL;

    //node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_ADD, params, dimof(params));

    vxReleaseScalar(&spolicy);
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorConvertDepthNode(vx_graph graph, vx_tensor input, vx_enum policy, vx_scalar norm, vx_scalar offset, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar overflow_policy_sc = vxCreateScalar(context, VX_TYPE_ENUM, &policy);

    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)overflow_policy_sc,
        (vx_reference)norm,
        (vx_reference)offset,
        (vx_reference)output,
    };

    //vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_CONVERT_DEPTH, params, dimof(params));

    vxReleaseScalar(&overflow_policy_sc);
    //return node;
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTransposeNode(vx_graph graph, vx_tensor input, vx_tensor output, vx_size dimension0, vx_size dimension1)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar dim1 = vxCreateScalar(context, VX_TYPE_SIZE, &dimension0);
    vx_scalar dim2 = vxCreateScalar(context, VX_TYPE_SIZE, &dimension1);

    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)dim1,
        (vx_reference)dim2,
        (vx_reference)output,
    };

    //vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_TENSOR_TRANSPOSE, params, dimof(params));

    vxReleaseScalar(&dim1);
    vxReleaseScalar(&dim2);
    //return node;
    return NULL;
}

vx_node VX_API_CALL vxHOGCellsNode(vx_graph graph, vx_image input, vx_int32 cell_width, vx_int32 cell_height, vx_int32 num_bins, vx_tensor magnitudes, vx_tensor bins)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar cell_width_scalar = vxCreateScalar(context, VX_TYPE_INT32, &cell_width);
    vx_scalar cell_height_scalar = vxCreateScalar(context, VX_TYPE_INT32, &cell_height);
    vx_scalar num_bins_scalar = vxCreateScalar(context, VX_TYPE_INT32, &num_bins);

    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)cell_width_scalar,
        (vx_reference)cell_height_scalar,
        (vx_reference)num_bins_scalar,
        (vx_reference)magnitudes,
        (vx_reference)bins,
    };
    //vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_HOG_CELLS, params, dimof(params));

    vxReleaseScalar(&cell_width_scalar);
    vxReleaseScalar(&cell_height_scalar);
    vxReleaseScalar(&num_bins_scalar);

    //return node;
    return NULL;
}

vx_node VX_API_CALL vxHOGFeaturesNode(vx_graph graph, vx_image input, vx_tensor magnitudes, vx_tensor bins, const vx_hog_t *params, vx_size hog_param_size, vx_tensor features)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array hog_param = vxCreateArray(vxGetContext((vx_reference)graph), VX_TYPE_HOG_PARAMS, 1);
    vxAddArrayItems(hog_param, 1, params, hog_param_size*sizeof(vx_hog_t));
    vx_scalar hog_param_size_scalar = vxCreateScalar(context, VX_TYPE_INT32, &hog_param_size);

    vx_reference param[] = {
        (vx_reference)input,
        (vx_reference)magnitudes,
        (vx_reference)bins,
        (vx_reference)hog_param,
        (vx_reference)hog_param_size_scalar,
        (vx_reference)features,
    };
    //vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_HOG_FEATURES, param, dimof(param));

    vxReleaseScalar(&hog_param_size_scalar);
    vxReleaseArray(&hog_param);

    //return node;
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxLBPNode(vx_graph graph,
    vx_image in, vx_enum format, vx_int8 kernel_size, vx_image out)
{
    vx_scalar sformat = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &format);
    vx_scalar ksize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT8, &kernel_size);
    vx_reference params[] = {
            (vx_reference)in,
            (vx_reference)sformat,
            (vx_reference)ksize,
            (vx_reference)out,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
        VX_KERNEL_LBP,
        params,
        dimof(params));
    */
    vxReleaseScalar(&sformat);
    vxReleaseScalar(&ksize);
    //return node;
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxMaxNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
            (vx_reference) in1,
            (vx_reference) in2,
            (vx_reference) out,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
                                           VX_KERNEL_MAX,
                                           params,
                                           dimof(params));
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxMinNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference params[] = {
            (vx_reference) in1,
            (vx_reference) in2,
            (vx_reference) out,
    };
    /*
    vx_node node = vxCreateNodeByStructure(graph,
                                                           VX_KERNEL_MIN,
                                           params,
                                           dimof(params));
    return node;
    */
    return NULL;
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianPyramidNode(vx_graph graph, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)laplacian,
        (vx_reference)output,
    };
    return NULL;
    /*
    return vxCreateNodeByStructure(graph,
                                   VX_KERNEL_LAPLACIAN_PYRAMID,
                                   params,
                                   dimof(params));
                                   */
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianReconstructNode(vx_graph graph, vx_pyramid laplacian, vx_image input,
                                       vx_image output)
{
    vx_reference params[] = {
        (vx_reference)laplacian,
        (vx_reference)input,
        (vx_reference)output,
    };
    return NULL;
    /*
    return vxCreateNodeByStructure(graph,
                                   VX_KERNEL_LAPLACIAN_RECONSTRUCT,
                                   params,
                                   dimof(params));
                                   */
}


