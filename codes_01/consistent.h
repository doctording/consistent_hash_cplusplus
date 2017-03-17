/*
* =====================================================================================
*
*       Filename:  consistent_hash.h
*
*    Description:
*
*        Version:  1.0
*        Created:  01/12/2016 10:50:26 AM
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  seth.hg@gmail.com
*
* =====================================================================================
*/
#ifndef __CONSISTENT_HASH_ROUTER_H
#define __CONSISTENT_HASH_ROUTER_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

// string to uint32_t ��һ��hashת������
typedef uint32_t(*hash_func_t)(const std::string&);

class VirtualNode
{
public:
	bool operator < (const VirtualNode& n) const
	{
		return hash_id < n.hash_id;
	}
public:
	uint32_t hash_id;   // ����node id
	uint32_t node_id;   // ��ʵ��node
};

class ConsistentHashRouter
{
public:
	ConsistentHashRouter();
	int Init(std::vector<uint32_t>&, hash_func_t, int num = 1);
	int AddNode(uint32_t, hash_func_t);
	int DelNode(uint32_t);
	int Route(const std::string&, uint32_t&);

private:
	void PopulateVirtualNodes(uint32_t, std::vector<VirtualNode>&, int, hash_func_t);

private:
	std::vector<VirtualNode> virtual_nodes;
	int n_vnodes_per_node;
	hash_func_t hash_func;
	bool has_init;

	//std::map<uint32_t, uint32_t> realCntMap; // ÿһ̨��ʵ������Ӧ������
};

#endif