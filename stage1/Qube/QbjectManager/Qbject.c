#include "Qbject.h"
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
	spin_init(treelock);
	root.name[0] = '\0';
}
static BOOL name_match(QNode *qnode, char *s, uint32 len) {
	return ((len < MAX_QNODE_NAME_LEN) && (memcmp(qnode->name, s, len) == 0) && (qnode->name[len] == '\0'));
}
QHandle create_qbject(char * path, ACCESS access)
{
	spin_lock(treelock);
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
			h = cur->create_qbject(start, access, 0); // first chance
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
	};

	if (cur->create_qbject) {
		QHandle h = cur->create_qbject(start, access, CREATE_QBJECT_FLAGS_SECOND_CHANCE); // second chance
	}

	spin_unlock(treelock);

	if (h) {
		((Qbject*)h)->associated_qnode = cur;
	}

	return h;
}

QHandle allocate_qbject(uint32 content_size)
{
	return kalloc(sizeof(Qbject) + content_size);
}
