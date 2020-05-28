#ifndef CC_UTILS_H
#define CC_UTILS_H

// 返回 1 如果两个字符串相等, 否则返回 0
int str_equal(const char* a, const char* b);

// 字符串连接操作, 注意要用户释放内存
char* str_concat(const char* a, const char* b);

// 从 c 中复制 一下字符串
char* str_dup(const char* c);

#endif // CC_UTILS_H

