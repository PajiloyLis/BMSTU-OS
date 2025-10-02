#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bugakov Ivan");

#define PROC_DIR "seq_dir_1"
#define PROC_FILE "seq_file_1"
#define PROC_LINK "seq_link_1"
#define SIZE 16

static struct proc_dir_entry *my_dir = NULL;
static struct proc_dir_entry *my_file = NULL;
static struct proc_dir_entry *my_symblink = NULL;

static int pid_array[SIZE];
static int pid_count = 0;

static ssize_t proc_file_write(struct file* file, const char __user* buf, size_t count, loff_t *ppos)
{
    char pid_buf[16];
    int pid;
    printk(KERN_INFO "+++ call fortune_write\n");
    if(count >= sizeof(pid_buf))
        return -EINVAL;
    if(copy_from_user(pid_buf, buf, count))
        return -EFAULT;
    pid_buf[count] = 0;
    if(kstrtoint(pid_buf, 10, &pid))
        return -EINVAL;
    if(pid_count < SIZE)
    {
        pid_array[pid_count++]=pid;
        printk(KERN_INFO, "+++ saved pid: %d\n", pid);
    }else{
        return -ENOSPC;
    }
    return count;
}

static void *start(struct seq_file *m, loff_t *pos)
{
    printk(KERN_INFO "+++ start\n");
    if (*pos >= pid_count)
        return NULL;
    return &pid_array[*pos];
}

static void *next(struct seq_file *m, void *v, loff_t *pos)
{
    printk(KERN_INFO "+++ next: s = %px v = %px pos = %px value = %lld\n", m, v, pos, *pos);
    (*pos)++;
    if (*pos >= pid_count)
        return NULL;
    return &pid_array[*pos];
}

static void stop(struct seq_file *m, void *v)
{
    printk(KERN_INFO "+++ stop:\n");
}

static int show(struct seq_file *m, void *v)
{
    int pid = *(int *)v;
    struct task_struct *task;
    printk(KERN_INFO "+++ show: pid = %d v = %px \n", pid, v);
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(!task){
        seq_printf(m, "Error pid task for pid %d");
        return 0;
    }
    seq_printf(m, "+++ pid %lld\ncomm %s\nstatic_prio %d\nflags %x\n", pid, task->comm, task->static_prio, task->flags);
    return 0;
}

ssize_t my_seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    printk(KERN_INFO "+++ read\n");
    return seq_read(file, buf, size, ppos);
}

static int my_seq_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "+++ release\n");
    return seq_release(inode, file);
}

static struct seq_operations my_seq_ops = {
        .start = start,
        .next = next,
        .stop = stop,
        .show = show
};


static int proc_file_open(struct inode* inode, struct file* file) {
    printk(KERN_INFO
    "+++ call fortune_open\n");
    return seq_open(file, &my_seq_ops);
}

static struct proc_ops my_proc_ops = {
        .proc_open = proc_file_open,
        .proc_read = my_seq_read,
        .proc_write = proc_file_write,
        .proc_release = my_seq_release
};

static int __init md_init(void)
{
    my_dir = proc_mkdir(PROC_DIR, NULL);
    printk(KERN_INFO "task_struct size %d\n", sizeof(struct task_struct));
    if(!my_dir)
    {
        printk(KERN_ERR "+++ error proc_mkdir\n");
        return -ENOMEM;
    }
    my_file = proc_create(PROC_FILE, 0666, my_dir, &my_proc_ops);
    if(!my_file){
        printk(KERN_ERR "+++ error proc_create\n");
        remove_proc_entry(PROC_DIR, NULL);
        return -ENOMEM;
    }
    my_symblink = proc_symlink(PROC_LINK, NULL, "/proc/" PROC_DIR "/" PROC_FILE);
    if(!my_symblink)
    {
        printk(KERN_ERR "+++ error proc_symlink\n");
        remove_proc_entry(PROC_FILE, my_dir);
        remove_proc_entry(PROC_DIR, NULL);
        return -ENOMEM;
    }

    printk(KERN_INFO "+++ module loaded\n");
    return 0;
}

static void __exit md_exit(void)
{
    remove_proc_entry(PROC_LINK, NULL);
    remove_proc_entry(PROC_FILE, my_dir);
    remove_proc_entry(PROC_DIR, NULL);

    printk(KERN_INFO "+++ module unloaded");
}

module_init(md_init);
module_exit(md_exit);
