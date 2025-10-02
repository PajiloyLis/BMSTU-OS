#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/fs_struct.h>
#include <linux/path.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bugakov Ivan");

static int __init

md_init(void) {
    struct task_struct *task = &init_task;
    do {
        printk(KERN_INFO
        "### pid - %d, comm - %s, ppid - %d, pcomm - %s, state - %ld, on_cpu - %d, flags - %x, prio - %d, static_prio - %d, "
        "normal_prio - %d, policy - %u, exit_state - %d, exit_code = %d, exit-signal = %d, utime - %llu, stime - %llu, "
        "switch_count - %d, last_switch_time - %llu\n",
                task->pid,
                task->comm,
                task->parent->pid,
                task->parent->comm,
                (long) task->__state,
                task->on_cpu,
                task->flags,
                task->prio,
                task->static_prio,
                task->normal_prio,
                task->policy,
                task->exit_state,
                task->exit_code,
                task->exit_signal,
                task->utime,
                task->stime,
                task->last_switch_count,
                task->last_switch_time
        );
    } while ((task = next_task(task)) != &init_task);
    printk(KERN_INFO
    "### current: pid - %d, comm - %s, ppid - %d, pcomm - %s, state - %ld, on_cpu - %d, flags - %x, prio - %d, static_prio - %d, "
    "normal_prio - %d, policy - %u, exit_state - %d, exit_code - %d, exit-signal - %d, utime - %llu, stime - %llu, "
    "switch_count - %d, last_switch_time - %llu\n",
            current->pid,
            current->comm,
            current->parent->pid,
            current->parent->comm,
            (long) current->__state,
            current->on_cpu,
            current->flags,
            current->prio,
            current->static_prio,
            current->normal_prio,
            current->policy,
            current->exit_state,
            current->exit_code,
            current->exit_signal,
            current->utime,
            current->stime,
            current->last_switch_count,
            current->last_switch_time
    );
    return 0;
}

static void __exit

md_exit(void) {
    printk(KERN_INFO
    "Module exit\n");
}

module_init(md_init);
module_exit(md_exit);
