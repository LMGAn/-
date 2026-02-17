//
// Created by 29871 on 2026/2/16.
// 内存池头文件 - 定义内存池相关数据结构和函数接口
//

#pragma once  // 防止头文件重复包含
#include <stdint.h>  // 包含标准整数类型定义
#include <pthread.h>
/**
 * 空闲链表节点结构体
 * 用于管理被释放的内存块，实现内存重用
 */
typedef struct poolFreed {
    struct poolFreed* nextFree;  // 指向下一个空闲节点的指针
} poolFreed;

// /**
//  * 内存池核心数据结构
//  * 管理固定大小元素的内存分配和回收
//  */
typedef struct {
    uint32_t elementSize;   // 每个元素的大小（字节）
    uint32_t blockSize;     // 每个内存块能容纳的元素数量
    uint32_t used;          // 当前块中已使用的元素数量
    uint32_t block;         // 当前正在使用的块索引
    uint32_t blocksUsed;    // 已分配的块数组大小（容量）
    uint8_t **blocks;       // 指向块指针数组的指针（二维数组）
    poolFreed* freed;       // 空闲链表头指针，指向第一个可重用的内存块
    pthread_mutex_t mutex;
} pool;

/**
 * 初始化内存池
 * @param p: 指向要初始化的内存池结构体的指针
 * @param elementSize: 每个元素的大小（字节）
 * @param blockSize: 每个内存块包含的元素数量
 * @return void
 *
 * 功能说明：
 * 1. 设置元素大小和块大小
 * 2. 初始化计数器为0
 * 3. 分配初始块指针数组
 * 4. 预分配第一个内存块
 */
void poolInitialize(pool *p, uint32_t elementSize, uint32_t blockSize);

/**
 * 从内存池中分配一个元素
 * @param p: 指向内存池结构体的指针
 * @return void* 分配到的元素指针，失败返回NULL
 *
 * 分配策略优先级：
 * 1. 首先尝试从空闲链表中获取已释放的元素
 * 2. 其次在当前块中分配未使用的空间
 * 3. 最后扩展块数组并分配新块
 */
void *poolMalloc(pool* p);

/**
 * 将元素释放回内存池
 * @param p: 指向内存池结构体的指针
 * @param element: 要释放的元素指针
 * @return void
 *
 * 实现方式：将释放的元素添加到空闲链表头部，供后续重用
 */
void poolFree(pool* p, void* element);

/**
 * 完全释放内存池及其所有资源
 * @param p: 指向内存池结构体的指针
 * @return void
 *
 * 清理工作：
 * 1. 释放所有已分配的内存块
 * 2. 释放块指针数组
 * 3. 将指针置为NULL防止野指针
 */
void poolFreePool(pool* p);