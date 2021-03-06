/*
 * RandomScheduler.cpp
 *
 *  Created on: Jun 29, 2012
 *      Author: tcpan
 */

#include "RandomScheduler.h"

#include <stdlib.h>

namespace cci {
namespace rt {

RandomScheduler::RandomScheduler(std::vector<int> &_roots, std::vector<int> &_leaves) :
			Scheduler_I(_roots, _leaves) {
}

RandomScheduler::RandomScheduler(bool _root, bool _leaf) :
	Scheduler_I(_root, _leaf) {
}

int RandomScheduler::getRootFromLeaf(int leafId) {
//	std::ostream_iterator<int> os(std::cout, ",");
//	copy(roots.begin(), roots.end(), os);
//	std::cout << std::endl;

	int size = roots.size();
	if (size == 0) return -1;
	else if (size == 1) return roots[0];
	else return roots[rand() % size];
}
int RandomScheduler::getLeafFromRoot(int rootId) {
//	std::ostream_iterator<int> os(std::cout, ",");
//	copy(leaves.begin(), leaves.end(), os);
//	std::cout << std::endl;

	int size = leaves.size();
	if (size == 0) return -1;
	else if (size == 1) return leaves[0];
	else return leaves[rand() % size];
}
} /* namespace rt */
} /* namespace cci */
