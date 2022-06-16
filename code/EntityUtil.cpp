#include "EntityUtil.h"

bool Entity::isComplex() const {
	return leaves.size() > 0;
}

/*
* @brief Adds an entity to the map of known entities (without any references)
*/
bool addEntity(EntityMap& entities, int num, std::string sName)
{
	Entity e;
	e.name = sName;
	std::set<int> set;
	e.references = set;
	return entities.insert({ num,e }).second;
}

/*
* @brief Adds a reference from a given existing entity to another entity (existing or not)
*/
bool addReferenceToEntity(EntityMap& entities, int iEntityNum, int iReferenceTo)
{
	auto& entity = entities.find(iEntityNum);

	//Returns false if entity that should gain a reference to the other entity does not exist -- should NOT happen in theory
	if (entity == entities.end()) {
		return false;
	}

	entity->second.references.insert(iReferenceTo);
	return true;
}

/*
* @brief Returns true if an entity is referenced by another one
*/
bool isEntityReferenced(const EntityMap& entities, int iNum, const std::set<int>& allReferences)
{
	return allReferences.find(iNum) != allReferences.end();
}

/*
* @brief Returns all entities which are not referenced by any other one
*/
std::vector<int> unreferencedEntities(const EntityMap& entities, const std::set<int>& allReferences)
{
	std::vector<int> res;

	for (auto entity : entities)
	{
		if (!isEntityReferenced(entities, entity.first, allReferences)) res.push_back(entity.first);
	}

	return res;
}

bool isEntityReferencing(const EntityMap& entities, int iEntity, int iToRef) 
{
	bool res = false;
	const auto& entity = entities.find(iEntity);
	if (entity == entities.end()) return false;

	if (entity->second.isComplex())
	{
		for (const Entity &eLeaf : entity->second.leaves) 
		{
			for (int iRef : eLeaf.references) 
			{
				if (iRef == iToRef) return true;
				res = isEntityReferencing(entities, iRef, iToRef);
				if (res) return true;
			}
		}
	}
	else 
	{
		for (int iRef : entity->second.references) 
		{
			if (iRef == iToRef) return true;
			res = isEntityReferencing(entities, iRef, iToRef);
			if (res) return true;
		}
	}

	return false;
}

std::vector<int> unrefEntitiesReferencing(const EntityMap& entities, const std::vector<int>& viUnref, int iToRef) 
{
	std::vector<int> res;

	for (int iEntity : viUnref) 
	{
		if (iEntity == iToRef) res.push_back(iEntity);
		if (isEntityReferencing(entities, iEntity, iToRef))
		{
			res.push_back(iEntity);
		}
	}

	return res;
}