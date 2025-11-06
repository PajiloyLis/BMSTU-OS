#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define IRQ_NUM 1
#define DIR_NAME "key_buf_wq"

typedef struct
{
    struct work_struct work;
    ktime_t start_time;
    int code;
} key_work_t;

static struct workqueue_struct *work_queue;
static key_work_t *work1, *work2;

static char *key_names[84] = {
        " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace",
        "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Enter", "Ctrl",
        "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "Shift (left)", "\\",
        "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift (right)",
        "*", "Alt", "Space", "CapsLock",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
        "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
        " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

static struct proc_dir_entry *proc_file;

static int key_code = -1;
static char *key_name;

static int my_show(struct seq_file *m, void *v)
{
    if (key_code != -1)
        seq_printf(m, "Key: %s Code: %d\n", key_name, key_code);
    return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_show, NULL);
}

static struct proc_ops key_fops = {
        .proc_read = seq_read,
        .proc_open = my_open,
        .proc_release = single_release};

static void wq_first(struct work_struct *work)
{
    key_work_t *key_work = (key_work_t *)work;
    int code = key_work->code;
    int is_release = (code & 0x80) ? 1 : 0;
    int press_code = code & 0x7F;

    ktime_t end_time;
    s64 diff_ns;
    if (is_release && press_code < 84)
    {
        key_code = press_code;
        end_time = ktime_get();
        diff_ns = ktime_to_ns(ktime_sub(end_time, key_work->start_time));

        key_name = key_names[press_code];
        printk(KERN_INFO "INFO Key: %s Code: %d Time: %lld\n", key_name, key_code, diff_ns);
    }
}

static void wq_second(struct work_struct *work)
{
    printk(KERN_INFO "INFO work2\n");
    msleep(10);
}

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    if (irq == IRQ_NUM)
    {
        int code = inb(0x60);

        work1->start_time = ktime_get();
        work1->code = code;
        work2->code = code;

        queue_work(work_queue, (struct work_struct *)work1);
        queue_work(work_queue, (struct work_struct *)work2);

        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static int __init my_init(void)
{
    proc_file = proc_create(DIR_NAME, 0444, NULL, &key_fops);
    if (!proc_file)
    {
        printk(KERN_ERR "ERROR: Failed to create proc file\n");
        return -ENOMEM;
    }

    int ret = request_irq(IRQ_NUM, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler));
    if (ret)
    {
        printk(KERN_ERR "Error: request_irq: %d\n", ret);
        return ret;
    }

    work_queue = alloc_workqueue("my_workqueue", __WQ_LEGACY | WQ_MEM_RECLAIM, 1);
    if (!work_queue)
    {
        free_irq(IRQ_NUM, (void *)(my_irq_handler));
        printk(KERN_ERR "Error: Failed to create workqueue\n");
        return -ENOMEM;
    }

    work1 = (key_work_t *)kmalloc(sizeof(key_work_t), GFP_KERNEL);
    if (!work1)
    {
        free_irq(IRQ_NUM, (void *)(my_irq_handler));
        destroy_workqueue(work_queue);
        printk(KERN_ERR "Error: Failed to allocate immediate work\n");
        return -ENOMEM;
    }
    INIT_WORK((struct work_struct *)work1, wq_first);

    work2 = (key_work_t *)kmalloc(sizeof(key_work_t), GFP_KERNEL);
    if (!work2)
    {
        free_irq(IRQ_NUM, (void *)(my_irq_handler));
        destroy_workqueue(work_queue);
        kfree(work1);
        printk(KERN_ERR "ERROR: Failed to allocate delayed work\n");
        return -ENOMEM;
    }
    INIT_WORK((struct work_struct *)work2, wq_second);

    printk(KERN_INFO "Keyboard module loaded\n");
    return 0;
}

static void __exit my_exit(void)
{
    proc_remove(proc_file);

    flush_workqueue(work_queue);
    destroy_workqueue(work_queue);

    free_irq(IRQ_NUM, (void *)(my_irq_handler));

    kfree(work1);
    kfree(work2);

    printk(KERN_INFO "INFO: Keyboard module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
