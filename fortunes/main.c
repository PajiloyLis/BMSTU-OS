#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bugakov Ivan");

#define DIR "fortune_dir"
#define FILE "fortune"
#define SYMLINK "fortune_link"

#define MESSAGE_SIZE 256
#define COOKIE_BUF_SIZE PAGE_SIZE

static struct proc_dir_entry *my_dir = NULL;
static struct proc_dir_entry *my_file = NULL;
static struct proc_dir_entry *my_symblink = NULL;

static char pid_str[16] = {0};

static ssize_t fortune_read(struct file *file, char __user* buf, size_t count, loff_t *ppos) {
    printk(KERN_INFO "+++ call fortune_read");
    char msg[MESSAGE_SIZE] = {0};
    int msg_len;
    pid_t pid;
    struct task_struct *task;
    if(*ppos > 0)
        return 0;


    if(kstrtoint(pid_str, 10, &pid) != 0)
    {
        printk(KERN_INFO "+++ error: kstrtoint");
        return -EINVAL;
    }

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(!task){
        rcu_read_unlock();
        printk(KERN_INFO "+++ error: pid_task");
        return -ESRCH;
    }

    snprintf(msg, MESSAGE_SIZE, "pid: %d\ncomm: %s\nstatic prio^ %d\nflags 0x%x\n", pid, task->comm, task->static_prio, task->flags);

    msg_len = strlen(msg);

    if(copy_to_user(buf, msg, msg_len))
        return -EFAULT;

    *ppos = msg_len;
    printk(KERN_INFO "+++ message symbols: %d\n", msg_len);
    return msg_len;
}

static int fortune_open(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "+++ call fortune_open");
    return 0;
}

static int fortune_release(struct inode* inode, struct file* file){
    printk(KERN_INFO "+++ call fortune_release");
    return 0;
}

static ssize_t fortune_write(struct file* file, const char __user* buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "+++ call fortune_write");

    if(count >= sizeof(pid_str))
        return -EINVAL;

    if(copy_from_user(pid_str, buf, count))
        return -EFAULT;

    pid_str[count] = 0;
    printk(KERN_INFO "+++ pid: %s", pid_str);
    return count;
}

static struct proc_ops fops = {
    .proc_read = fortune_read,
    .proc_write = fortune_write,
    .proc_open = fortune_open,
    .proc_release = fortune_release
};


static int __init my_init(void)
{
    printk(KERN_INFO "+++ load module");
    my_dir = proc_mkdir(DIR, NULL);
    if(!my_dir)
    {
        printk(KERN_ERR "+++ Error proc_mkdir");
        return -ENOMEM;
    }

    my_file = proc_create(FILE, 0666, my_dir, &fops);
    if(!my_file)
    {
        printk(KERN_INFO "+++ error proc_create");
        remove_proc_entry(DIR, NULL);
        return -ENOMEM;
    }

    my_symblink = proc_symlink(SYMLINK, NULL, "/proc/" DIR "/" FILE);
    if(!my_symblink)
    {
        printk(KERN_INFO "+++ error: proc_symblink");
        remove_proc_entry(FILE, my_dir);
        remove_proc_entry(DIR, NULL);
        return -ENOMEM;
    }

    return 0;
}

static void __exit my_exit(void)
{
    remove_proc_entry(SYMLINK, NULL);
    remove_proc_entry(FILE, my_dir);
    remove_proc_entry(DIR, NULL);

    printk(KERN_INFO "+++ unload");
}

module_init(my_init);
module_exit(my_exit);