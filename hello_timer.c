#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/fs.h>
#define DELAY 3
#define TEXT "Hello, world!"

static unsigned long times = 0;
static unsigned char timer_exists = 0;

static int timer_init(void);
static void timer_exit(void);

static ssize_t times_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t times_store(struct kobject *, struct kobj_attribute *, const char *, size_t);

static void repeat(unsigned long);



static struct kobj_attribute times_attrb =
    __ATTR(interval, 0664, times_show, times_store);

static struct kobject *times_obj = NULL;

static struct timer_list timer;

static ssize_t times_show(struct kobject *kobj, struct kobj_attribute *attr,
                          char *buf)
{
    return sprintf(buf, "%lu\n", times);
}

static ssize_t times_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    if (kstrtoul(buf, 10, &times) == -EINVAL) {
        return -EINVAL;
    }

    if (timer_exists) {
        del_timer(&timer);
    }

    timer_exists = 1;
    timer.data = times;
    timer.function = repeat;
    timer.expires = jiffies + DELAY * HZ;

    add_timer(&timer);

    return count;
}

static void repeat(unsigned long arg)
{
    unsigned long i = 0;

    if (!arg) {
        return;
    }

    for (i = 0; i < arg; ++i) {
        printk(KERN_INFO "%s\n", TEXT);
    }

    timer.expires = jiffies + DELAY * HZ;

    add_timer(&timer);
}

static int __init timer_init()
{

    init_timer_on_stack(&timer);

    times_obj = kobject_create_and_add("timer", NULL);
    if (!times_obj) {
        return -ENOMEM;
    }

    if (sysfs_create_file(times_obj, &times_attrb.attr)) {
        timer_exit();
        return -EINVAL;
    }

    return 0;
}

static void timer_exit()
{
    if (timer_exists) {
        del_timer(&timer);
    }

    if (times_obj) {
        kobject_put(times_obj);
    }
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IHAR ZHARUKAU");
