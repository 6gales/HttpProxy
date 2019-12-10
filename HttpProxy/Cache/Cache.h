#pragma once
#include <unordered_map>
#include <vector>
#include <pthread.h>
#include "CacheEntry.h"

class Cache
{
	pthread_mutex_t cacheLock;
	std::unordered_map<std::string, CacheEntry> cache;

public:
	Cache()
	{
		pthread_mutex_init(&cacheLock, NULL);
	}

	bool createIfNotExists(std::string key)
	{
		pthread_mutex_lock(&cacheLock);
		
		auto it = cache.insert(std::make_pair(key, CacheEntry()));
		
		pthread_mutex_unlock(&cacheLock);

		return it.second;
	}

	CacheEntry& getEntry(std::string key)
	{
		pthread_mutex_lock(&cacheLock);

		auto it = cache.find(key);
		if (it != cache.end())
		{
			pthread_mutex_unlock(&cacheLock);
			throw std::invalid_argument(key + " not found in cache");
		}

		pthread_mutex_unlock(&cacheLock);

		return it->second;
	}

	~Cache()
	{
		pthread_mutex_destroy(&cacheLock);
	}
};