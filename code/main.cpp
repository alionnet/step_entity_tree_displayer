#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <vector>
#include <set>
#include <map>


#include "Options.h"
#include "EntityUtil.h"
#include "ParsingUtil.h"

/*
* @brief Prints n '\t' for display readability
*/
void printNTabs(int n) {
	for (int i = 0; i < n; ++i)
	{
		std::cout << "|\t";
	}
}

//Forward declaration

void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth, opt::Options& opts);


void printNumberlessEntity(const std::map<int,Entity>& toRelay, const Entity& e, int iDepth, opt::Options& opts) {
	if (!opts.bMaxDepth || (opts.bMaxDepth && iDepth < opts.iMaxDepth))
	{
		printNTabs(iDepth);
		std::cout << "Numberless Entity (" << e.name << ")";
		if (e.references.size() > 0) 
		{
			std::cout << " references " << e.references.size() << " entities:";
		}
		std::cout << std::endl;
		
		for (int iRef : e.references)
		{
			printEntity(toRelay, iRef, iDepth + 1, opts);
		}
	}
}

/*
* @brief Prints an entity (simple or complex)
*/
void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth, opt::Options& opts) {
	auto entity = entities.find(iNum);

	if (entity != entities.end()) 
	{
		if (!opts.bMaxDepth || (opts.bMaxDepth && iDepth < opts.iMaxDepth))
		{
			printNTabs(iDepth);
			if (entity->second.isComplex())
			{
				std::cout << "Complex Entity #" << entity->first << " contains " << entity->second.leaves.size() << " entities: " << std::endl;

				for (const Entity& e : entity->second.leaves)
				{
					printNumberlessEntity(entities, e, iDepth + 1, opts);
				}
			}
			else
			{
				std::cout << "Entity #" << entity->first << " (" << entity->second.name << ")";
				if (entity->second.references.size() > 0)
				{
					std::cout << " references " << entity->second.references.size() << " entities:";
				}
				std::cout << std::endl;

				if (!opts.bNoDuplicates || (opts.bNoDuplicates && opts.treated.find(iNum) == opts.treated.end())) 
				{
					for (int iRef : entity->second.references)
					{
						printEntity(entities, iRef, iDepth + 1, opts);
					}
				}
				else 
				{
					printNTabs(iDepth + 1);
					std::cout << "Already developped" << std::endl;
					return;
				}
			}
		}

		if (opts.bNoDuplicates) 
		{
			opts.treated.insert(iNum);
		}
	}
	else {
#ifdef DEBUG
		std::cerr << "Error: Entity #" << iNum << " not found. Something probably went very wrong." << std::endl;
#endif // DEBUG

	}
}

void printEntityTree(const std::map<int, Entity>& entities, const std::set<int> &allReferences, opt::Options &opts) {
	std::vector<int> viToUse;
	std::vector<int> viUnref = unreferencedEntities(entities, allReferences);

	std::vector<int>* toUse = &viToUse;

	if (opts.bFilterId) {
		viToUse = unrefEntitiesReferencing(entities, viUnref, opts.iFilterId);
		if (viToUse.empty())
		{
			std::cout << "Specified filter ID was not found in the file" << std::endl;
			std::cerr << "Specified filter ID was not found in the file" << std::endl;
		}
	}
	else if (opts.bFilterType) {
		viToUse = opts.ofFilteredType;
		if (viToUse.empty())
		{
			std::cout << "Specified filter type was not found in the file" << std::endl;
			std::cerr << "Specified filter type was not found in the file" << std::endl;
		}
	}
	else {
		viToUse = viUnref;
	}

	for (int iEntity : *toUse)
	{
		printEntity(entities, iEntity, 0, opts);
		std::cout << "\\__" << std::endl;
	}
}



void usage() {
	std::cerr << "Usage: ./setv filename [options]" << std::endl;
}

int main(int argc, char* argv[]) {
	opt::Options opts;
	opts.parseArgs(argc, argv);

	if (opts.bError) {
		usage();
		return 1;
	}

	std::string sFile = std::string(argv[1]);

	std::ifstream ifsFile;
	ifsFile.open(sFile);

	std::string sLine;
	bool data = false;

	int iLineNum = 0;


	std::regex entityRegex = std::regex("#[1-9][0-9]*[ \t]*=[ \t]*.*;[ \t\n]*");

	std::map<int, Entity> entityReferences;
	std::set<int> allReferences;

	if (ifsFile.is_open()) 
	{
		while (ifsFile) 
		{
			++iLineNum;
			std::getline(ifsFile, sLine);
			std::string sSavedLine;
			sSavedLine += sLine;

			//Next line is also part of current line
			while (sLine.size() == 0 || !isLastNonSpaceSemiColon(sLine)) {
				std::getline(ifsFile, sLine);
				sSavedLine += sLine;
				++iLineNum;
			}

			//STEP-files should follow the standard defined by ISO-10303-21, and have this token on their first line
			if (iLineNum == 1 && !std::regex_match(sSavedLine, std::regex("ISO-10303-21;[ \t\r\n]*"))) 
			{
				std::cerr << "First line does not match expected token ISO-10303-21;" << std::endl;
				ifsFile.close();
				return 3;
			}


			if (!data) 
			{
				//Look for start of DATA section
				data = std::regex_match(sSavedLine,std::regex("^DATA;[ \t\r\n]*"));
				continue;
			}

			//Look for end of DATA section
			if (std::regex_match(sSavedLine, std::regex("ENDSEC;[ \t]*"))) break;

			//Skip comments
			if (sSavedLine[0] == '/' && sSavedLine[1] == '*') continue;

			if (!std::regex_match(sSavedLine, entityRegex)) 
			{
				std::cout << "Skipping line " << iLineNum << " not recognized as entity : \"" << sSavedLine << "\"" << std::endl;
				continue;
			}


			std::pair<int, std::string> infos = entityNumberAndName(sSavedLine);

			if (opts.bFilterType && infos.second == opts.sFilterType)
			{
				opts.ofFilteredType.push_back(infos.first);
			}

			if (infos.second == "C P L X") 
			{
				if (!addEntity(entityReferences, infos.first, ""))
				{
					std::cerr << "Warning: Redefinition of entity #" << infos.first << " at Line " << iLineNum << ". Data ignored: " << sSavedLine << std::endl;
					continue;
				}

				parseComplexEntity(entityReferences, infos.first, sSavedLine, allReferences);
			}
			else {
				std::set<int> references = entityReferencesTo(sSavedLine);

				for (int iRef : references) 
				{
					allReferences.insert(iRef);
				}

				if (!addEntity(entityReferences, infos.first, infos.second))
				{
					std::cerr << "Warning: Redefinition of entity #" << infos.first << " at Line " << iLineNum << ". Data ignored: " << sSavedLine << std::endl;
					continue;
				}

				for (int iRef : references)
				{
					bool refAdded = addReferenceToEntity(entityReferences, infos.first, iRef);
#ifdef DEBUG
					if (!refAdded) std::cerr << "Attemtpting to add reference from non-existing entity #" << infos.first << ". Something went very wrong" << std::endl;
#endif // DEBUG

				}
			}

			

		}
	}
	else {
		std::cerr << "Couldn't open file " << sFile << std::endl;
		return 2;
	}

	printEntityTree(entityReferences, allReferences, opts);

	ifsFile.close();
	return 0;
}