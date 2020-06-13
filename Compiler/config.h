#ifndef CC_CONFIG_H
#define CC_CONFIG_H

#define CC_DEBUG

// 在 log_internal_error 的时候触发 exception从而实现断点
#define THROW_EXCEPT_ON_LOG_INTERNAL_ERROR

#ifdef _MSC_VER
// 主要处理 GCC & MSVC 兼容问题

#define fileno _fileno

#endif

#define NEW_STRUCT(type, name) struct type* name = (struct type*)calloc(1, sizeof(struct type));


#ifdef CC_DEBUG
#define YYDEBUG 1
#endif

// 如果是 C++ 的话, struct X 自动声明了 X 这个类型
#ifndef __cplusplus
#define STRUCT_TYPE(x) typedef struct x x;
#else
#define STRUCT_TYPE(x) 
#endif // !__cplusplus

#ifdef _WIN32
#define CC_OS_WIN
#endif



#endif
