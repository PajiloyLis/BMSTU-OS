#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/atomic.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

#define IRQ_NUM 1
#define DIR_NAME "key_buf"

static char *key_names[84] = {
        " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace",
        "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Enter", "Ctrl",
        "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "Shift (left)", "\\",
        "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift (right)",
        "*", "Alt", "Space", "CapsLock",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
        "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
        " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};


static struct proc_dir_entry *proc_entry = NULL;

static int code1 = -1;
static char *name1;

static int code;

static ktime_t irq_start_time;

static void my_tasklet_function(struct tasklet_struct *t)
{
    int is_release = (code & 0x80) ? 1 : 0;
    int press_code = code & 0x7F;

    if (is_release && press_code < 84)
    {
        code1 = press_code;

        name1 = key_names[press_code];

        ktime_t end_time = ktime_get();
        ktime_t diff;
        s64 nsecs;
        diff = ktime_sub(end_time, irq_start_time);
        nsecs = ktime_to_ns(diff);

        printk(KERN_INFO "INFO Key: %s Code: %d Time: %lld\n", name1, code1, nsecs);
    }
}

DECLARE_TASKLET(my_tasklet, my_tasklet_function);

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    if (irq == IRQ_NUM)
    {
        code = inb(0x60);
        irq_start_time = ktime_get();
        tasklet_schedule(&my_tasklet);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static int my_single_show(struct seq_file *m, void *v)
{
    if (code1 != -1)
        seq_printf(m, "Key: %s Code: %d\n",
                   name1, code1);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_single_show, NULL);
}

static const struct proc_ops key_buf_fops = {
        .proc_open = my_proc_open,
        .proc_read = seq_read,
        .proc_release = single_release,
};

static int __init my_init(void)
{
    int ret = request_irq(IRQ_NUM, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler));
    if (ret)
    {
        printk(KERN_ERR "Error: request_irq: %d\n", ret);
        return ret;
    }

    proc_entry = proc_create(DIR_NAME, 0444, NULL, &key_buf_fops);
    if (!proc_entry)
    {
        printk(KERN_ERR "Failed to create\n");
        free_irq(IRQ_NUM, (void *)(my_irq_handler));
        return -ENOMEM;
    }

    printk(KERN_INFO "INFO module loaded.\n");
    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(&my_tasklet);
    free_irq(IRQ_NUM, (void *)(my_irq_handler));

    if (proc_entry)
        proc_remove(proc_entry);

    printk(KERN_INFO "INFO module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
