//using size_t = unsigned long long;
#include <unordered_map>
// 检查一些对齐参数, 结构体对齐
// ===============================
struct StructA
{
    char u; // offset = 0
    int x; // offset = 4
    double f; // offset = 8
};
constexpr size_t int64 = alignof(__int64);

static_assert(alignof(StructA) == 8, "结构体对齐按照最大算");
static_assert(sizeof(StructA) == 16, "结构体对齐后的大小");


// Union 的访问能力
// ===============================
union UnionA
{
    struct {
        int a;
        char b;
        int x;
    };

    union {
        char z;
    };
    float d;
};

static_assert(alignof(UnionA) == 4, "结构体对齐按照最大算");
static_assert(sizeof(UnionA) == 12, "结构体对齐后的大小");

int test(){
    // 如果 Union 的一个字段没有名字
    // 那么可以直接访问子元素
    union UnionA ua;
    ua.a;
    ua.x;
    ua.z;

    std::hash<const char*> a;

    constexpr unsigned int x = 0xF0000000;
    constexpr int v = x;

    static char *const (*(*const var)())[10], b;
    auto x = var();

    constexpr unsigned n = -123456u;

    return 0;
}



 