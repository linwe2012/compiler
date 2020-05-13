#include "config.h"
#include <stdint.h>

enum ValueType
{
	Value_Label,
	Value_Runtime,
	Value_Constant
};
struct Value {

	int64_t value_id;
	void* constant;
};
STRUCT_TYPE(Value);


struct Gen
{
	void (*label_)(Value a);
	void (*sub_)(Value a, Value b);
	void (*cmp_)(Value a, Value b);
	void (*jne_)(Value label_id);
	void (*je_)(Value label_id);

};