#include "Qbject.h"
QNode root;

QResult qkr_main(KernelGlobalData * kgd) {
	root.type = QNODE_TYPE_ROOT;
	root.CreateQbjectFunction = NULL;
	root.QbjectReadFunction = NULL;
	root.QbjectWriteFunction = NULL;
	root.left_child = NULL;
	root.rigth_sibling = NULL;
}


Qbject * OpenQbject(char * path, ACCESS access)
{
	char *start;
	char *end;
	start = end = path;

	return NULL;
}

Qbject * create_qbject(char * path, ACCESS access)
{
	return NULL;
}

Qbject * allocate_qbject(uint32 content_size)
{
	return NULL;
}
