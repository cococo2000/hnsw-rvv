#pragma once
#include "hnswlib.h"

#if defined(USE_RVV)
#include <riscv_vector.h>
static float
L2SqrSIMDExtRVV(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    // float *pVect1 = (float *) pVect1v;
    // float *pVect2 = (float *) pVect2v;
    // size_t qty = *((size_t *) qty_ptr);
    
    // // float sum = 0.0f;
    // // size_t i = 0;
    // // while (i < qty) {
    // //     size_t vl = __riscv_vsetvl_e32m1(qty - i);
    // //     vfloat32m1_t v1 = __riscv_vle32_v_f32m1(pVect1 + i, vl);
    // //     vfloat32m1_t v2 = __riscv_vle32_v_f32m1(pVect2 + i, vl);
    // //     vfloat32m1_t diff = __riscv_vfsub_vv_f32m1(v1, v2, vl);
    // //     vfloat32m1_t sqr = __riscv_vfmul_vv_f32m1(diff, diff, vl);
    // //     vfloat32m1_t sum_vec = __riscv_vfredusum_vs_f32m1_f32m1(sqr, __riscv_vfmv_s_f_f32m1(0.0f, 1), vl);
    // //     sum += __riscv_vfmv_f_s_f32m1_f32(sum_vec);
    // //     i += vl;
    // // }

    // // 0404
    // size_t max_vl = __riscv_vsetvl_e32m1(qty);
    // size_t i = 0;
    // float sum = 0.0f;
    
    // for (; i + max_vl <= qty; i += max_vl) {
    //     vfloat32m1_t v1 = __riscv_vle32_v_f32m1(pVect1 + i, max_vl);
    //     vfloat32m1_t v2 = __riscv_vle32_v_f32m1(pVect2 + i, max_vl);
    //     vfloat32m1_t diff = __riscv_vfsub_vv_f32m1(v1, v2, max_vl);
    //     vfloat32m1_t sqr = __riscv_vfmul_vv_f32m1(diff, diff, max_vl);
    //     vfloat32m1_t sum_vec = __riscv_vfredusum_vs_f32m1_f32m1(sqr, __riscv_vfmv_s_f_f32m1(0.0f, 1), max_vl);
    //     sum += __riscv_vfmv_f_s_f32m1_f32(sum_vec);
    // }

    // if (i < qty) {
    //     size_t remaining = qty - i;
    //     size_t vl = __riscv_vsetvl_e32m1(remaining);
    //     vfloat32m1_t v1 = __riscv_vle32_v_f32m1(pVect1 + i, vl);
    //     vfloat32m1_t v2 = __riscv_vle32_v_f32m1(pVect2 + i, vl);
    //     vfloat32m1_t diff = __riscv_vfsub_vv_f32m1(v1, v2, vl);
    //     vfloat32m1_t sqr = __riscv_vfmul_vv_f32m1(diff, diff, vl);
    //     vfloat32m1_t sum_vec = __riscv_vfredusum_vs_f32m1_f32m1(sqr, __riscv_vfmv_s_f_f32m1(0.0f, 1), vl);
    //     sum += __riscv_vfmv_f_s_f32m1_f32(sum_vec);
    // }

    // return sum;


    // 4.20
    float *a = (float *) pVect1v;
    float *b = (float *) pVect2v;
    size_t dim = *((size_t *) qty_ptr);

    // Get the maximum vector length the hardware can handle for float32 with LMUL=1
    size_t max_vl = __riscv_vsetvlmax_e32m1();
    size_t i = 0; // Loop counter
    float sum = 0.0f; // Scalar accumulator for the final sum

    // Initialize a vector register to accumulate sums within the loop
    vfloat32m1_t vec_sum_acc = __riscv_vfmv_v_f_f32m1(0.0f, max_vl); // Initialize accumulator vector to zeros

    // Main loop: process chunks using vector instructions
    for (; i < dim; ) {
        // Set the vector length for this iteration based on remaining elements
        size_t vl = __riscv_vsetvl_e32m1(dim - i);

        // Load elements from a and b into vector registers
        vfloat32m1_t v1 = __riscv_vle32_v_f32m1(a + i, vl);
        vfloat32m1_t v2 = __riscv_vle32_v_f32m1(b + i, vl);

        // Calculate difference: diff = v1 - v2
        vfloat32m1_t diff = __riscv_vfsub_vv_f32m1(v1, v2, vl);

        // Calculate square of difference: sqr = diff * diff
        // Alternatively, use vfmac/vfmacc for fused multiply-accumulate if accumulating directly
        vfloat32m1_t sqr = __riscv_vfmul_vv_f32m1(diff, diff, vl);

        // Accumulate the squares within the vector register
        vec_sum_acc = __riscv_vfadd_vv_f32m1(vec_sum_acc, sqr, vl);

        // Move to the next chunk
        i += vl;
    }

    // Perform final reduction of the accumulated vector sum after the loop
    // Initialize a scalar target vector (element 0 will hold the result)
    vfloat32m1_t scalar_sum_vec = __riscv_vfmv_s_f_f32m1(0.0f, max_vl); 
    // Reduce vec_sum_acc into the first element of scalar_sum_vec
    scalar_sum_vec = __riscv_vfredusum_vs_f32m1_f32m1(vec_sum_acc, scalar_sum_vec, max_vl); 
    // Extract the scalar result from the first element
    sum = __riscv_vfmv_f_s_f32m1_f32(scalar_sum_vec); 

    // Note: The loop structure with vsetvl handles the tail automatically.
    // The previous tail processing logic is integrated into the main loop.

    return sum; // Return the final L2 squared distance
}
#endif

namespace hnswlib {

static float
L2Sqr(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);

    float res = 0;
    for (size_t i = 0; i < qty; i++) {
        float t = *pVect1 - *pVect2;
        pVect1++;
        pVect2++;
        res += t * t;
    }
    return (res);
}

#if defined(USE_AVX512)

// Favor using AVX512 if available.
static float
L2SqrSIMD16ExtAVX512(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);
    float PORTABLE_ALIGN64 TmpRes[16];
    size_t qty16 = qty >> 4;

    const float *pEnd1 = pVect1 + (qty16 << 4);

    __m512 diff, v1, v2;
    __m512 sum = _mm512_set1_ps(0);

    while (pVect1 < pEnd1) {
        v1 = _mm512_loadu_ps(pVect1);
        pVect1 += 16;
        v2 = _mm512_loadu_ps(pVect2);
        pVect2 += 16;
        diff = _mm512_sub_ps(v1, v2);
        // sum = _mm512_fmadd_ps(diff, diff, sum);
        sum = _mm512_add_ps(sum, _mm512_mul_ps(diff, diff));
    }

    _mm512_store_ps(TmpRes, sum);
    float res = TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3] + TmpRes[4] + TmpRes[5] + TmpRes[6] +
            TmpRes[7] + TmpRes[8] + TmpRes[9] + TmpRes[10] + TmpRes[11] + TmpRes[12] +
            TmpRes[13] + TmpRes[14] + TmpRes[15];

    return (res);
}
#endif

#if defined(USE_AVX)

// Favor using AVX if available.
static float
L2SqrSIMD16ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);
    float PORTABLE_ALIGN32 TmpRes[8];
    size_t qty16 = qty >> 4;

    const float *pEnd1 = pVect1 + (qty16 << 4);

    __m256 diff, v1, v2;
    __m256 sum = _mm256_set1_ps(0);

    while (pVect1 < pEnd1) {
        v1 = _mm256_loadu_ps(pVect1);
        pVect1 += 8;
        v2 = _mm256_loadu_ps(pVect2);
        pVect2 += 8;
        diff = _mm256_sub_ps(v1, v2);
        sum = _mm256_add_ps(sum, _mm256_mul_ps(diff, diff));

        v1 = _mm256_loadu_ps(pVect1);
        pVect1 += 8;
        v2 = _mm256_loadu_ps(pVect2);
        pVect2 += 8;
        diff = _mm256_sub_ps(v1, v2);
        sum = _mm256_add_ps(sum, _mm256_mul_ps(diff, diff));
    }

    _mm256_store_ps(TmpRes, sum);
    return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3] + TmpRes[4] + TmpRes[5] + TmpRes[6] + TmpRes[7];
}

#endif

#if defined(USE_SSE)

static float
L2SqrSIMD16ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);
    float PORTABLE_ALIGN32 TmpRes[8];
    size_t qty16 = qty >> 4;

    const float *pEnd1 = pVect1 + (qty16 << 4);

    __m128 diff, v1, v2;
    __m128 sum = _mm_set1_ps(0);

    while (pVect1 < pEnd1) {
        //_mm_prefetch((char*)(pVect2 + 16), _MM_HINT_T0);
        v1 = _mm_loadu_ps(pVect1);
        pVect1 += 4;
        v2 = _mm_loadu_ps(pVect2);
        pVect2 += 4;
        diff = _mm_sub_ps(v1, v2);
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

        v1 = _mm_loadu_ps(pVect1);
        pVect1 += 4;
        v2 = _mm_loadu_ps(pVect2);
        pVect2 += 4;
        diff = _mm_sub_ps(v1, v2);
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

        v1 = _mm_loadu_ps(pVect1);
        pVect1 += 4;
        v2 = _mm_loadu_ps(pVect2);
        pVect2 += 4;
        diff = _mm_sub_ps(v1, v2);
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

        v1 = _mm_loadu_ps(pVect1);
        pVect1 += 4;
        v2 = _mm_loadu_ps(pVect2);
        pVect2 += 4;
        diff = _mm_sub_ps(v1, v2);
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));
    }

    _mm_store_ps(TmpRes, sum);
    return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3];
}
#endif

#if defined(USE_SSE) || defined(USE_AVX) || defined(USE_AVX512)
static DISTFUNC<float> L2SqrSIMD16Ext = L2SqrSIMD16ExtSSE;

static float
L2SqrSIMD16ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    size_t qty = *((size_t *) qty_ptr);
    size_t qty16 = qty >> 4 << 4;
    float res = L2SqrSIMD16Ext(pVect1v, pVect2v, &qty16);
    float *pVect1 = (float *) pVect1v + qty16;
    float *pVect2 = (float *) pVect2v + qty16;

    size_t qty_left = qty - qty16;
    float res_tail = L2Sqr(pVect1, pVect2, &qty_left);
    return (res + res_tail);
}
#endif


#if defined(USE_SSE)
static float
L2SqrSIMD4Ext(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float PORTABLE_ALIGN32 TmpRes[8];
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);


    size_t qty4 = qty >> 2;

    const float *pEnd1 = pVect1 + (qty4 << 2);

    __m128 diff, v1, v2;
    __m128 sum = _mm_set1_ps(0);

    while (pVect1 < pEnd1) {
        v1 = _mm_loadu_ps(pVect1);
        pVect1 += 4;
        v2 = _mm_loadu_ps(pVect2);
        pVect2 += 4;
        diff = _mm_sub_ps(v1, v2);
        sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));
    }
    _mm_store_ps(TmpRes, sum);
    return TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3];
}

static float
L2SqrSIMD4ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    size_t qty = *((size_t *) qty_ptr);
    size_t qty4 = qty >> 2 << 2;

    float res = L2SqrSIMD4Ext(pVect1v, pVect2v, &qty4);
    size_t qty_left = qty - qty4;

    float *pVect1 = (float *) pVect1v + qty4;
    float *pVect2 = (float *) pVect2v + qty4;
    float res_tail = L2Sqr(pVect1, pVect2, &qty_left);

    return (res + res_tail);
}
#endif

class L2Space : public SpaceInterface<float> {
    DISTFUNC<float> fstdistfunc_;
    size_t data_size_;
    size_t dim_;

 public:
    L2Space(size_t dim) {
        fstdistfunc_ = L2Sqr;
#if defined(USE_RVV)
        fstdistfunc_ = L2SqrSIMDExtRVV;
        std::cout << "Using RVV" << std::endl;
#elif defined(USE_SSE) || defined(USE_AVX) || defined(USE_AVX512)
    #if defined(USE_AVX512)
        if (AVX512Capable()) {
            L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX512;
            std::cout << "Using AVX512" << std::endl;
        }
        else if (AVXCapable()) {
            L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX;
            std::cout << "Using AVX" << std::endl;
        }
    #elif defined(USE_AVX)
        if (AVXCapable()) {
            L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX;
            std::cout << "Using AVX" << std::endl;
        }
    #else
        std::cout << "Using SSE" << std::endl;
    #endif

        if (dim % 16 == 0)
            fstdistfunc_ = L2SqrSIMD16Ext;
        else if (dim % 4 == 0)
            fstdistfunc_ = L2SqrSIMD4Ext;
        else if (dim > 16)
            fstdistfunc_ = L2SqrSIMD16ExtResiduals;
        else if (dim > 4)
            fstdistfunc_ = L2SqrSIMD4ExtResiduals;
#endif
        dim_ = dim;
        data_size_ = dim * sizeof(float);
    }

    size_t get_data_size() {
        return data_size_;
    }

    DISTFUNC<float> get_dist_func() {
        return fstdistfunc_;
    }

    void *get_dist_func_param() {
        return &dim_;
    }

    ~L2Space() {}
};

static int
L2SqrI4x(const void *__restrict pVect1, const void *__restrict pVect2, const void *__restrict qty_ptr) {
    size_t qty = *((size_t *) qty_ptr);
    int res = 0;
    unsigned char *a = (unsigned char *) pVect1;
    unsigned char *b = (unsigned char *) pVect2;

    qty = qty >> 2;
    for (size_t i = 0; i < qty; i++) {
        res += ((*a) - (*b)) * ((*a) - (*b));
        a++;
        b++;
        res += ((*a) - (*b)) * ((*a) - (*b));
        a++;
        b++;
        res += ((*a) - (*b)) * ((*a) - (*b));
        a++;
        b++;
        res += ((*a) - (*b)) * ((*a) - (*b));
        a++;
        b++;
    }
    return (res);
}

static int L2SqrI(const void* __restrict pVect1, const void* __restrict pVect2, const void* __restrict qty_ptr) {
    size_t qty = *((size_t*)qty_ptr);
    int res = 0;
    unsigned char* a = (unsigned char*)pVect1;
    unsigned char* b = (unsigned char*)pVect2;

    for (size_t i = 0; i < qty; i++) {
        res += ((*a) - (*b)) * ((*a) - (*b));
        a++;
        b++;
    }
    return (res);
}

class L2SpaceI : public SpaceInterface<int> {
    DISTFUNC<int> fstdistfunc_;
    size_t data_size_;
    size_t dim_;

 public:
    L2SpaceI(size_t dim) {
        if (dim % 4 == 0) {
            fstdistfunc_ = L2SqrI4x;
        } else {
            fstdistfunc_ = L2SqrI;
        }
        dim_ = dim;
        data_size_ = dim * sizeof(unsigned char);
    }

    size_t get_data_size() {
        return data_size_;
    }

    DISTFUNC<int> get_dist_func() {
        return fstdistfunc_;
    }

    void *get_dist_func_param() {
        return &dim_;
    }

    ~L2SpaceI() {}
};
}  // namespace hnswlib
