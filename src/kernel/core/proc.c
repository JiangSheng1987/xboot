/*
 * kernel/core/proc.c
 *
 * Copyright(c) 2007-2013 jianjun jiang <jerryjianjun@gmail.com>
 * official site: http://xboot.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <xboot.h>
#include <xboot/proc.h>

static struct proc_list_t __proc_list = {
	.entry = {
		.next	= &(__proc_list.entry),
		.prev	= &(__proc_list.entry),
	},
};
struct proc_list_t * proc_list = &__proc_list;

struct proc_t * proc_search(const char *name)
{
	struct proc_list_t * list;
	struct list_head * pos;

	if(!name)
		return NULL;

	for(pos = (&proc_list->entry)->next; pos != (&proc_list->entry); pos = pos->next)
	{
		list = list_entry(pos, struct proc_list_t, entry);
		if(strcmp(list->proc->name, name) == 0)
			return list->proc;
	}

	return NULL;
}

bool_t proc_register(struct proc_t * proc)
{
	struct proc_list_t * list;

	list = malloc(sizeof(struct proc_list_t));
	if(!list || !proc)
	{
		free(list);
		return FALSE;
	}

	if(!proc->name || !proc->read || proc_search(proc->name))
	{
		free(list);
		return FALSE;
	}

	list->proc = proc;
	list_add(&list->entry, &proc_list->entry);

	return TRUE;
}

bool_t proc_unregister(struct proc_t * proc)
{
	struct proc_list_t * list;
	struct list_head * pos;

	if(!proc || !proc->name)
		return FALSE;

	for(pos = (&proc_list->entry)->next; pos != (&proc_list->entry); pos = pos->next)
	{
		list = list_entry(pos, struct proc_list_t, entry);
		if(list->proc == proc)
		{
			list_del(pos);
			free(list);
			return TRUE;
		}
	}

	return FALSE;
}

static s32_t self_proc_read(u8_t * buf, s32_t offset, s32_t count)
{
	struct proc_list_t * list;
	struct list_head * pos;
	s8_t * p;
	s32_t len = 0;

	if((p = malloc(SZ_4K)) == NULL)
		return 0;

	len += sprintf((char *)(p + len), (const char *)"[proc]");
	for(pos = (&proc_list->entry)->next; pos != (&proc_list->entry); pos = pos->next)
	{
		list = list_entry(pos, struct proc_list_t, entry);
		len += sprintf((char *)(p + len), (const char *)"\r\n %s", list->proc->name);
	}

	len -= offset;

	if(len < 0)
		len = 0;

	if(len > count)
		len = count;

	memcpy(buf, (u8_t *)(p + offset), len);
	free(p);

	return len;
}

static struct proc_t self_proc = {
	.name	= "proc",
	.read	= self_proc_read,
};

static __init void self_proc_init(void)
{
	proc_register(&self_proc);
}

static __exit void self_proc_exit(void)
{
	proc_unregister(&self_proc);
}

core_initcall(self_proc_init);
core_exitcall(self_proc_exit);
