#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define PROCFS_FILE "num_pagefaults"

static struct proc_dir_entry* proc_entry;

static int pagefault_write(struct seq_file* m, void* n) {
    unsigned long* ret;

    ret = kmalloc(NR_VM_EVENT_ITEMS * sizeof(unsigned long), GFP_KERNEL);

    if (!ret) {
        return 1;
    }

    all_vm_events(ret);

    seq_printf(m, "%lu\n", ret[PGFAULT]);

    kfree(ret);

    return 0;
}

static int pagefault_open(struct inode* node, struct file* file) {
    return single_open(file, pagefault_write, NULL);
}

static int __init pagefault_init(void) {
    static struct file_operations fops = {
        .owner = THIS_MODULE,
        .llseek = seq_lseek,
        .read = seq_read,
        .open = pagefault_open,
        .release = seq_release,
    };

    proc_entry = proc_create(PROCFS_FILE, 0, NULL, &fops);

    if (proc_entry == NULL) {
        return 1;
    }

    return 0;
}

static void __exit pagefault_exit(void) {
    proc_remove(proc_entry);    
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason B");
MODULE_DESCRIPTION("Lab 1 CSU Chico CSCI 340");

module_init(pagefault_init);
module_exit(pagefault_exit);
