/*
* =====================================================================================
*
*       Filename:  consistent_hash.cpp
*
*    Description:
*
*        Version:  1.0
*        Created:  2017-03-17 
*       Revision:  none
*       Compiler:  ms vs2013
*
*         Author:  https://github.com/doctording
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

// ���캯��
ConsistentHashRouter::ConsistentHashRouter()
{
	virtual_nodes.clear();
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
void ConsistentHashRouter::PopulateVirtualNodes(uint32_t node_id, std::set<VirtualNode>& nodes, int num, hash_func_t func)
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
		
		nodes.insert(new_node);
	}
}

// ��ʼ������
int ConsistentHashRouter::Init(std::set<uint32_t>& nodes, hash_func_t func, int num)
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
	std::set<uint32_t>::iterator it;
	for (it = nodes.begin(); it != nodes.end(); it++)
	{
		// Ϊÿ������ڵ㴴����ڵ�
		PopulateVirtualNodes((*it), virtual_nodes, n_vnodes_per_node, func);
	}

	hash_func = func;

	return 0;
}

// ���һ̨��ʵ�Ļ���
int ConsistentHashRouter::AddNode(uint32_t id)
{
	PopulateVirtualNodes(id, virtual_nodes, n_vnodes_per_node, hash_func);
	return 1;
}

//ɾ��һ̨��ʵ�Ļ���
int ConsistentHashRouter::DelNode(uint32_t id)
{
	std::set<VirtualNode>::iterator it;
	for (it = virtual_nodes.begin(); it != virtual_nodes.end();)
	{
		if (it->node_id == id)
		{
			it = virtual_nodes.erase(it);
		}
		else
			it++;
	}

	return 1;
}

// ����һ��key, �ҵ����Ӧ����ʵ����
int ConsistentHashRouter::Route(const std::string& key, uint32_t& out)
{
	if (virtual_nodes.empty())
		return -1;

	// 1. �����key��Ӧ��hashֵ
	uint32_t hash_val = hash_func(key);

	// 2. �ҵ�hashֵ��Ӧ������ڵ�
	VirtualNode vnTmp;
	vnTmp.hash_id = hash_val;
	std::set<VirtualNode>::iterator first = virtual_nodes.lower_bound(vnTmp); // set��lower_bound����

	// ����
	if (first == virtual_nodes.end())
		out = virtual_nodes.begin()->node_id;
	else
		out = first->node_id;

	return 1;
}