#include <linux/kernel.h>
#include <linux/types.h>
#include <uapi/asm-generic/errno-base.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/sched.h>
#include "process_ancestors.h"


asmlinkage long sys_process_ancestors(
	struct process_info info_array[],
	long size,
	long *num_filled)
{
	struct task_struct *cur_task = current;
	struct process_info info;
	struct list_head *cur_child;
	struct list_head *cur_sibling;
	struct list_head *addr;
	long count_filled = 0;
	long count_children = 0;
	long count_sibling = 0;

	if(size <= 0){
		return -EINVAL;
	}
	while(1){
		info.pid = cur_task->pid;
		strncpy(info.name, cur_task->comm, ANCESTOR_NAME_LEN);
		info.state = cur_task->state;
		info.uid = cur_task->cred->uid.val;
		info.nvcsw = cur_task->nvcsw;
		info.nivcsw = cur_task->nivcsw;

		addr = &(cur_task->children);
		cur_child = &(cur_task->children);
		count_children = 1;
		while(cur_child->next != addr){
			cur_child = cur_child->next;
			count_children++;
		}

		addr = &(cur_task->sibling);
		cur_sibling = &(cur_task->sibling);
		count_sibling = 1;
		while(cur_sibling->next != addr){
			cur_sibling = cur_sibling->next;
			count_sibling++;
		}

		info.num_children = count_children;
		info.num_siblings = count_sibling;

		if(copy_to_user(info_array, &info, sizeof(struct process_info)))
			return -EFAULT;
		info_array++;
		count_filled++;
		// printk("pid of ancestor%ld: %ld\n", count_filled, info.pid);
		if(cur_task->parent == cur_task || count_filled >= size)
			break;
		else
			cur_task = cur_task->parent;
	}
	if(copy_to_user(num_filled, &count_filled, sizeof(long)))
		return -EFAULT;
	return 0;
}