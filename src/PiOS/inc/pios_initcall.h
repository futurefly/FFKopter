/*
 * pios_initcall.h
 *
 *  Created on: 19.10.2011
 *      Author: gandhi
 */

#ifndef PIOS_INITCALL_H_
#define PIOS_INITCALL_H_

typedef int32_t (*initcall_t)(void);
    typedef struct {
        initcall_t fn_minit;
        initcall_t fn_tinit;
    } initmodule_t;

/* Init module section */

    extern initmodule_t __module_initcall_start[], __module_initcall_end[];

#define __define_initcall(level,fn,id) \
    static initcall_t __initcall_##fn##id_attribute__((__used__)) \
    __attribute__((__section_(".initcall" level ".init"))) = fn

#define __define_module_initcall(level,ifn,sfn) \
  static initmodule_t __initcall_##fn __attribute__((__used__)) \
  __attribute__((__section__(".initcall" level ".init"))) = { .fn_minit = ifn, .fn_tinit = sfn };

#define UAVOBJ_INITCALL(fn)     __define_initcall("uavobj",fn,1)
#define MODULE_INITCALL(ifn, sfn)	__define_module_initcall("module",ifn,sfn)

#define MODULE_INITIALISE_ALL {for (initmodule_t *fn = __module_initcall_start; fn < __module_initcall_end; fn++)\
	if (fn->fn_minit) \
	(fn->fn_minit) ();}

#define MODULE_TASKCREATE_ALL { for (initmodule_t *fn = __module_initcall_start; fn < __module_initcall_end; fn++)\
	if (fn->fn_tinit) \
	(fn->fn_tinit) (); }

#endif



#endif /* PIOS_INITCALL_H_ */
