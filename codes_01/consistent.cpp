/*
* =====================================================================================
*
*       Filename:  demo.cpp
*
*    Description:  һ���Թ�ϣ��ʾ��
*
*        Version:  1.0
*        Created:  01/12/2016 10:42:23 AM
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  seth.hg@gmail.com
*
* =====================================================================================
*/
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "consistent.h"

#define ID_MAX  ((uint32_t)-1 + (uint64_t)1)    // 2^32

// �ȽϽ��
enum CMP_RES_T {
	EQ = 0,
	MOVE_LEFT = -1,
	MOVE_RIGHT = 1,
};

CMP_RES_T cmp_vnode_entry(const VirtualNode& dst, const uint32_t& src)
{
	if (src < dst.hash_id)
	{
		return MOVE_LEFT;
	}

	if (src > dst.hash_id)
	{
		return MOVE_RIGHT;
	}

	return EQ;
}

// ���ҽڵ��ʱ�������ڽ��ĵ�һ���ڵ��ǲ��Ҵ��ڵ��ڸ�Ԫ�صĵ�һ��
//
// ���ֲ����㷨�����������в�С��val�ĵ�һ��Ԫ�ص��±�
template<typename T1, typename T2>
int bin_search(std::vector<T1> array, const T2& val, CMP_RES_T(*cmp_func)(const T1&, const T2&))
{
	int left = 0, right = array.size() - 1;
	int mid;
	CMP_RES_T res;

	while (true)
	{
		if (left > right)
		{
			// ���������û�е���val��ֵ���򷵻��Ҳ���±꣬
			// ��������>val����СԪ��
			return left % array.size();
		}
		mid = (left + right) / 2;
		res = cmp_func(array[mid], val);
		if (res == EQ)
		{
			return mid;
		}
		if (MOVE_LEFT == res)
		{
			right = mid - 1;
		}
		else if (MOVE_RIGHT == res)
		{
			left = mid + 1;
		}
	}
}

// ���캯��
ConsistentHashRouter::ConsistentHashRouter()
{
	virtual_nodes.clear();
	has_init = false;
	hash_func = NULL;
	n_vnodes_per_node = 1;

	//realCntMap.clear();
}

static inline uint32_t node_hash(uint32_t node_id)
{

}

/*
	����hash��������0��2^32-1�����ȷֲ���ڵ�
*/
void ConsistentHashRouter::PopulateVirtualNodes(uint32_t node_id, std::vector<VirtualNode>& nodes, int num, hash_func_t func)
{
	uint64_t step = ID_MAX / (num + 1); 
	// TODO: ����hash��ʽ��ȡ��ʼhash_id
	std::stringstream ss;
	ss << node_id;
	uint64_t id = (uint64_t)func(ss.str());

	// ��������ڵ�
	for (int i = 0; i < num; i++)
	{
		VirtualNode new_node;
		new_node.hash_id = (uint32_t)((id + i * step) % ID_MAX);
		new_node.node_id = node_id;
		nodes.push_back(new_node);
	}
}

// 
int ConsistentHashRouter::Init(std::vector<uint32_t>& nodes, hash_func_t func, int num)
{
	if (nodes.size() < 1)
	{
		return -1;
	}

	if (NULL == func)
	{
		return -1;
	}

	if (num < 1)
	{
		return -1;
	}

	n_vnodes_per_node = num;
	std::vector<uint32_t>::iterator it;
	for (it = nodes.begin(); it != nodes.end(); it++)
	{
		// Ϊÿ������ڵ㴴����ڵ�
		PopulateVirtualNodes((*it), virtual_nodes, n_vnodes_per_node, func);
	}
	// ����hash_id������ڵ��������
	sort(virtual_nodes.begin(), virtual_nodes.end());

	/*
	std::vector<VirtualNode>::iterator it2;
	for (it2 = virtual_nodes.begin(); it2 != virtual_nodes.end(); it2++)
	{
	std::cout << (*it2).hash_id << " " << (*it2).node_id << std::endl;
	}
	*/

	hash_func = func;
	has_init = true;
	return 0;
}

// ɾ��һ̨��ʵ����
int ConsistentHashRouter::AddNode(uint32_t id, hash_func_t func)
{
	if (false == has_init)
	{
		return -1;
	}

	std::vector<VirtualNode>::iterator it = virtual_nodes.begin();
	while (it != virtual_nodes.end())
	{
		if (it->node_id == id)
		{
			return -1;
		}
		++it;
	}

	PopulateVirtualNodes(id, virtual_nodes, n_vnodes_per_node, func);

	sort(virtual_nodes.begin(), virtual_nodes.end());

	return 1;
}

int ConsistentHashRouter::DelNode(uint32_t id)
{
	if (false == has_init)
	{
		return -1;
	}

	std::vector<VirtualNode> virtual_nodes_tmp;

	std::vector<VirtualNode>::iterator it = virtual_nodes.begin();
	while (it != virtual_nodes.end())
	{
		if (it->node_id != id)
		{
			virtual_nodes_tmp.push_back(*it);
		}
		++it;
	}
	virtual_nodes.clear();
	virtual_nodes = virtual_nodes_tmp;

	sort(virtual_nodes.begin(), virtual_nodes.end());
}

// ����һ��key, �ҵ����Ӧ����ʵ����
int ConsistentHashRouter::Route(const std::string& key, uint32_t& out)
{
	if (false == has_init)
	{
		return -1;
	}

	// 1. �����key��Ӧ��hashֵ
	uint32_t hash_val = hash_func(key);

	// 2. �ҵ�hashֵ��Ӧ������ڵ�
	int node_idx = bin_search(virtual_nodes, hash_val, cmp_vnode_entry);

	//printf("hash_val = %lu, node_idx = %d, vnode_id = %lu\n", hash_val, node_idx, virtual_nodes[node_idx].hash_id);
	out = virtual_nodes[node_idx].node_id;
#if 0
	// ��Ӧ����������1
	if (realCntMap.find(out) == realCntMap.end())
	{
		realCntMap[out] = 1;
	}
	else{
		realCntMap[out] += 1;
	}
#endif
	return 0;
}

