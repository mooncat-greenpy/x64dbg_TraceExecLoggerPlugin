#include "log_instruction.h"


json log_registry()
{
	json reg_json = json::object();
	reg_json["type"] = "register";

	REGDUMP reg_dump;
	DbgGetRegDumpEx(&reg_dump, sizeof(reg_dump));
	REGISTERCONTEXT reg = reg_dump.regcontext;

	reg_json["cax"] = make_address_json(reg.cax);
	reg_json["cbx"] = make_address_json(reg.cbx);
	reg_json["ccx"] = make_address_json(reg.ccx);
	reg_json["cdx"] = make_address_json(reg.cdx);
	reg_json["csi"] = make_address_json(reg.csi);
	reg_json["cdi"] = make_address_json(reg.cdi);
	reg_json["csp"] = make_address_json(reg.csp);
	reg_json["cbp"] = make_address_json(reg.cbp);
#ifdef _WIN64
	reg_json["r8"] = make_address_json(reg.r8);
	reg_json["r9"] = make_address_json(reg.r9);
	reg_json["r10"] = make_address_json(reg.r10);
	reg_json["r11"] = make_address_json(reg.r11);
	reg_json["r12"] = make_address_json(reg.r12);
	reg_json["r13"] = make_address_json(reg.r13);
	reg_json["r14"] = make_address_json(reg.r14);
	reg_json["r15"] = make_address_json(reg.r15);
#endif
	return reg_json;
}
