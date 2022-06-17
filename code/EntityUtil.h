#include <string>
#include <set>
#include <vector>
#include <map>

#ifndef ENTITY_UTIL_H
#define ENTITY_UTIL_H

/*
* @brief Represents an entity and contains its name and references to other entities
*/
struct Entity {
public:
	bool isComplex() const;

private:

public:
	std::string name;
	std::set<int> references;

	//Only has content if complex entity
	std::vector<Entity> leaves;

private:

};

typedef std::map<int, Entity> EntityMap;

bool addEntity(EntityMap& entities, int num, std::string sName);
bool addReferenceToEntity(EntityMap& entities, int iEntityNum, int iReferenceTo);
bool isEntityReferenced(const EntityMap& entities, int iNum, const std::set<int>& allReferences);
std::set<int> unreferencedEntities(const EntityMap& entities, const std::set<int>& allReferences);

std::set<int> unrefEntitiesReferencing(const EntityMap& entities, const std::set<int>& viUnref, int iToRef);


#endif // !ENTITY_UTIL_H
