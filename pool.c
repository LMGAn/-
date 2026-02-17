//
// Created by 29871 on 2026/2/17.
//
//
// Created by 29871 on 2026/2/16.
// 内存池实现文件 - 包含内存池的核心算法实现
//

#include "pool.h"      // 包含内存池接口定义

#include <stdio.h>
#include <string.h>    // 包含内存操作函数
#include <stdlib.h>    // 包含malloc/free等内存管理函数

// 宏定义区域
#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))           // 求两个数的最大值
#define POOL_BLOCKS_INITIAL 1                 // 初始块数组大小
#endif

/**
 * 初始化内存池
 * @param p: 指向要初始化的内存池结构体的指针
 * @param elementSize: 每个元素的大小（字节）
 * @param blockSize: 每个内存块包含的元素数量
 *
 * 初始化步骤：
 * 1. 参数验证和基本设置
 * 2. 分配块指针数组
 * 3. 初始化数组元素为NULL
 * 4. 预分配第一个内存块
 */
void poolInitialize(pool *p, const uint32_t elementSize, const uint32_t blockSize) {
    uint32_t i;
    // 空指针检查，防止段错误
    if (!p) return;

    // 初始化所有字段为0，防止未初始化的字段导致问题
    memset(p, 0, sizeof(pool));
    
    pthread_mutex_init(&(p->mutex), NULL);
    
    // 设置元素大小，确保至少能容纳poolFreed结构体
    p->elementSize = max(elementSize, sizeof(poolFreed));

    // 块大小检查，避免传递0导致逻辑异常，默认值为8
    p->blockSize = (blockSize == 0) ? 8 : blockSize;

    // 初始化计数器
    p->used = 0;        // 当前块已使用元素数
    p->block = 0;       // 当前块索引
    p->freed = NULL;    // 空闲链表为空

    // 设置初始块数组的大小为 POOL_BLOCKS_INITIAL（1）
    // 采用渐进式分配策略，避免一次性分配过多内存
    p->blocksUsed = POOL_BLOCKS_INITIAL;

    // 分配存储块指针的数组，每个元素是指向uint8_t的指针
    // 数组大小为：指针大小 × 块数量
    p->blocks = (uint8_t **) malloc(sizeof(uint8_t*) * p->blocksUsed);
    if (!p->blocks) {
        // 分配失败，清理已分配的资源
        pthread_mutex_destroy(&(p->mutex));
        return;
    }

    // 初始化块指针数组，所有元素初始为NULL
    for (i = 0; i < p->blocksUsed; ++i) {
        p->blocks[i] = NULL;
    }

    // 预分配第一个内存块
    // 大小为：元素大小 × 块容量
    p->blocks[0] = (uint8_t*) malloc(p->elementSize * p->blockSize);
    if (!p->blocks[0]) {
        // 分配失败，清理已分配的资源
        free(p->blocks);
        p->blocks = NULL;
        pthread_mutex_destroy(&(p->mutex));
        return;
    }
}

/**
 * 从内存池中分配一个元素
 * @param p: 指向内存池结构体的指针
 * @return void* 分配到的元素指针，失败返回NULL
 *
 * 分配策略：
 * 1. 优先从空闲链表中获取已释放的元素（最快）
 * 2. 在当前块中分配未使用的空间
 * 3. 当前块满时切换到下一块
 * 4. 块数组满时动态扩展数组
 * 5. 必要时分配新的内存块
 */
void* poolMalloc(pool *p) {
    // 参数检查
    if (!p) return NULL;

    pthread_mutex_lock(&(p->mutex));
    // 策略1：从空闲链表中获取空闲元素（内存重用）
    if (p->freed) {
        poolFreed *pFreed = p->freed;           // 获取链表头节点
        p->freed = pFreed->nextFree;            // 更新链表头
        pthread_mutex_unlock(&(p->mutex));
        return pFreed;                          // 返回重用的内存
    }

    // 策略2：当前块有空闲位置，直接分配
    if (p->used < p->blockSize) {
        // 计算元素在当前块中的位置：
        // 基地址 + 元素索引 × 元素大小
        void* element = p->blocks[p->block] + p->elementSize * p->used;
        ++p->used;                              // 更新使用计数
        pthread_mutex_unlock(&(p->mutex));
        return element;                         // 返回分配的内存
    }

    // 策略3：当前块已满，需要处理块切换
    p->used = 0;    // 重置当前块使用计数
    p->block++;     // 切换到下一个块

    // 处理当前块数组用完的情况，需要扩展块数组
    if (p->block == (int32_t)p->blocksUsed) {
        uint32_t i;

        // 扩容策略：将块数组大小扩大两倍（左移1位）
        p->blocksUsed <<= 1;

        // 使用realloc重新分配块指针数组
        // realloc会自动处理内存复制和原内存释放
        uint8_t** newBlocks = (uint8_t**) realloc(p->blocks, sizeof(uint8_t*) * p->blocksUsed);
        p->blocks = newBlocks;

        // 初始化新分配的数组元素为NULL
        for (i = p->blocksUsed >> 1; i < p->blocksUsed; ++i) {
            p->blocks[i] = NULL;
        }
    }

    // 为新的块分配实际内存空间（懒加载）
    if (p->blocks[p->block] == NULL) {
        // 使用普通malloc而不是_aligned_malloc，保持一致性
        p->blocks[p->block] = (uint8_t*) malloc(p->elementSize * p->blockSize);
        if (!p->blocks[p->block]) {
            pthread_mutex_unlock(&(p->mutex));
            return NULL;  // 分配失败
        }
    }

    // 分配第一个元素并更新计数
    ++(p->used);
    pthread_mutex_unlock(&(p->mutex));
    // 返回新块的第一个元素地址
    return p->blocks[p->block];
}

/**
 * 将指定元素释放回内存池
 * @param p: 指向内存池结构体的指针
 * @param element: 要释放的元素指针
 *
 * 实现机制：头插法将元素添加到空闲链表头部
 * 这样可以实现O(1)时间复杂度的释放操作
 */
void poolFree(pool *p, void *element) {
    // 参数有效性检查
    if (!p || !element) return;

    // 额外的安全检查：确保释放的指针确实来自这个内存池
    // 这里可以添加更严格的验证逻辑
    
    pthread_mutex_lock(&(p->mutex));
    
    // 头插法将元素插入空闲链表：
    // 1. 设置当前元素的next指针指向原来的链表头
    // 2. 更新链表头为当前元素
    ((poolFreed*)element)->nextFree = p->freed;
    p->freed = (poolFreed*)element;
    
    pthread_mutex_unlock(&(p->mutex));

    /*
     * 注释掉的另一种写法（功能相同）：
     * poolFreed* pFreed = p->freed;      // 保存原链表头
     * p->freed = (poolFreed*)element;    // 新元素成为链表头
     * p->freed->nextFree = pFreed;       // 新头指向原头
     */
}

/**
 * 完全释放内存池及其所有资源
 * @param p: 指向内存池结构体的指针
 *
 * 清理顺序很重要：
 * 1. 先释放所有具体的内存块
 * 2. 再释放块指针数组
 * 3. 最后将指针置NULL防止野指针
 */
void poolFreePool(pool *p) {
    // 参数检查
    if (!p) return;

    uint32_t i;
    // 遍历并释放所有已分配的内存块
    for (i = 0; i < p->blocksUsed; ++i) {
        // 遇到NULL说明后面都没有分配，可以提前结束
        if (!p->blocks[i]) break;
        else {
            free(p->blocks[i]);     // 释放具体内存块
            p->blocks[i] = NULL;    // 防止野指针
        }
    }

    // 释放块指针数组本身
    free(p->blocks);
    p->blocks = NULL;               // 防止野指针
    pthread_mutex_destroy(&(p->mutex));
}