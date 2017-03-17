/*
* =====================================================================================
*
*       Filename:  main.cpp
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
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <ctime>
#include <iostream>
#include <vector>
#include <cassert>
#include <iostream>

#include <random>

#include "crc32.h"
#include "consistent.h"
#include <stdlib.h>

#define ITEMS 10000 // 数据量
#define REALS 10 // 实际服务器总数量

uint32_t hash2(const std::string& key)
{
	const char *str = key.c_str();
	return Crc32_Compute(str);
}

static char *random_key()
{
	static char buf[11];
	const char *random_str = "adf9pr134;fjasd f0pr1223rfa;dvasdhrf9erfhpaudsfja;er19043rfnqef;jqef014qpeqp3380p,4tnm;132rj138jfpa98dfn1;43yt9p4rfhl;adfy91n4p1k3j4tr";
	int len = strlen(random_str);
	int pos = rand() % (len - 11) + 1;
	assert(pos > 0);
	assert(pos < len - 10);
	int random_len = (rand() % 10) + 1;
	assert(random_len > 0);
	assert(random_len < 11);
	strncpy(buf, random_str + pos, random_len);
	buf[random_len] = '\0';
	return buf;
}


void testConsistentHashDelNode(ConsistentHashRouter router, std::vector<std::string> keys)
{
	ConsistentHashRouter router2 = router;
	router2.DelNode(2);

	uint32_t change = 0;

	for (int i = 0; i < ITEMS; i++)
	{
		char* key = (char*)keys[i].c_str();
		uint32_t n1;
		uint32_t n2;

		int ret = router.Route(key, n1);
		ret = router2.Route(key, n2);

		if (n1 != n2)
			change++;
	}

	printf("Change: %d\t(%0.2f%%)\n", change, change * 100.0 / ITEMS);
	
}

void testConsistentHashAddNode(ConsistentHashRouter router, std::vector<std::string> keys)
{
	ConsistentHashRouter router2 = router;
	router2.AddNode(11);

	uint32_t change = 0;

	for (int i = 1; i < ITEMS; i++)
	{
		char* key = (char*)keys[i].c_str();
		uint32_t n1;
		uint32_t n2;

		int ret = router.Route(key, n1);
		ret = router2.Route(key, n2);

		if (n1 != n2)
			change++;
	}

	printf("Change: %d\t(%0.2f%%)\n", change, change * 100.0 / ITEMS);

}

void testCommonDelNode(std::vector<std::string> keys)
{
	uint32_t change = 0;

	for (int i = 0; i < ITEMS; i++)
	{
		uint32_t hashKey = hash2(keys[i]);

		uint32_t n1 = hashKey % REALS;
		uint32_t n2 = hashKey % (REALS - 1);

		if (n1 != n2)
			change++;
	}

	printf("Change: %d\t(%0.2f%%)\n", change, change * 100.0 / ITEMS);

}

void testCommonAddNode(std::vector<std::string> keys)
{
	uint32_t change = 0;

	for (int i = 0; i < ITEMS; i++)
	{
		uint32_t hashKey = hash2(keys[i]);

		uint32_t n1 = hashKey % REALS;
		uint32_t n2 = hashKey % (REALS + 1);
	
		if (n1 != n2)
			change++;
	}

	printf("Change: %d\t(%0.2f%%)\n", change, change * 100.0 / ITEMS);

}

int main()
{
	ConsistentHashRouter router;
	// 10台真实机器
	std::set<uint32_t> nodes;
	for (uint32_t i = 1; i <= REALS; i +=1)
	{ 
		nodes.insert(i);
	}
	router.Init(nodes, hash2, 100);

	std::vector<std::string> keys;
	srand(time(NULL));
	// 假设有1,000，000实际ip;
	for (int i = 0; i < ITEMS; i++)
	{
		char *key = random_key();
		//printf("key = %-10s\n", key);
		std::string tmp(key);
		keys.push_back(tmp);
	}

	printf("\n");

#if 1
	printf("数据量：%u,  服务器是数量：%u\n\n", ITEMS, REALS);

	printf("普通取模哈希，删除1台服务器，数据迁移变化：\n");
	testCommonDelNode(keys);
	printf("\n");

	printf("普通取模哈希， 添加1台服务器，数据迁移变化：\n");
	testCommonAddNode(keys);
	printf("\n");

	printf("采用一致性hash, 删除1台服务器，数据迁移变化：\n");
	testConsistentHashDelNode(router, keys);
	printf("\n");

	printf("采用一致性hash, 添加1台服务器，数据迁移变化：\n");
	testConsistentHashAddNode(router, keys);
	printf("\n");
#endif

	return 0;
}
