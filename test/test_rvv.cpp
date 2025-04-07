#include <stdio.h>
#include "../utils/dataset.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>

#if defined(USE_RVV)
#include <riscv_vector.h>
static float
L2SqrSIMD16ExtRVV(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);
    
    float sum = 0.0f;
    size_t i = 0;

    while (i < qty) {
        size_t vl = __riscv_vsetvl_e32m1(qty - i);
        vfloat32m1_t v1 = __riscv_vle32_v_f32m1(pVect1 + i, vl);
        vfloat32m1_t v2 = __riscv_vle32_v_f32m1(pVect2 + i, vl);
        vfloat32m1_t diff = __riscv_vfsub_vv_f32m1(v1, v2, vl);
        vfloat32m1_t sqr = __riscv_vfmul_vv_f32m1(diff, diff, vl);
        vfloat32m1_t sum_vec = __riscv_vfredusum_vs_f32m1_f32m1(sqr, __riscv_vfmv_s_f_f32m1(0.0f, 1), vl);
        float tmp = __riscv_vfmv_f_s_f32m1_f32(sum_vec);

        sum += tmp;
        // // check result
        // float check_tmp = 0.0;
        // for (int j = 0; j < vl; j++) {
        //     float t = pVect1[i + j] - pVect2[i + j];
        //     float s = t * t;
        //     check_tmp += s;
        // }
        // if (std::abs(tmp - check_tmp) > 1e-6) {
        //     std::cout << "Error: " << tmp << " != " << check_tmp << std::endl;
        // }

        i += vl;
    }
    
    return sum;
}
#endif

static float
L2Sqr(const void *pVect1v, const void *pVect2v, const void *qty_ptr) {
    float *pVect1 = (float *) pVect1v;
    float *pVect2 = (float *) pVect2v;
    size_t qty = *((size_t *) qty_ptr);
    // std::cout << "qty: " << qty << std::endl;
    // std::cout << "sizeof(size_t): " << sizeof(size_t) << std::endl;

    float res = 0;
    for (size_t i = 0; i < qty; i++) {
        // std::cout << "i: " << i << std::endl;
        float t = *pVect1 - *pVect2;
        // std::cout << "pVect1: " << *pVect1 << std::endl;
        // std::cout << "pVect2: " << *pVect2 << std::endl;
        // std::cout << "t: " << t << std::endl;
        pVect1++;
        pVect2++;
        res += t * t;
    }
    return (res);
}

int main() {
    float a[64] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float b[64] = {2.0, 3.0, 4.0, 5.0, 6.0};
    for (int i = 6; i < 64; i++) {
        a[i] = 0.0;
        b[i] = 0.0;
    }
    size_t qty = 64;
    float result = 0.0;
#if defined(USE_RVV)
    result = L2SqrSIMD16ExtRVV(a, b, &qty);
    printf("L2 squared distance: %f\n", result); // 应输出 5.0
#else
    result = L2Sqr(a, b, &qty);
    printf("L2 squared distance: %f\n", result); // 应输出 5.0
#endif

    // // float *av = generate_random_data(dim, cycle, 2);
    // // float *bv = generate_random_data(dim, cycle, 3);
    int dataset_dim;
    int train_elements, test_elements, dataset_topk;
    float *av, *bv, *neighbors, *distances;
    read_bin("../data/random-xs-32-euclidean.bin", dataset_dim, train_elements, test_elements, dataset_topk, av, bv, neighbors, distances);
    int cycle = std::min(train_elements, test_elements);
    size_t dim = (size_t) dataset_dim;
    result = 0.0;
    auto start = clock();
    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < cycle; i++) {
            // std::cout << "i: " << i << std::endl;
            #if defined(USE_RVV)
            result += L2SqrSIMD16ExtRVV(av + i * dim, bv + i * dim, &dim);
            // printf("L2RVV squared distance: %f\n", result);
            #else
            // std::cout << "Use L2Sqr" << std::endl;
            result += L2Sqr(av + i * dim, bv + i * dim, &dim);
            // printf("L2 squared distance: %f\n", result);
            #endif
        }
    }
    auto end = clock();
    printf("Time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    std::cout << "Result: " << result << std::endl;
    delete[] av;
    delete[] bv;
    return 0;
}
