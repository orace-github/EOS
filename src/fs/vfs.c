/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <fs/vfs.h>
static vfs_t __vfs;
extern process_t* current;
static fd_hashtable_t __fd_hashtable;
u8_t __absolute[PATH_MAX_LEN];

static inline int testchar(u8_t __caracter)
{
    switch(__caracter)
    {
        case '\'': return 0;
        case '{': return 0;
        case '(': return 0;
        case '[': return 0;
        case '-': return 0;
        case '|': return 0;
        case '\\': return 0;
        case '^': return 0;
        case ')': return 0;
        case ']': return 0;
        case '+': return 0;
        case '=': return 0;
        case '}': return 0;
        case '\t': return 0;
        case '*': return 0;
        case ',': return 0;
        case ';': return 0;
        case '?': return 0;
        case ' ': return 0;
        case '<': return 0;
        case '>': return 0;
        default: return 1;
    }
    return 1;
}

static inline int testpath(u8_t* __path)
{
    int __len = strlen(__path);
    int i = 0;
    for(i = 0; i < __len && testchar(__path[i]); i++);
    return (i == __len) ? 1 : 0;
}

int path_clean(u8_t* __path)
{
    int __plen = strlen(__path);
    int i = 0, j = 0;
    // Clear previous value of __absolute
    memset(__absolute,0,PATH_MAX_LEN);
    if(!testpath(__path) || __plen > PATH_MAX_LEN)
        return 0;
     // If path is realtive ; convert it in absolute path
    if(__path[0] != '/')
    {
        int __len = strlen(current->cwd);
        strncpy(__absolute,current->cwd,__len);
        strncpy(__absolute+__len,__path,PATH_MAX_LEN-__len);
        __plen = strlen(__absolute);
        strncpy(__path,__absolute,__plen);
    }
    for(i = 0; i < __plen; i++)
	{
		while(__path[i] == '/' && i < __plen)
		{
			if(__path[i+1] == '.')
			{
				if(__path[i+2] == '.')
				{
					if(__path[i+3] == '/' || __path[i+3] == '\0')
					{
						if(i)
						{
							// find index of previous slash
							for(j = i-1; j >= 0 && __path[j] != '/'; j--);
							if(__path[j] != '/')
								return 0;
						}
						else
							j = i;
						strncpy(__path+j,__path+i+3,PATH_MAX_LEN-j);
						__plen -= (i+3-j);
						i = j;
					}
					else
						i += 3;
				}
				else if(__path[i+2] == '/' || __path[i+2] == '\0')
				{
					strncpy(__path+i,__path+i+2,PATH_MAX_LEN-i);
					__plen -= 2;
				}
				else
					i++;
			}
			else if(__path[i+1] == '/' || __path[i+1] == '\0')
			{
				strncpy(__path+i,__path+i+1,PATH_MAX_LEN-i);
				__plen--;
			}
			else
				break;
		}
	}
	return 1;
}

dentry_t* mount_point(u8_t* __path, u8_t** __lpath)
{
	dentry_t* __mount = __vfs.root;
	dentry_t* __dentry = __vfs.root;
	// make sure that path is great
	strncpy(__vfs.path,__path,PATH_MAX_LEN);
	// make sure that path is clean
	if(!path_clean(__vfs.path))
		return NULL;
	// slash index
	u8_t* __slash = __vfs.path + 1;
	*__lpath = __slash;
	if(__slash[0] == '\0')
		return __mount;
	// dentry base name
	u8_t __name[DNAME_MAX_LEN];
	while(__slash != NULL && __slash[0] != '\0')
	{
		// next slash index
		u8_t* __nslash = strchr(__slash,'/');
		// if path is ex: /home/data
		if(!__nslash)
		{
			int __len = strlen(__slash);
			strncpy(__name,__slash,__len + 1);
			__dentry = __dentry->lookup(__dentry,__name);
			if(!__dentry)
				return __mount;
			// update slash
			__slash = __nslash;
			if(__dentry->mounted)
			{
				__mount = __dentry;
				*__lpath = __slash;
			}
		}
		else
		{
			int __len = (int)(__nslash - __slash);
			strncpy(__name,__slash,__len + 1);
			__dentry = __dentry->lookup(__dentry,__name);
			if(!__dentry)
				return __mount;
			// update slash
			__slash = __nslash + 1;
			if(__dentry->mounted)
			{
				__mount = __dentry;
				*__lpath = __slash;
			}
		}
	}
	return __mount;	
}

void path(u8_t* __path, path_t* __vpath)
{
	// ## substract each entry of file path ##
	if(!__path || __path[0] == '\0' || __path[1] == '\0')
		return;

	u8_t* __slash = strchr(__path,'/');
	if(!__slash)
	{
		path_entry_t* __entry = sys_alloc(sizeof(path_entry_t));
		__vpath->size++;
		int __len = strlen(__path);
		__entry->name = sys_alloc(__len + 1);
		strncpy(__entry->name,__path,__len + 1);
		__entry->index = __vpath->size;
		list_insert(&__vpath->e_list,&__entry->head);
		return;
	}
	else
	{
		path_entry_t* __entry = sys_alloc(sizeof(path_entry_t));
		__vpath->size++;
		int __len = (int)(__slash - __path);
		__entry->name = sys_alloc(__len + 1);
		strncpy(__entry->name,__path,__len + 1);
		__entry->index = __vpath->size;
		list_insert(&__vpath->e_list,&__entry->head);
		__path = __slash + 1;
		path(__path,__vpath);
	}
}

void destroy_path(path_t* __path)
{
	// ## destroy each entry of file path ##
	struct list_head* __it = NULL, *__next = NULL;
	list_for_each_safe(__it,__next,&__path->e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		sys_free(__entry->name);
		list_del(&__entry->head);
		sys_free(__entry);
	}
	sys_free(__path);
}

fd_t* fileid()
{
	// make sure that fd hashtable is good
	init_fdhashtable(&__fd_hashtable);
	// check if current process is good
	if(!current)
	{
		#ifdef __DEBUG__
			kprintf("PANIC: no current process.\n");
		#endif //__DEBUG__
		return NULL;
	}

	for(int i = 0; i < MAX_FILE; i++)
		if(current->fd[i] == -1)
		{
			fd_t* __fd = sys_alloc(sizeof(fd_t));
			__fd->fd = current->id + i;
			__fd->file = NULL;
			__fd->process = current;
			__fd->index = i;
			__fd_hashtable.insert(__fd,&__fd_hashtable);
			current->fd[i] = __fd->fd;
			return __fd;
		}
	return NULL;
}

int init_vfs(dentry_t* __root, dev_t* __dev)
{
	init_fdhashtable(&__fd_hashtable);
	#ifdef __DEBUG__
		kprintf("Installing VFS...");
	#endif //__DEBUG__
	__vfs.root = __root;
	if(!efs2_mkfs(__dev->dev_dentry) ||
		!efs2_mount(__dev->dev_dentry,__vfs.root,READ_O|WRITE_O))
	{
		kprintf("[failed]\n");
		return 0;
	}
	__vfs.semid = semaphore_open(0x01 /* mutual exclusion */,IPC_READ);
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
	return 1;
}

int vfs_mknode(u8_t* __path, u16_t __type)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount)
		return 0;
	// check if file system is writable
	if(!IS_WR_O(__mount->d_fsmode))
		return 0;

	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->mknode(__mount,__entry->name,__entry->index == __listpath.size ? __type : DIRECTORY_FILE);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;
		}
	}

	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return 1;
}

int vfs_open(u8_t* __path, u16_t __mode)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount)
	{
		#ifdef __DEBUG__
			kprintf("PANIC: VFS OPEN ERROR [1]\n");
		#endif //__DEBUG__
		return -1;
	}

	if((!IS_WR_O(__mount->d_fsmode) && IS_WR_O(__mode)) || (!IS_RD_O(__mount->d_fsmode) && IS_RD_O(__mode)))
	{
		#ifdef __DEBUG__
			kprintf("PANIC: VFS OPEN ERROR [2]\n");
		#endif //__DEBUG__
		return -1;	
	}
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			#ifdef __DEBUG__
				kprintf("PANCI: VFS OPEN ERROR [4]\n");
			#endif //__DEBUG__
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return -1;
		}
	}
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	// get a new file descriptor
	if(__mount->open(__mount,__mode))
	{
		fd_t* __fd = fileid();
		if(!__fd)
			return -1;
		__fd->file = __mount;
		return __fd->fd;
	}
	return -1;
}

int vfs_write(int __id, void* __buf, u32_t __bytes)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	semaphore_wait(__vfs.semid);
	// call local function write of file
	int __ret = __fd->file->write(__fd->file,__buf,__bytes);
	semaphore_signal(__vfs.semid);
	return __ret;		
}

int vfs_read(int __id, void* __buf, u32_t __bytes)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	semaphore_wait(__vfs.semid);
	// call local function write of file
	int __ret = __fd->file->read(__fd->file,__buf,__bytes);
	semaphore_signal(__vfs.semid);
	return __ret;		
}

int vfs_exist(u8_t* __path)
{
	// call vfs_lookup 
	return (vfs_lookup(__path) != NULL);
}

int vfs_mount(u8_t* __devpath, u8_t* __fpath, u16_t __mode)
{
	// find both dentry and device dentry
	dentry_t* __dentry = vfs_lookup(__fpath);
	dentry_t* __devdentry = vfs_lookup(__devpath);

	// make sure that both memory location are safe
	if(!__dentry || !__devdentry)
		return 0;

	// call local function mount of dentry
	return __dentry->mount(__devdentry,__dentry,__mode);
}

int vfs_umount(u8_t* __path)
{
	// find dentry
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry || !__dentry->mounted)
		return 0;
	// call local function umount of dentry
	return __dentry->umount(__dentry);
}

int vfs_mkdir(u8_t* __path)
{
	// call vfs_mknode with type:DIRECTORY_FILE
	return vfs_mknode(__path,DIRECTORY_FILE);
}

int vfs_rndir(u8_t* __path, u8_t* __name)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount)
		return 0;
	if(!IS_WR_O(__mount->d_fsmode))
		return 0;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;
		}
	}
	dentry_t* __ret = __mount->rndir(__mount,__name);
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return (__ret != NULL);
}

int vfs_rmdir(u8_t* __path, u8_t __option)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount)
		return 0;
	if(!IS_WR_O(__mount->d_fsmode))
		return 0;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;
		}
	}

	int __ret = __mount->rmdir(__mount,__option);
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return __ret;
}

int vfs_chdir(u8_t* __path)
{
	// find dentry
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry || !IS_DIRECTORY_FILE(__dentry->d_mode))
		return 0;
	int __len = strlen(__vfs.path);
	if(__vfs.path[__len-1] != '/')
		__vfs.path[__len] = '/';
	// copy find inside process working directory
	strncpy(current->cwd,__vfs.path,PATH_MAX_LEN);
	return 1;
}

int vfs_chmod(u8_t* __path, u16_t __mode)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount)
		return 0;
	if(!IS_WR_O(__mount->d_fsmode))
		return 0;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;
		}
	}

	dentry_t* __ret = __mount->chmod(__mount,__mode);
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return (__ret != NULL);
}

int vfs_lock(u8_t* __path, u8_t __flags)
{
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry)
		return 0;
	return __dentry->lock(__dentry,__flags);
}

int vfs_unlock(u8_t* __path)
{
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry)
		return 0;
	return __dentry->unlock(__dentry);
}

int vfs_close(int __id)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	// free process entry
	__fd->process->fd[__fd->index] = -1;
	// remove and free fd from hashtable
	__fd_hashtable.remove(__fd,&__fd_hashtable);
	// call local close function
	int __eax = __fd->file->close(__fd->file);
	sys_free(__fd);
	return __eax;
}

int vfs_eof(int __id)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	// call local function eof of file
	return __fd->file->eof(__fd->file);	
}

int vfs_link(u8_t* __fpath, u8_t* __lpath)
{
	// find file to be link
	dentry_t* __file = vfs_lookup(__fpath);
	if(!__file)
		return 0;
	
	u8_t* __localpath = NULL;
	dentry_t* __parent = NULL;
	u8_t* __name = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__lpath,&__localpath);
	// check if mount point is safe
	if(!__mount || !IS_RD_O(__mount->d_fsmode))
		return 0;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		// find parent dentry and base name inside __listpath
		if(__entry->index == __listpath.size)
		{
			__parent = __mount;
			__name = __entry->name;
		}
		__mount = __mount->lookup(__mount,__entry->name);
		// make sure that link path is great
		if(!__mount && __entry->index != __listpath.size)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;		
		}
	}
	dentry_t* __link = __file->link(__file,__parent,__name);
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return (__link != NULL);	
}

int vfs_unlink(u8_t* __path)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount || !IS_RD_O(__mount->d_fsmode))
		return 0;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return 0;
		}
	}
	int __ret = __mount->unlink(__mount);
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	return __ret;
}

int vfs_touch(u8_t* __path)
{
	// call vfs_mknode with type:ORDINARY_FILE
	return vfs_mknode(__path,ORDINARY_FILE);
}

int vfs_stat(u32_t __id, stat_t* __stat)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	// call local function stat of file
	return __fd->file->stat(__fd->file,__stat);	
}

int vfs_seek(u32_t __id, u32_t __off, u32_t __pos)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	// call local function seek of file
	return __fd->file->seek(__fd->file,__off,__pos);	
}

int vfs_flush(u32_t __id)
{
	// find from hashtable fd
	fd_t* __fd = __fd_hashtable.find(__id,&__fd_hashtable);
	if(!__fd)
		return 0;
	// call local function flush of file
	return __fd->file->flush(__fd->file);
}

char* vfs_readfile(u8_t* __path)
{
	// find dentry
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry)
		return NULL;
	// call local function readfile
	return __dentry->readfile(__dentry);
}

dentry_t* vfs_lookup(u8_t* __path)
{
	u8_t* __localpath = NULL;
	// find the mount point
	dentry_t* __mount = mount_point(__path,&__localpath);
	// check if mount point is safe
	if(!__mount || !IS_RD_O(__mount->d_fsmode))
		return NULL;
	// split __path into a list
	static path_t __listpath; INIT_PATH(&__listpath);
	path(__localpath,&__listpath);
	// hold semaphore
	semaphore_wait(__vfs.semid);
	struct list_head* __it = NULL;
	list_for_each(__it,&__listpath.e_list.head)
	{
		path_entry_t* __entry = list_entry(__it,path_entry_t,head);
		__mount = __mount->lookup(__mount,__entry->name);
		if(!__mount)
		{
			semaphore_signal(__vfs.semid);
			destroy_path(&__listpath);
			return NULL;
		}
	}
	// release semaphore
	semaphore_signal(__vfs.semid);
	// destroy __lispath
	destroy_path(&__listpath);
	// get a new file descriptor
	return __mount;
}

void vfs_sync()
{

}
