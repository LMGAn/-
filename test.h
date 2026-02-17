//
// Created by 29871 on 2026/2/17.
//

#ifndef UNTITLED_TEST_H
#define UNTITLED_TEST_H
#define TEST_LOOP_COUNT 100000
#include "pool.h"
#include <stdlib.h>

double test_pool_performance(int element_size, int block_size);
double test_malloc_performance(int element_size);

int test_pool(int element_size, int block_size);
void performance_comparison_test(void);
#endif //UNTITLED_TEST_H