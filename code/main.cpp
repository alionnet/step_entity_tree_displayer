#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <set>
#include <map>


bool isLastNonSpaceSemiColon(std::string sLine) {
	std::size_t iLen = sLine.size();
	for (long long int i = iLen-1; i >= 0; --i) 
	{
		if (sLine[i] == ' ' || sLine[i] == '\t' || sLine[i] == '\r' || sLine[i] == '\n') continue;

		if (sLine[i] == ';') return true;

		return false;
	}
}

struct Entity {
	std::string name;
	std::set<int> references;

};

bool addEntity(std::map<int,Entity>& entities, int num, std::string sName) 
{
	Entity e;
	e.name = sName;
	std::set<int> set;
	e.references = set;
	return entities.insert({num,e}).second;
}

bool addReferenceToEntity(std::map<int,Entity>& entities, int iEntityNum, int iReferenceTo) 
{
	auto& entity = entities.find(iEntityNum);

	//Returns false if entity that should gain a reference to the other entity does not exist -- should NOT happen in theory
	if (entity == entities.end()) {
		return false;
	}

	entity->second.references.insert(iReferenceTo);
	return true;
}

std::pair<int,std::string> entityNumberAndName(std::string sLine) 
{
	std::string num = "";
	std::string name = "";

	bool parsingNum = false;
	bool numFound = false;

	bool parsingName = false;
	for (char c : sLine) 
	{
		if (!numFound)
		{

			if ('0' <= c && c <= '9') 
			{
				if (!parsingNum) parsingNum = true;
				num += c;
				continue;
			}
			else if (parsingNum) 
			{
				parsingNum = false;
				numFound = true;
				continue;
			}

			//Either the # or a space ? in both cases skip to try to find a number
			else {
				continue;
			}
		}

		//First character of name must be a capital letters
		if (('A' <= c && c <= 'Z') && !parsingName) 
		{
			name += c;
			parsingName = true;
			continue;
		}
		//Otherwise skip to try to find first character of entity name
		else if (!parsingName) continue;

		//Allows capital letters, numbers and underscores as part of entity name
		if (('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_')
		{
			name += c;
		}
		else {
			parsingName = false;
			break;
		}
		
	}

	return {std::stoi(num),name};
}

//Extracts all entities that are referenced by the entity defined on line sLine
std::set<int> entityReferencesTo(std::string sLine)
{
	std::set<int> res;
	bool pastEqual = false;
	bool parsingNumber = false;
	std::string numberParsed;

	for (char c : sLine) {

		//Skips everything before the = to avoid the entity number
		if (c != '=' && !pastEqual) continue;

		if (c == '=' && !pastEqual) 
		{
			pastEqual = true;
			continue;
		}

		//Now the entity number is passed, we can start looking for references

		//Signals that we start parsing a reference
		if (c == '#')
		{
			parsingNumber = true;
			numberParsed = "";
			continue;
		}

		if (parsingNumber && '0' <= c && c <= '9') 
		{
			numberParsed += c;
		}
		else if (parsingNumber)
		{
			parsingNumber = false;
			if (numberParsed.size() > 0) 
			{
				res.insert(std::stoi(numberParsed));
			}
		}
	}

	return res;
}

//Returns true if an entity is referenced by another one
bool isEntityReferenced(const std::map<int, Entity>& entities, int iNum)
{
	for (auto entity : entities)
	{
		if (entity.first == iNum) continue;

		for (int iRef : entity.second.references)
		{
			if (iRef == iNum) return true;
		}
	}

	return false;
}

//Returns all entities which are not referenced by any other one
std::vector<int> unreferencedEntities(const std::map<int, Entity>& entities) {
	std::vector<int> res;

	for (auto entity : entities)
	{
		if (!isEntityReferenced(entities, entity.first)) res.push_back(entity.first);
	}

	return res;
}

//Utility for display readability
void printNTabs(int n) {
	for (int i = 0; i < n; ++i)
	{
		std::cout << "|\t";
	}
}

void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth = 0) {
	printNTabs(iDepth);
	auto entity = entities.find(iNum);

	if (entity != entities.end()) 
	{
		std::cout << "Entity #" << entity->first << " (" << entity->second.name << ")" << (entity->second.references.size() > 0 ? " refrences:" : "") << std::endl;

		for (int iRef : entity->second.references)
		{
			printEntity(entities, iRef, iDepth + 1);
		}
	}
	else {
#ifdef DEBUG
		std::cerr << "Error: Entity #" << iNum << " not found. Something probably went very wrong." << std::endl;
#endif // DEBUG

	}
}

void printEntityTree(const std::map<int, Entity>& entities) {
	std::vector<int> aiUnref = unreferencedEntities(entities);

	for (int iEntity : aiUnref)
	{
		printEntity(entities, iEntity);
		std::cout << "\\__" << std::endl;
	}
}

void usage() {
	std::cerr << "Usage: ./setv filename" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
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
			if (iLineNum == 1 && !std::regex_match(sSavedLine, std::regex("ISO-10303-21;[ \t\n]*"))) 
			{
				std::cerr << "First line does not match expected token ISO-10303-21;" << std::endl;
				ifsFile.close();
				return 3;
			}


			if (!data) 
			{
				//Look for start of DATA section
				data = std::regex_match(sSavedLine,std::regex("^DATA;[ \t\n]*"));
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
			std::set<int> references = entityReferencesTo(sSavedLine);

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
	else {
		std::cerr << "Couldn't open file " << sFile << std::endl;
		return 2;
	}

	printEntityTree(entityReferences);

	ifsFile.close();
	return 0;
}