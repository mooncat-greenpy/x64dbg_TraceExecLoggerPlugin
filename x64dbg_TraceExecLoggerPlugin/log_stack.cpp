#include "log_stack.h"


json log_stack()
{
	json stack_json = json::object();
	stack_json["type"] = "stack";
	stack_json["data"] = json::array();

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	duint csp = reg_dump.regcontext.csp;

	for (int i = 0; i < 0x10; i++)
	{
		json tmp_json = json::object();
		duint stack_addr = csp + i * sizeof(duint);
		duint stack_value = 0;
		if (!DbgMemIsValidReadPtr(stack_addr))
		{
			tmp_json["address"] = stack_addr;
			tmp_json["value"] = "";
			stack_json["data"].push_back(tmp_json);
			continue;
		}
		DbgMemRead(stack_addr, &stack_value, sizeof(stack_value));

		tmp_json["address"] = stack_addr;
		tmp_json["value"] = make_address_json(stack_value);
		stack_json["data"].push_back(tmp_json);
	}

	return stack_json;
}