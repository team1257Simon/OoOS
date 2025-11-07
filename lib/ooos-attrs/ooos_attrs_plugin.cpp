
#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "backend.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "rtl.h"
#include "diagnostic.h"
#include "tree-pass.h"
#include "stringpool.h"
#include "attribs.h"
int plugin_is_GPL_compatible;
static const char* plugin_name = "ooos_attrs";
constexpr static const char attr_name_nointerrupts[] = "nointerrupts";
extern "C" { extern gcc::context* g; }
static pass_data gp_nointerrupts_data
{
	.type = GIMPLE_PASS,
	.name = "ooos_attrs",
	.optinfo_flags = OPTGROUP_NONE,
	.tv_id = TV_NONE,
	.properties_required = PROP_gimple_any,
	.todo_flags_finish = TODO_update_ssa | TODO_cleanup_cfg
};
static attribute_spec nointerrupts_attr
{
	.name = attr_name_nointerrupts,
	.min_length = 0,
	.max_length = 0,
	.decl_required = true,
	.affects_type_identity = true
};
static void register_attributes(void*, void*) { register_attribute(std::addressof(nointerrupts_attr)); }
struct nointerrupts_gimple_pass : public gimple_opt_pass
{
	nointerrupts_gimple_pass(pass_data const& data, gcc::context* ctxt) : gimple_opt_pass(data, ctxt) {}
	virtual unsigned int execute(function*) override
	{
		basic_block entry 	= ENTRY_BLOCK_PTR_FOR_FN(cfun)->next_bb;
		unsigned num_calls 	= 0U;
		for(gimple_stmt_iterator i = gsi_start_bb(entry); !gsi_end_p(i); gsi_next(std::addressof(i)))
		{
			gimple* stmt = gsi_stmt(i);
			if(gimple_code(stmt) == GIMPLE_CALL)
			{
				tree callee = gimple_call_fndecl(stmt);
				if(callee != NULL_TREE)
				{
					tree attr = lookup_attribute(attr_name_nointerrupts, DECL_ATTRIBUTES(callee));
					if(attr != NULL_TREE)
					{
						gasm* before_call		= gimple_build_asm_vec("pushf\n\tcli", nullptr, nullptr, nullptr, nullptr);
						gasm* after_call		= gimple_build_asm_vec("popf", nullptr, nullptr, nullptr, nullptr);
						gimple_asm_set_volatile(before_call, true);
						gimple_asm_set_volatile(after_call, true);
						gsi_insert_before(std::addressof(i), before_call, GSI_SAME_STMT);
						gsi_insert_after(std::addressof(i), after_call, GSI_NEW_STMT);
					}
				}
			}
		}
		return 0U;
	}
} gimple_pass(gp_nointerrupts_data, g);
static plugin_info pi
{
	.version = "0",
	.help = "",
};
int plugin_init(plugin_name_args* info, plugin_gcc_version* ver)
{
	
	register_callback(plugin_name, PLUGIN_INFO, nullptr, std::addressof(pi));
	register_pass_info gp
	{
		.pass 						= std::addressof(gimple_pass),
		.reference_pass_name 		= "ssa",
		.ref_pass_instance_number 	= 1,
		.pos_op 					= PASS_POS_INSERT_AFTER
	};
	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, nullptr, std::addressof(gp));
	register_callback(plugin_name, PLUGIN_ATTRIBUTES, register_attributes, nullptr);
	return 0;
}