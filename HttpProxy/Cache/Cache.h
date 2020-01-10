#pragma once
#include <map>
#include <string>
#include <pthread.h>
#include <stdexcept>
#include "CacheEntry.h"

class Cache
{
	pthread_mutex_t cacheLock;
	std::map<std::string, CacheEntry*> cache;

public:
	Cache()
	{
		pthread_mutex_init(&cacheLock, NULL);
	}

	bool createIfNotExists(std::string key)
	{
		pthread_mutex_lock(&cacheLock);
		
		std::map<std::string, CacheEntry*>::iterator it = cache.find(key);
		bool found = it != cache.end();

		if (found && it->second->isInvalid())
		{
			delete it->second;
			found = false;
		}
		
		if (!found)
		{
			cache.insert(std::make_pair(key, new CacheEntry()));
		}

		pthread_mutex_unlock(&cacheLock);

		return !found;
	}

	CacheEntry *getEntry(std::string key)
	{
		pthread_mutex_lock(&cacheLock);

		std::map<std::string, CacheEntry*>::iterator it = cache.find(key);
		if (it == cache.end())
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
		for (std::map<std::string, CacheEntry*>::iterator it = cache.begin(); it != cache.end(); ++it)
		{
			delete it->second;
		}
	}
};