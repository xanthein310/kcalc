#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/livepatch.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "expression.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Patch calc kernel module");
MODULE_VERSION("0.1");

void livepatch_nop_cleanup(struct expr_func *f, void *c)
{
    /* suppress compilation warnings */
    (void) f;
    (void) c;
}

int livepatch_nop(struct expr_func *f, vec_expr_t args, void *c)
{
    (void) args;
    (void) c;
    pr_err("function nop is now patched\n");
    return 0;
}

void livepatch_fib_cleanup(struct expr_func *f, void *c)
{
    /* suppress compilation warnings */
    (void) f;
    (void) c;
}

int livepatch_fib(struct expr_func *f, vec_expr_t args, void *c)
{
    (void) c;
    if (vec_len(&args) == 1) {
        int k = expr_eval(&vec_nth(&args, 0)) >> 4;

        printk("args = %d\n", expr_eval(&vec_nth(&args, 0)));

        int count = 0;
        int temp_k = k;

        if (k == 0)
            return 0;
        int fn = 0;
        int fn1 = 1;

        while ((temp_k & 0x80000000) == 0) {
            count++;
            temp_k <<= 1;
        }

        k = k << count;
        for (count = 32 - count; count > 0; count--) {
            int f2n1 = fn1 * fn1 + fn * fn;
            int f2n = fn * (2 * fn1 - fn);
            if (k & 0x80000000) {
                fn = f2n1;
                fn1 = f2n + f2n1;
            } else {
                fn = f2n;
                fn1 = f2n1;
            }
            k <<= 1;
        }

        return fn << 4;
    }

    return 0;
}


/* clang-format off */
static struct klp_func funcs[] = {
    {
        .old_name = "user_func_nop",
        .new_func = livepatch_nop,
    },
    {
        .old_name = "user_func_nop_cleanup",
        .new_func = livepatch_nop_cleanup,
    },
    {
        .old_name = "user_func_fib",
        .new_func = livepatch_fib,
    },
    {
        .old_name = "user_func_fib_cleanup",
        .new_func = livepatch_fib_cleanup,
    },
    {},
};
static struct klp_object objs[] = {
    {
        .name = "calc",
        .funcs = funcs,
    },
    {},
};
/* clang-format on */

static struct klp_patch patch = {
    .mod = THIS_MODULE,
    .objs = objs,
};

static int livepatch_calc_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
    return klp_enable_patch(&patch);
#else
    int ret = klp_register_patch(&patch);
    if (ret)
        return ret;
    ret = klp_enable_patch(&patch);
    if (ret) {
        WARN_ON(klp_unregister_patch(&patch));
        return ret;
    }
    return 0;
#endif
}

static void livepatch_calc_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)
    WARN_ON(klp_unregister_patch(&patch));
#endif
}

module_init(livepatch_calc_init);
module_exit(livepatch_calc_exit);
MODULE_INFO(livepatch, "Y");
