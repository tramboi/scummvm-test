/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/resmanager.h"

#include "sword25/kernel/resource.h"
#include "sword25/kernel/resservice.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define BS_LOG_PREFIX "RESOURCEMANAGER"

ResourceManager::~ResourceManager() {
	// Clear all unlocked resources
	emptyCache();

	// All remaining resources are not released, so print warnings and release
	Common::List<Resource *>::iterator iter = _resources.begin();
	for (; iter != _resources.end(); ++iter) {
		BS_LOG_WARNINGLN("Resource \"%s\" was not released.", (*iter)->getFileName().c_str());

		// Set the lock count to zero
		while ((*iter)->getLockCount() > 0) {
			(*iter)->release();
		};

		// Delete the resource
		delete(*iter);
	}
}

/**
 * Returns a resource by it's ordinal index. Returns NULL if any error occurs
 * Note: This method is not optimised for speed and should be used only for debugging purposes
 * @param Ord       Ordinal number of the resource. Must be between 0 and GetResourceCount() - 1.
 */
Resource *ResourceManager::getResourceByOrdinal(int ord) const {
	// �berpr�fen ob der Index Ord innerhald der Listengrenzen liegt.
	if (ord < 0 || ord >= getResourceCount()) {
		BS_LOG_ERRORLN("Resource ordinal (%d) out of bounds (0 - %d).", ord, getResourceCount() - 1);
		return NULL;
	}

	// Liste durchlaufen und die Resource mit dem gew�nschten Index zur�ckgeben.
	int curOrd = 0;
	Common::List<Resource *>::const_iterator iter = _resources.begin();
	for (; iter != _resources.end(); ++iter, ++curOrd) {
		if (curOrd == ord)
			return (*iter);
	}

	// Die Ausf�hrung sollte nie an diesem Punkt ankommen.
	BS_LOG_EXTERRORLN("Execution reached unexpected point.");
	return NULL;
}

/**
 * Registers a RegisterResourceService. This method is the constructor of
 * BS_ResourceService, and thus helps all resource services in the ResourceManager list
 * @param pService      Which service
 */
bool ResourceManager::registerResourceService(ResourceService *pService) {
	if (!pService) {
		BS_LOG_ERRORLN("Can't register NULL resource service.");
		return false;
	}

	_resourceServices.push_back(pService);

	return true;
}

/**
 * Deletes resources as necessary until the specified memory limit is not being exceeded.
 */
void ResourceManager::deleteResourcesIfNecessary() {
	// If enough memory is available, or no resources are loaded, then the function can immediately end
	if (_kernelPtr->getUsedMemory() < _maxMemoryUsage || _resources.empty())
		return;

	// Keep deleting resources until the memory usage of the process falls below the set maximum limit.
	// The list is processed backwards in order to first release those resources who have been
	// not been accessed for the longest
	Common::List<Resource *>::iterator iter = _resources.end();
	do {
		--iter;

		// The resource may be released only if it isn't locked
		if ((*iter)->getLockCount() == 0)
			iter = deleteResource(*iter);
	} while (iter != _resources.begin() && _kernelPtr->getUsedMemory() > _maxMemoryUsage);
}

/**
 * Releases all resources that are not locked.
 **/
void ResourceManager::emptyCache() {
	// Scan through the resource list
	Common::List<Resource *>::iterator iter = _resources.begin();
	while (iter != _resources.end()) {
		if ((*iter)->getLockCount() == 0) {
			// Delete the resource
			iter = deleteResource(*iter);
		} else
			++iter;
	}
}

/**
 * Returns a requested resource. If any error occurs, returns NULL
 * @param FileName      Filename of resource
 */
Resource *ResourceManager::requestResource(const Common::String &fileName) {
	// Get the absolute path to the file
	Common::String uniqueFileName = getUniqueFileName(fileName);
	if (uniqueFileName == "")
		return NULL;

	// Determine whether the resource is already loaded
	// If the resource is found, it will be placed at the head of the resource list and returned
	{
		Resource *pResource = getResource(uniqueFileName);
		if (pResource) {
			moveToFront(pResource);
			(pResource)->addReference();
			return pResource;
		}
	}

	// The resource was not found, therefore, must not be loaded yet
	if (_logCacheMiss)
		BS_LOG_WARNINGLN("\"%s\" was not precached.", uniqueFileName.c_str());

	Resource *pResource;
	if ((pResource = loadResource(uniqueFileName))) {
		pResource->addReference();
		return pResource;
	}

	return NULL;
}

/**
 * Loads a resource into the cache
 * @param FileName      The filename of the resource to be cached
 * @param ForceReload   Indicates whether the file should be reloaded if it's already in the cache.
 * This is useful for files that may have changed in the interim
 */
bool ResourceManager::precacheResource(const Common::String &fileName, bool forceReload) {
	// Get the absolute path to the file
	Common::String uniqueFileName = getUniqueFileName(fileName);
	if (uniqueFileName == "")
		return false;

	Resource *resourcePtr = getResource(uniqueFileName);

	if (forceReload && resourcePtr) {
		if (resourcePtr->getLockCount()) {
			BS_LOG_ERRORLN("Could not force precaching of \"%s\". The resource is locked.", fileName.c_str());
			return false;
		} else {
			deleteResource(resourcePtr);
			resourcePtr = 0;
		}
	}

	if (!resourcePtr && loadResource(uniqueFileName) == NULL) {
		BS_LOG_ERRORLN("Could not precache \"%s\",", fileName.c_str());
		return false;
	}

	return true;
}

/**
 * Moves a resource to the top of the resource list
 * @param pResource     The resource
 */
void ResourceManager::moveToFront(Resource *pResource) {
	// Erase the resource from it's current position
	_resources.erase(pResource->_iterator);
	// Re-add the resource at the front of the list
	_resources.push_front(pResource);
	// Reset the resource iterator to the repositioned item
	pResource->_iterator = _resources.begin();
}

/**
 * Loads a resource and updates the m_UsedMemory total
 *
 * The resource must not already be loaded
 * @param FileName      The unique filename of the resource to be loaded
 */
Resource *ResourceManager::loadResource(const Common::String &fileName) {
	// ResourceService finden, der die Resource laden kann.
	for (uint i = 0; i < _resourceServices.size(); ++i) {
		if (_resourceServices[i]->canLoadResource(fileName)) {
			// If more memory is desired, memory must be released
			deleteResourcesIfNecessary();

			// Load the resource
			Resource *pResource;
			if (!(pResource = _resourceServices[i]->loadResource(fileName))) {
				BS_LOG_ERRORLN("Responsible service could not load resource \"%s\".", fileName.c_str());
				return NULL;
			}

			// Add the resource to the front of the list
			_resources.push_front(pResource);
			pResource->_iterator = _resources.begin();

			// Also store the resource in the hash table for quick lookup
			_resourceHashTable[pResource->getFileNameHash() % HASH_TABLE_BUCKETS].push_front(pResource);

			return pResource;
		}
	}

	BS_LOG_ERRORLN("Could not find a service that can load \"%s\".", fileName.c_str());
	return NULL;
}

/**
 * Returns the full path of a given resource filename.
 * It will return an empty string if a path could not be created.
*/
Common::String ResourceManager::getUniqueFileName(const Common::String &fileName) const {
	// Get a pointer to the package manager
	PackageManager *pPackage = (PackageManager *)_kernelPtr->getPackage();
	if (!pPackage) {
		BS_LOG_ERRORLN("Could not get package manager.");
		return Common::String("");
	}

	// Absoluten Pfad der Datei bekommen und somit die Eindeutigkeit des Dateinamens sicherstellen
	Common::String uniquefileName = pPackage->getAbsolutePath(fileName);
	if (uniquefileName == "")
		BS_LOG_ERRORLN("Could not create absolute file name for \"%s\".", fileName.c_str());

	return uniquefileName;
}

/**
 * Deletes a resource, removes it from the lists, and updates m_UsedMemory
 */
Common::List<Resource *>::iterator ResourceManager::deleteResource(Resource *pResource) {
	// Remove the resource from the hash table
	_resourceHashTable[pResource->getFileNameHash() % HASH_TABLE_BUCKETS].remove(pResource);

	Resource *pDummy = pResource;

	// Delete the resource from the resource list
	Common::List<Resource *>::iterator result = _resources.erase(pResource->_iterator);

	// Delete the resource
	delete(pDummy);

	// Return the iterator
	return result;
}

/**
 * Returns a pointer to a loaded resource. If any error occurs, NULL will be returned.
 * @param UniquefileName        The absolute path and filename
 * Gibt einen Pointer auf die angeforderte Resource zur�ck, oder NULL, wenn die Resourcen nicht geladen ist.
 */
Resource *ResourceManager::getResource(const Common::String &uniquefileName) const {
	// Determine whether the resource is already loaded
	const Common::List<Resource *>& hashBucket = _resourceHashTable[Common::hashit(uniquefileName) % HASH_TABLE_BUCKETS];
	{
		Common::List<Resource *>::const_iterator iter = hashBucket.begin();
		for (; iter != hashBucket.end(); ++iter) {
			// Wenn die Resource gefunden wurde wird sie zur�ckgegeben.
			if ((*iter)->getFileName() == uniquefileName)
				return *iter;
		}
	}

	// Resource wurde nicht gefunden, ist also nicht geladen
	return NULL;
}

/**
 * Writes the names of all currently locked resources to the log file
 */
void ResourceManager::dumpLockedResources() {
	for (Common::List<Resource *>::iterator iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter)->getLockCount() > 0) {
			BS_LOGLN("%s", (*iter)->getFileName().c_str());
		}
	}
}

/**
 * Specifies the maximum amount of memory the engine is allowed to use.
 * If this value is exceeded, resources will be unloaded to make room. This value is meant
 * as a guideline, and not as a fixed boundary. It is not guaranteed not to be exceeded;
 * the whole game engine may still use more memory than any amount specified.
 */
void ResourceManager::setMaxMemoryUsage(uint maxMemoryUsage) {
	_maxMemoryUsage = maxMemoryUsage;
	deleteResourcesIfNecessary();
}

} // End of namespace Sword25
