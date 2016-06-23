#include "Qbject.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
// TODO: Use better synchronization.
QNode root;
SpinLock treelock;
QResult qkr_main(KernelGlobalData * kgd) {
	root.type = QNODE_TYPE_ROOT;
	root.create_qbject = NULL;
	root.read = NULL;
	root.write = NULL;
	root.left_child = NULL;
	root.rigth_sibling = NULL;
	spin_init(&treelock);
	root.name[0] = '\0';
	return QSuccess;
}
static BOOL name_match(QNode *qnode, char *s, uint32 len) {
	return ((len < MAX_QNODE_NAME_LEN) && (memcmp(qnode->name, s, len) == 0) && (qnode->name[len] == '\0'));
}

QHandle create_qbject(char * path, ACCESS access)
{
	spin_lock(&treelock);
	char *start,*end;
	start = path;
	while (*start == '/') {
		start++;
	}
	QNode *cur;
	cur = &root;
	QHandle h = NULL;
	while (1) {
		// first - try to create the Qbject using the assigned crate_qbject function if any.
		if (cur->create_qbject) {
			h = cur->create_qbject(cur->qnode_context,start, access, 0); // first chance
			if (h) {
				break;
			}
		}
		
		// then try to open child
		end = start;
		while ((*end != '\0') && (*end != '/')) {
			end++;
		}
		if (start == end) {
			break;
		}
		QNode *child;
		for (child = cur->left_child; child != NULL; child = child->rigth_sibling){
			if (name_match(child, start, end - start)) {
				start = end;
				while (*start == '/') {
					start++;
				}
				break;
			}
		}
		if (!child) {
			break;
		}
		cur = child;
	};

	if (cur->create_qbject) {
		h = cur->create_qbject(cur->qnode_context,start, access, CREATE_QBJECT_FLAGS_SECOND_CHANCE); // second chance
	}

	spin_unlock(&treelock);

	if (h) {
		((Qbject*)h)->associated_qnode = cur;
	}

	return h;
}

QResult read_qbject(QHandle qhandle, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read)
{
	return ((Qbject*)qhandle)->associated_qnode->read((Qbject*)qhandle, buffer, position, num_of_bytes_to_read, res_num_read);
}

QResult write_qbject(QHandle qhandle, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_write)
{
	return ((Qbject*)qhandle)->associated_qnode->write((Qbject*)qhandle, buffer, position, num_of_bytes_to_read, res_num_write);
}

QResult create_qnode(char * path)
{
	spin_lock(&treelock);
	char *start, *end;
	start = path;
	while (*start == '/') {
		start++;
	}
	QNode *cur;
	cur = &root;
	while (1) {
		end = start;
		while ((*end != '\0') && (*end != '/')) {
			end++;
		}
		if (start == end) {
			break;
		}
		if (end - start >= MAX_QNODE_NAME_LEN) {
			spin_unlock(&treelock);
			return QFail;
		}
		QNode *child;
		for (child = cur->left_child; child != NULL; child = child->rigth_sibling) {
			if (name_match(child, start, end - start)) {
				start = end;
				while (*start == '/') {
					start++;
				}
				break;
			}
		}
		if (!child) {
			child = kheap_alloc(sizeof(QNode));
			if (child == 0) {
				spin_unlock(&treelock);
				return QFail;
			}
			child->type = QNODE_TYPE_GENERIC;
			strncpy(child->name, start, end - start);
			child->name[end - start] = '\0';
			child->qnode_context = NULL;
			child->create_qbject = NULL;
			child->read = NULL;
			child->write = NULL;
			child->left_child = NULL;
			child->rigth_sibling = cur->left_child;
			cur->left_child = child;
		}
		start = end;
		while (*start == '/') {
			start++;
		}
		cur = child;
	};
	spin_unlock(&treelock);
	return QSuccess;
}

QResult set_qnode_attributes(char* path, QNodeAttributes* attr) {
	spin_lock(&treelock);
	char *start, *end;
	start = path;
	while (*start == '/') {
		start++;
	}
	QNode *cur;
	cur = &root;
	while (1) {
		end = start;
		while ((*end != '\0') && (*end != '/')) {
			end++;
		}
		if (start == end) {
			break;
		}
		if (end - start >= MAX_QNODE_NAME_LEN) {
			spin_unlock(&treelock);
			return QFail;
		}
		QNode *child;
		for (child = cur->left_child; child != NULL; child = child->rigth_sibling) {
			if (name_match(child, start, end - start)) {
				start = end;
				while (*start == '/') {
					start++;
				}
				break;
			}
		}
		if (!child) {
			spin_unlock(&treelock);
			return QFail;
		}
		start = end;
		while (*start == '/') {
			start++;
		}
		cur = child;
	};
	cur->qnode_context = attr->qnode_context;
	cur->create_qbject = attr->create_qbject;
	cur->read = attr->read;
	cur->write = attr->write;
	cur->get_property = attr->get_property;
	spin_unlock(&treelock);
	return QSuccess;
}

QHandle allocate_qbject(uint32 content_size)
{
	return kheap_alloc(sizeof(Qbject) + content_size);
}
