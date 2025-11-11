#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
fd_map::fd_map() : __base(256) {}
file_vnode* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd) { return (fd < 3) ? ctx->stdio_ptrs[fd] : fsptr->get_file(fd); }
filesystem::filesystem() : pipes{ 256UZ }, device_nodes{}, current_open_files{}, next_fd{ 3 }, blockdev{ nullptr } {}
filesystem::~filesystem() = default;
void filesystem::tie_block_device(block_device* dev) { blockdev = dev; }
std::string filesystem::get_path_separator() const noexcept { return std::string(path_separator()); }
vnode* filesystem::get_fd_node(int fd) { return current_open_files.find_fd(fd); }
void filesystem::register_fd(vnode* node) { next_fd = current_open_files.add_fd(node) + 1; }
const char* filesystem::path_separator() const noexcept { return "/"; }
file_vnode* filesystem::open_file(const char* path, std::ios_base::openmode mode, bool create) { return open_file(std::string(path), mode, create); }
file_vnode* filesystem::on_open(tnode* node) { return on_open(node, std::ios_base::in | std::ios_base::out); }
file_vnode* filesystem::on_open(tnode* node, std::ios_base::openmode) { if(file_vnode* fn = node->as_file(); fn->on_open()) { return fn; } return nullptr; }
file_vnode* filesystem::get_file(int fd) { return dynamic_cast<file_vnode*>(current_open_files.find_fd(fd)); }
directory_vnode* filesystem::get_directory(int fd) { return dynamic_cast<directory_vnode*>(current_open_files.find_fd(fd)); }
dev_t filesystem::get_dev_id() const noexcept { return xgdevid(); }
size_t filesystem::block_size() { return blockdev ? blockdev->sector_size() : physical_block_size; }
void filesystem::pubsyncdirs() { syncdirs(); }
filesystem::target_pair filesystem::get_parent(std::string const& path, bool create) { return get_parent(get_root_directory(), path, create); }
directory_vnode* filesystem::get_directory_or_null(std::string const& path, bool create) noexcept { try { return open_directory(path, create); } catch(...) { return nullptr; } }
bool filesystem::write_blockdev(uint64_t lba_dest, const void* src, size_t sectors) { return blockdev->write(lba_dest, src, sectors); }
bool filesystem::read_blockdev(void* dest, uint64_t lba_src, size_t sectors) { return blockdev->read(dest, lba_src, sectors); }
tnode* filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents)
{
	target_pair ogparent = get_parent(ogpath, false);
	target_pair tgparent = get_parent(tgpath, create_parents);
	if(ogparent.first->is_mount() || tgparent.first->is_mount())
		if(mount_vnode* mount = dynamic_cast<mount_vnode*>(ogparent.first); mount && mount == dynamic_cast<mount_vnode*>(tgparent.first))
			return mount->mounted->link(ogparent.second, tgparent.second, create_parents);
	return xlink(ogparent, tgparent);
}
file_vnode* filesystem::get_file_or_null(std::string const& path)
{
	try
	{
		target_pair parent		= get_parent(path, false);
		if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(parent.first)) return mount->mounted->get_file_or_null(parent.second);
		if(tnode* node			= parent.first->find(parent.second))
		{ 
			file_vnode* file = on_open(node);
			if(file) register_fd(file);
			return file;
		}
		return nullptr;
	}
	catch(...) { return nullptr; }
}
vnode* fd_map::find_fd(int i) noexcept
{
	iterator result = find(i);
	if(result != end()) { return *result; }
	return nullptr;
}
int fd_map::add_fd(vnode* node)
{
	while(contains(node->fd) && find_fd(node->fd) != node) node->fd++;
	insert(node);
	return node->fd;
}
void filesystem::close_file(file_vnode* fd) 
{
	if(fd && fd->is_file())
		on_close(fd);
	syncdirs();
}
void filesystem::on_close(file_vnode* fd)
{
	if(fd->is_device()) return; 
	fd->seek(0);
	int vid		= fd->vid();
	current_open_files.erase(vid);
	next_fd		= vid; 
	fd->on_close();
}
void filesystem::dldevnode(device_vnode* n)
{
	n->prune_refs();
	current_open_files.erase(n->vid());
	device_nodes.erase(*n); 
	syncdirs(); 
}
void filesystem::dlpipenode(vnode* fn)
{
	if(pipe_vnode* n = dynamic_cast<pipe_vnode*>(fn))
	{
		n->prune_refs();
		current_open_files.erase(n->vid());
		pipes.erase(n->vid());
		syncdirs();
	}
}
device_vnode* filesystem::lndev(std::string const& where, int fd, dev_t id, bool create_parents)
{
	target_pair parent		= get_parent(where, create_parents);
	if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(parent.first)) return mount->mounted->lndev(parent.second, fd, id, create_parents);
	if(parent.first->find(parent.second)) throw std::logic_error("[FS] cannot create link " + parent.second + " because it already exists"); 
	device_vnode* result	= mkdevnode(parent.first, parent.second, id, fd);
	register_fd(result);
	return result; 
}
bool filesystem::link_stdio(dev_t dev_id)
{
	if(!dreg[dev_id]) return false;    
	current_open_files.add_fd(lndev("/dev/stdin", dev_id, 0));
	current_open_files.add_fd(lndev("/dev/stdout", dev_id, 1));
	current_open_files.add_fd(lndev("/dev/stderr", dev_id, 2));
	return true;
}
bool filesystem::unlink(std::string const& what, bool ignore_nonexistent, bool dir_recurse)
{
	directory_vnode* pdir;
	std::string fname;
	if(ignore_nonexistent) try 
	{ 
		target_pair parent	= get_parent(what, false); 
		pdir				= parent.first;
		fname				= parent.second;
	} 
	catch(...) { return false; }
	else
	{
		target_pair parent	= get_parent(what, false); 
		pdir				= parent.first;
		fname				= parent.second;
	}
	if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(pdir)) return mount->mounted->unlink(fname, ignore_nonexistent, dir_recurse);
	return xunlink(pdir, fname, ignore_nonexistent, dir_recurse);
}
device_vnode* filesystem::mkdevnode(directory_vnode* parent, std::string const& name, dev_t id, int fd)
{
	device_stream* dev		= dreg[id];
	if(!dev) { throw std::invalid_argument("[FS] no device found with that id"); }
	device_vnode* result	= device_nodes.emplace(name, fd, dev, id).first.base();
	parent->add(result);
	register_fd(result);
	return result;
}
pipe_pair filesystem::mkpipe()
{
	int first_fd			= next_fd;
	while(current_open_files.contains(first_fd)) first_fd++;
	pipe_vnode* first_pipe	= std::addressof(pipes[first_fd]);
	size_t id				= first_pipe->pipe_id();
	register_fd(first_pipe);
	int second_fd			= next_fd;
	while(current_open_files.contains(second_fd)) second_fd++;
	pipe_vnode* second_pipe	= pipes.emplace(second_fd, id).first.base();
	register_fd(second_pipe);
	return pipe_pair{ .in = first_pipe, .out = second_pipe };
}
pipe_pair filesystem::mkpipe(directory_vnode*, std::string const& name)
{
	int first_fd            = next_fd;
	while(current_open_files.contains(first_fd)) first_fd++;
	pipe_vnode* first_pipe	= pipes.emplace(name, first_fd).first.base();
	size_t id               = first_pipe->real_id;
	register_fd(first_pipe);
	int second_fd           = next_fd;
	while(current_open_files.contains(second_fd)) second_fd++;
	pipe_vnode* second_pipe	= pipes.emplace(name, second_fd, id).first.base();
	register_fd(second_pipe);
	return pipe_pair{ .in = first_pipe, .out = second_pipe };
}
bool filesystem::xunlink(directory_vnode* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse)
{
	tnode* node = parent->find(what);
	if(!node) { if(!ignore_nonexistent) throw std::logic_error("[FS] cannot unlink " + what + " because it does not exist"); else return false; }
	if(node->is_directory() && (*node)->num_refs() <= 1)
	{ 
		if(!node->as_directory()->is_empty() && !dir_recurse) 
			throw std::logic_error("[FS] folder " + what + " cannot be deleted because it is not empty (call with dir_recurse = true to remove it anyway)");
		if(dir_recurse) 
			for(std::string s : node->as_directory()->lsdir())
				this->xunlink(node->as_directory(), s, true, true);
	}
	if(!parent->unlink(what)) return false;
	(*node)->rm_reference(node);
	if(!(*node)->has_refs())
	{

		if(node->is_file()) 
		{
			if(node->is_pipe())
				dlpipenode(node->ptr());
			else if(node->as_file()->is_device()) 
				dldevnode(dynamic_cast<device_vnode*>(node->as_file()));
			else dlfilenode(node->as_file()); 
		}
		if(node->is_directory()) dldirnode(node->as_directory()); }
	return true;
}
tnode* filesystem::xlink(target_pair ogparent, target_pair tgparent)
{
	if(ogparent.first->is_mount() || tgparent.first->is_mount()) throw std::invalid_argument("[FS] cannot create hardlink across mount point");
	tnode* node	= ogparent.first->find(ogparent.second);
	if(!node) throw std::runtime_error(std::string("[FS] path does not exist: ") + ogparent.first->name() + path_separator() + ogparent.second);
	if(tgparent.first->find(tgparent.second)) throw std::logic_error(std::string("[FS] target ") + tgparent.first->name() + path_separator() + tgparent.second + " already exists");
	if(!tgparent.first->link(node, tgparent.second)) throw std::runtime_error(std::string("[FS] failed to create link: ") + tgparent.first->name() + path_separator() + tgparent.second);
	return tgparent.first->find(tgparent.second);
}
filesystem::target_pair filesystem::get_parent(directory_vnode* start, std::string const& path, bool create)
{
	std::vector<std::string> pathspec = std::ext::split(path, path_separator());
	for(size_t i = 0; i < pathspec.size() - 1; i++)
	{
		if(pathspec[i].empty()) continue;
		tnode* cur	= start->find(pathspec[i]);
		if(!cur) 
		{
			if(create) 
			{
				directory_vnode* created	= mkdirnode(start, pathspec[i]);
				cur							= start->add(created);
				start						= created; 
			} 
			else { throw std::out_of_range("[FS] path " + pathspec[i] + " does not exist (use open_directory(\".../" + pathspec[i] + "\", true) to create it)"); } 
		}
		else if(cur->is_mount())
		{
			mount_vnode* mount = cur->as_mount();
			std::vector<std::string> rem(pathspec.begin() + i, pathspec.end());
			return target_pair(std::piecewise_construct, std::forward_as_tuple(mount), std::forward_as_tuple(std::ext::join(rem, mount->mounted->path_separator())));   
		}
		else if(cur->is_directory()) start = cur->as_directory();
		else throw std::invalid_argument("[FS] path is invalid because entry " + pathspec[i] + " is not a directory");
	}
	return target_pair(std::piecewise_construct, std::forward_as_tuple(start), std::forward_as_tuple(pathspec.back()));
}
vnode* filesystem::find_node(std::string const& path, bool ignore_links, std::ios_base::openmode mode)
{
	try
	{
		target_pair parent		= get_parent(path, false);
		if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(parent.first)) return mount->mounted->find_node(parent.second, ignore_links, mode);
		tnode* tn				= ignore_links ? parent.first->find_l(parent.second) : parent.first->find(parent.second);
		if(!tn) return nullptr;
		if(!current_open_files.contains(tn->ref().vid())) { register_fd(tn->ptr()); }
		if(tn->is_file() || tn->is_pipe()) { return on_open(tn, mode); }
		else return tn->ptr();
	}
	catch(std::out_of_range&) { return nullptr; }
}
file_vnode* filesystem::open_file(std::string const& path, std::ios_base::openmode mode, bool create)
{
	target_pair parent		= get_parent(path, false);
	if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(parent.first)) return mount->mounted->open_file(parent.second, mode, create);
	tnode* node				= parent.first->find(parent.second);
	if(node && node->is_directory()) throw std::logic_error("[FS] path " + path + " exists and is a directory");
	if(!node)
	{
		if(!create) throw std::out_of_range("[FS] file not found: " + path); 
		if(file_vnode* created = mkfilenode(parent.first, parent.second)) { node = parent.first->add(created); }
		else throw std::runtime_error("[FS] failed to create file: " + path); 
	}
	file_vnode* result		= on_open(node, mode);
	register_fd(result);
	result->current_mode	= mode;
	return result;
}
file_vnode* filesystem::get_file(std::string const& path)
{
	target_pair parent		= get_parent(path, false);
	if(tnode* node			= parent.first->find(parent.second))
	{ 
		file_vnode* file	= on_open(node);
		if(file) register_fd(file);
		return file;
	}
	else throw std::runtime_error("[FS] file not found: " + path);
}
directory_vnode* filesystem::open_directory(std::string const& path, bool create)
{
	if(path.empty()) return get_root_directory(); // empty path or "/" refers to root directory
	target_pair parent			= get_parent(path, create);
	if(mount_vnode* mount		= dynamic_cast<mount_vnode*>(parent.first)) return mount->mounted->open_directory(parent.second, create);
	tnode* node					= parent.first->find(parent.second);
	if(!node) 
	{ 
		if(create) 
		{
			directory_vnode* cn	= mkdirnode(parent.first, parent.second);
			if(!cn) throw std::runtime_error("[FS] failed to create " + path);
			node				= parent.first->add(cn);
			register_fd(cn);
			return node->as_directory(); 
		} 
		else throw std::out_of_range("[FS] path " + path + " does not exist (use open_directory(\"" + path + "\", true) to create it)"); 
	}
	else if(node->is_file()) throw std::invalid_argument("[FS] path " + path + " exists and is a file");
	else { register_fd(node->ptr()); return node->as_directory(); }
}
void filesystem::create_node(directory_vnode* from, std::string const& path, mode_t mode, dev_t dev)
{
	if(!from) from			= get_root_directory();
	target_pair parent		= get_parent(from, path, false);
	if(mount_vnode* mount	= dynamic_cast<mount_vnode*>(parent.first)) mount->mounted->create_node(mount->mounted->get_root_directory(), parent.second, mode, dev);
	else
	{
		if(parent.first->find(parent.second)) { throw std::domain_error("[FS] target " + path + " already exists"); }
		file_mode m(mode);
		vnode* result;
		if(m.is_directory())	{ result            = mkdirnode(parent.first, parent.second); result->mode = mode; }
		else if(m.is_chardev())	{ result            = mkdevnode(parent.first, parent.second, dev, next_fd++); result->mode = mode; }
		else if(m.is_fifo())	{ pipe_pair pipes   = mkpipe(parent.first, parent.second); pipes.in->mode = mode; pipes.out->mode = mode; return; }
		else					{ result            = mkfilenode(parent.first, parent.second); result->mode = mode; }
		if(!result)				{ throw std::runtime_error("[FS] failed to create node at " + path); }
		result->fsync();
		syncdirs();
	}
}
void filesystem::create_pipe(int fds[2])
{
	pipe_pair result	= mkpipe();
	if(result.in && result.out) { fds[0] = result.in->vid(); fds[1] = result.out->vid(); }
	else throw std::runtime_error("[FS] failed to create pipe");
}