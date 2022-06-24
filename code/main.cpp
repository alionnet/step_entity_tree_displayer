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

#include "AP242/Product.h"

bool stringContains(const std::string& haystack, const std::string& needle) 
{
	bool res = true;
	for (size_t iChar = 0; iChar < haystack.size(); ++iChar) 
	{
		for (size_t iCharNeedle = 0; iCharNeedle < needle.size(); ++iCharNeedle)
		{
			if (needle[iCharNeedle] != haystack[iChar + iCharNeedle])
			{
				res = false;
				break;
			}

			res = true;
		}

		if (res) return true;

	}

	return false;
}


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
	if (opts.bFilterOutTypes && opts.ssFilteredTypes.find(e.name) != opts.ssFilteredTypes.end()) {
		return;
	}

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

	if (opts.bFilterOutTypes && opts.siFilteredOut.find(iNum) != opts.siFilteredOut.end()) {
		return;
	}

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
		return;
	}

	if (iDepth == 0) {
		std::cout << "\\__" << std::endl;
	}
}

void printEntityTree(const std::map<int, Entity>& entities, const std::set<int> &allReferences, opt::Options &opts) {
	std::set<int> viToUse;
	std::set<int> viUnref = unreferencedEntities(entities, allReferences);

	std::set<int>* toUse = &viToUse;

	if (opts.bFilterId) {
		viToUse = unrefEntitiesReferencing(entities, viUnref, opts.iFilterId);
		if (viToUse.empty())
		{
			std::cout << "Specified filter ID was not found in the file" << std::endl;
			std::cerr << "Specified filter ID was not found in the file" << std::endl;
		}
	}
	else if (opts.bFilterType) {
		viToUse = opts.siFilteredIn;
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
		//std::cout << "\\__" << std::endl;
	}
}


void parseProducts(std::string& sFilePath, ProductMap& products) {
	std::regex productRegex = std::regex("#[1-9][0-9]*[ \t]*=[ \t]*PRODUCT[(].*;[ \t\n]*");
	std::string sLine;

	std::ifstream ifsFile;
	ifsFile.open(sFilePath);

	while (ifsFile)
	{
		std::getline(ifsFile, sLine);
		std::string sSavedLine = "";
		sSavedLine += sLine;

		//Next line is also part of current line
		while (sLine.size() == 0 || !isLastNonSpaceSemiColon(sLine)) {
			std::getline(ifsFile, sLine);
			sSavedLine += sLine;

			if (!ifsFile) break;
		}

		if (!std::regex_match(sSavedLine, productRegex)) continue;

		std::size_t id_start = sSavedLine.find_first_of('\''), id_end = sSavedLine.substr(id_start + 1).find_first_of('\'');
		std::string identifier = sSavedLine.substr(id_start + 1, id_end);

		addProduct(products, identifier);
	}

	ifsFile.close();
}

void printAllTypes(const opt::Options &opts) 
{
	std::cout << "Present types are:" << std::endl;
	for (const std::string& type : opts.ssTypes)
	{
		std::cout << type << std::endl;
	}
}

void printProducts(const ProductMap& products, const EntityMap& entities, const std::set<int>& allReferences, opt::Options& opts)
{
	std::set<int> viUnref = unreferencedEntities(entities, allReferences);
	std::set<int> printed;

	if (opts.bAP242ProductNameFilter)
	{
		std::cout << "Some products were removed due to product name filter" << std::endl;
	}

	for (const auto& prod : products) {
		if (opts.bAP242ProductNameFilter && opts.sProductName != prod.first) continue;

		std::cout << "Product `" << prod.first << "`:" << std::endl;
		for (int i : prod.second.siEntitiesUsing) 
		{
			std::set<int> toTreat = unrefEntitiesReferencing(entities, viUnref, i);

			for (int iToTreat : toTreat)
			{
				if (!printed.count(iToTreat))
				{
					printEntity(entities, iToTreat, 0, opts);
					printed.insert(iToTreat);
				}
			}
		}
		std::cout << "End of product `"<< prod.first <<"`" << std::endl << std::endl << std::endl;
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

	EntityMap entityReferences;
	std::set<int> allReferences;
	ProductMap products;

	if (ifsFile.is_open()) 
	{
		if (opts.bAP242Products) {
			parseProducts(sFile, products);
		}

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
			if (opts.bOnlyTypes) 
			{
				if (opts.ssTypes.find(infos.second) == opts.ssTypes.end())
				{
					opts.ssTypes.insert(infos.second);
				}
				continue;
			}


			if (opts.bFilterType && infos.second == opts.sFilterType)
			{
				opts.siFilteredIn.insert(infos.first);
			}

			if (opts.bFilterOutTypes && opts.ssFilteredTypes.find(infos.second) != opts.ssFilteredTypes.end())
			{
				opts.siFilteredOut.insert(infos.first);
			}

			if (opts.bAP242Products) {
				for (auto& prod : products) {
					if (std::regex_match(sSavedLine,std::regex(".*=.*[' (]+"+prod.first+"[' )]+.*")))
					{
						prod.second.siEntitiesUsing.insert(infos.first);
					}
				}
			}



			if (infos.second == COMPLEX_TYPE) 
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
	else 
	{
		std::cerr << "Couldn't open file " << sFile << std::endl;
		return 2;
	}

	if (opts.bFilterOutTypes) 
	{
		std::cout << "Some entities were removed due to type filter" << std::endl;
	}

	if (opts.bOnlyTypes)
	{
		printAllTypes(opts);
	}
	else if (opts.bAP242Products)
	{
		printProducts(products, entityReferences, allReferences, opts);
	}
	else
	{
		printEntityTree(entityReferences, allReferences, opts);
	}

	ifsFile.close();
	return 0;
}