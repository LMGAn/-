//
// Created by 29871 on 2026/2/17.
//

#include "test.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>  // 用于高精度计时

/**
 * 测试内存池性能
 * @param element_size: 元素大小
 * @param block_size: 块大小
 * @return 执行时间（毫秒）
 */
double test_pool_performance(int element_size, int block_size) {
    pool p;
    clock_t start, end;
    
    // 初始化内存池
    poolInitialize(&p, element_size, block_size);
    
    // 检查初始化是否成功
    if (!p.blocks || !p.blocks[0]) {
        printf("内存池初始化失败\n");
        return -1.0;
    }
    
    // 开始计时
    start = clock();
    
    // 执行大量分配和释放操作
    void** pointers = (void**)malloc(sizeof(void*) * TEST_LOOP_COUNT);
    if (!pointers) {
        printf("无法分配测试指针数组\n");
        poolFreePool(&p);
        return -1.0;
    }
    
    int alloc_count = 0;
    
    // 分配阶段
    for (int i = 0; i < TEST_LOOP_COUNT; i++) {
        pointers[i] = poolMalloc(&p);
        if (pointers[i] == NULL) {
            printf("内存池分配失败 at iteration %d\n", i);
            break;
        }
        alloc_count++;
    }
    
    // 释放阶段
    for (int i = 0; i < alloc_count; i++) {
        if (pointers[i] != NULL) {
            poolFree(&p, pointers[i]);
        }
    }
    
    // 结束计时
    end = clock();
    
    // 清理
    free(pointers);
    poolFreePool(&p);
    
    // 返回执行时间（毫秒）
    return ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
}

/**
 * 测试标准malloc性能
 * @param element_size: 元素大小
 * @return 执行时间（毫秒）
 */
double test_malloc_performance(int element_size) {
    clock_t start, end;
    
    // 开始计时
    start = clock();
    
    // 执行大量分配和释放操作
    void** pointers = (void**)malloc(sizeof(void*) * TEST_LOOP_COUNT);
    
    // 分配阶段
    for (int i = 0; i < TEST_LOOP_COUNT; i++) {
        pointers[i] = malloc(element_size);
        if (pointers[i] == NULL) {
            printf("malloc分配失败 at iteration %d\n", i);
            break;
        }
    }
    
    // 释放阶段
    for (int i = 0; i < TEST_LOOP_COUNT; i++) {
        if (pointers[i] != NULL) {
            free(pointers[i]);
        }
    }
    
    // 结束计时
    end = clock();
    
    // 清理
    free(pointers);
    
    // 返回执行时间（毫秒）
    return ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
}

/**
 * 综合性能测试函数
 * @param element_size: 元素大小
 * @param block_size: 块大小
 * @return 测试结果代码
 */
int test_pool(int element_size, int block_size) {
    printf("\n=== 内存池性能测试 ===\n");
    printf("测试参数: 元素大小=%d字节, 块大小=%d, 循环次数=%d\n", 
           element_size, block_size, TEST_LOOP_COUNT);
    
    // 测试内存池性能
    printf("正在测试内存池性能...\n");
    double pool_time = test_pool_performance(element_size, block_size);
    printf("内存池耗时: %.2f 毫秒\n", pool_time);
    
    // 测试malloc性能
    printf("正在测试malloc性能...\n");
    double malloc_time = test_malloc_performance(element_size);
    printf("malloc耗时: %.2f 毫秒\n", malloc_time);
    
    // 计算性能提升
    if (malloc_time > 0) {
        double improvement = (malloc_time - pool_time) / malloc_time * 100;
        printf("性能提升: %.2f%%\n", improvement);
        
        if (improvement > 0) {
            printf("✅ 内存池性能优于malloc\n");
        } else {
            printf("❌ malloc性能优于内存池\n");
        }
    }
    
    // 内存使用统计
    printf("\n=== 内存使用统计 ===\n");
    pool test_pool_stats;
    poolInitialize(&test_pool_stats, element_size, block_size);
    
    // 分配一些内存来观察内存使用情况
    void* test_ptrs[100];
    for (int i = 0; i < 100; i++) {
        test_ptrs[i] = poolMalloc(&test_pool_stats);
    }
    
    printf("已分配块数: %u\n", test_pool_stats.block + 1);
    printf("当前块使用量: %u/%u\n", test_pool_stats.used, test_pool_stats.blockSize);
    printf("总块数组大小: %u\n", test_pool_stats.blocksUsed);
    
    // 释放测试内存
    for (int i = 0; i < 100; i++) {
        if (test_ptrs[i] != NULL) {
            poolFree(&test_pool_stats, test_ptrs[i]);
        }
    }
    poolFreePool(&test_pool_stats);
    
    return 0;
}

/**
 * 性能比较测试 - 对比内存池和malloc的性能差异
 */
void performance_comparison_test() {
    printf("\n=== 内存池 vs malloc 性能对比测试 ===\n");
    
    // 测试不同元素大小的性能
    int test_sizes[] = {16, 32, 64, 128, 256};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    printf("\n%-8s %-15s %-15s %-12s\n", "大小", "内存池(毫秒)", "malloc(毫秒)", "性能提升");
    printf("%s\n", "--------------------------------------------------------");
    
    for (int i = 0; i < num_sizes; i++) {
        int size = test_sizes[i];
        
        // 测试内存池性能
        double pool_time = test_pool_performance(size, 8);
        
        // 测试malloc性能
        double malloc_time = test_malloc_performance(size);
        
        // 计算性能提升
        double improvement = 0;
        if (malloc_time > 0) {
            improvement = (malloc_time - pool_time) / malloc_time * 100;
        }
        
        printf("%-8d %-15.2f %-15.2f %+11.2f%%\n", 
               size, pool_time, malloc_time, improvement);
    }
    
    // 内存使用效率测试
    printf("\n=== 内存使用效率测试 ===\n");
    pool efficiency_pool;
    poolInitialize(&efficiency_pool, 32, 8);
    
    const int ALLOC_COUNT = 1000;
    void* pointers[ALLOC_COUNT];
    
    // 分配大量小块内存
    for (int i = 0; i < ALLOC_COUNT; i++) {
        pointers[i] = poolMalloc(&efficiency_pool);
    }
    
    printf("分配了 %d 个 32字节 的元素\n", ALLOC_COUNT);
    printf("实际使用的内存块数: %u\n", efficiency_pool.block + 1);
    printf("每个块容量: %u 元素\n", efficiency_pool.blockSize);
    printf("总内存使用: %u 字节\n", 
           (efficiency_pool.block + 1) * efficiency_pool.blockSize * efficiency_pool.elementSize);
    
    // 释放所有内存
    for (int i = 0; i < ALLOC_COUNT; i++) {
        if (pointers[i] != NULL) {
            poolFree(&efficiency_pool, pointers[i]);
        }
    }
    
    printf("释放后空闲链表长度: ");
    int free_count = 0;
    poolFreed* current = efficiency_pool.freed;
    while (current != NULL) {
        free_count++;
        current = current->nextFree;
    }
    printf("%d 个元素\n", free_count);
    
    poolFreePool(&efficiency_pool);
}