// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// modified based on Google V8 engine
/*
#include "config.h"
#include "ast.h"

#define GENERAL_REGISTERS(V) \
  V(rax)                     \
  V(rcx)                     \
  V(rdx)                     \
  V(rbx)                     \
  V(rsp)                     \
  V(rbp)                     \
  V(rsi)                     \
  V(rdi)                     \
  V(r8)                      \
  V(r9)                      \
  V(r10)                     \
  V(r11)                     \
  V(r12)                     \
  V(r13)                     \
  V(r14)                     \
  V(r15)

#define I386_REGISTERS(V)   \
  V(ax)                     \
  V(cx)                     \
  V(dx)                     \
  V(bx)                     \
  V(sp)                     \
  V(bp)                     \
  V(si)                     \
  V(di)                     

#define X86_REGISTERS(V)   \
  V(eax)                     \
  V(ecx)                     \
  V(edx)                     \
  V(ebx)                     \
  V(esp)                     \
  V(ebp)                     \
  V(esi)                     \
  V(edi)                     

#define ALLOCATABLE_GENERAL_REGISTERS(V) \
  V(rax)                                 \
  V(rbx)                                 \
  V(rdx)                                 \
  V(rcx)                                 \
  V(rsi)                                 \
  V(rdi)                                 \
  V(r8)                                  \
  V(r9)                                  \
  V(r11)                                 \
  V(r12)                                 \
  V(r14)                                 \
  V(r15)

#define DOUBLE_REGISTERS(V) \
  V(xmm0)                   \
  V(xmm1)                   \
  V(xmm2)                   \
  V(xmm3)                   \
  V(xmm4)                   \
  V(xmm5)                   \
  V(xmm6)                   \
  V(xmm7)                   \
  V(xmm8)                   \
  V(xmm9)                   \
  V(xmm10)                  \
  V(xmm11)                  \
  V(xmm12)                  \
  V(xmm13)                  \
  V(xmm14)                  \
  V(xmm15)


enum ValuePosition
{
    Value_Register,
    Value_Memory,
    Value_Immediate
};

struct Value
{
    enum ValuePosition type;
};
STRUCT_TYPE(Value);

enum RegisterCode {
#define REGISTER_CODE(R) kRegCode_##R,

    GENERAL_REGISTERS(REGISTER_CODE)
    kGeneralRegisterEnd,

    kDoubleRegisterBegin = kGeneralRegisterEnd - 1,
    DOUBLE_REGISTERS(REGISTER_CODE)

    kNumRegisters,
    

    kI386RegisterBegin = kNumRegisters - 1,
    I386_REGISTERS(REGISTER_CODE)
    kI386RegisterEnd,

    kX86RegisterBegin = kI386RegisterEnd - 1,
    X86_REGISTERS(REGISTER_CODE)
    kX86RegisterEnd,

#undef REGISTER_CODE
    kNumRegisterCodes = kX86RegisterEnd,
};

#define kNumI368Registers (kI386RegisterEnd - kI386RegisterBegin - 1)

enum RegisterMode
{
    kReg16,
    kReg32,
    kReg64,
    kRegXMM,
    // 指向内存的指针
    kRegMemory,

};

struct Register
{
    Value super;
    enum RegisterCode code;
    const char* name;

    
    enum RegisterMode mode;
    enum RegisterMode xmm_mode;
};
STRUCT_TYPE(Register)

inline int high_bit(Register r) { return r.code >> 3; }

inline int low_bits(Register r) { return r.code & 0x7; }



#define DECLARE_REGISTER(R) \
extern Value* R;
GENERAL_REGISTERS(DECLARE_REGISTER)
#undef DECLARE_REGISTER



#ifdef CC_OS_WIN
// Windows calling convention
#define arg_reg_1 rcx;
#define arg_reg_2 rdx;
#define arg_reg_3 r8;
#define arg_reg_4 r9;
#else
// AMD64 calling convention
#define arg_reg_1 rdi;
#define arg_reg_2 rsi;
#define arg_reg_3 rdx;
#define arg_reg_4 rcx;
#endif  // V8_TARGET_OS_WIN



struct Immediate
{
    Value super;
    enum Types type;
    union {
        int64_t i64;
        int32_t i32;
        uint64_t ui64;
        double  f64;
        float f32;
    };
    
};
STRUCT_TYPE(Immediate)

struct MemoryValue
{
    Value super;
    enum Types type;
    void* ptr;
};
STRUCT_TYPE(MemoryValue)

struct TypedValueInterface
{
    Value super;
    enum Types type;
};

struct ASMContext
{
    char* code_buffer;

    // assembly = 1, 生成汇编代码
    // assembly = 0, 生成机器指令
    int assembly;
};
STRUCT_TYPE(ASMContext)

Value* x64_visit_ast(ASMContext* ctx, AST* ast);

Value* make_immediate_num(void* x, enum Types type);

void emit_mov(ASMContext* ctx, Value* src, Value* dst);

char* value_str(Value* src);

void emit_pop(ASMContext* ctx, Value* reg);

void emit_ret(ASMContext* ctx, int pop);

int emit_push(ASMContext* ctx, Value* val);

void emit_call(ASMContext* ctx, const char* name);

void emit_label(ASMContext* ctx, const char* name);

void x64_asm_init(ASMContext* ctx);

// dst -= src
void emit_sub(ASMContext* ctx, Value* dst, Value* src);
*/