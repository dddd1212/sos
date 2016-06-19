#include "disk_manager.h"
// TODO: Use better synchronization.
QResult qkr_main(KernelGlobalData * kgd) {
	QResult res;
	res = create_qnode("HDs/HD0");
	if (res != QSuccess) {
		return QFail;
	}

}