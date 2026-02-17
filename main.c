//
// Created by 29871 on 2026/2/16.
//
#include <stdio.h>
#include <string.h>

#include "pool.h"
#include "test.h"

void test01() {
    pool p;
    memset(&p, 0, sizeof(pool)); // 初始化为0
    poolInitialize(&p, 32, 4);
    // 验证参数赋值
    printf("elementSize: %d (32)\n", p.elementSize);
    printf("blockSize: %u (4)\n", p.blockSize);  // 修正这一行
    printf("used: %u (0)\n", p.used);
    printf("block: %u (0)\n", p.block);
    printf("blocksUsed: %u (1)\n", p.blocksUsed);
    printf("freed: %p (NULL)\n", (void*)p.freed);  // 修正这一行
    printf("block[0]: %s\n", p.blocks && p.blocks[0] ? "YES" : "NO");
    poolFreePool(&p);
}
// 测试空闲链表
void test02() {
    pool p;
    // 初始化
    poolInitialize(&p, 32, 4);
    // 申请四个元素
    void* e1 = poolMalloc(&p);
    void* e2 = poolMalloc(&p);
    void* e3 = poolMalloc(&p);
    void* e4 = poolMalloc(&p);

    printf("e1: %p\ne2: %p\ne3: %p\ne4: %p\n", e1, e2, e3, e4);

    // 分配第五个元素
    void* e5 = poolMalloc(&p);
    printf("e5: %p\n", e5);

    // 释放e2
    poolFree(&p, e2);

    // 再申请
    void* e6 = poolMalloc(&p);
    printf("e6: %p\n", e6);
}

int main() {
    system("chcp 65001");
    
    printf("内存池性能测试程序\n");
    printf("====================\n");
    
    // 基础功能测试
    printf("\n1. 基础功能测试\n");
    test01();
    test02();
    
    // 性能对比测试
    printf("\n2. 性能对比测试\n");
    performance_comparison_test();
    
    printf("\n所有测试完成！\n");
    return 0;
}

