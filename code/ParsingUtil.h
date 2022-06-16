#include <string>
#include <set>
#include <iostream>

#include "EntityUtil.h"

#ifndef PARSING_UTIL_H
#define PARSING_UTIL_H

bool isLastNonSpaceSemiColon(std::string sLine);
std::pair<int, std::string> entityNumberAndName(std::string sLine);
std::set<int> entityReferencesTo(std::string sLine);
void parseComplexEntity(EntityMap& entities, int iNum, const std::string& sLine, std::set<int>& allReferences);


#endif // !PARSING_UTIL_H
