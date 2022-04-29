#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <vector>
#include <set>
#include <map>

#define NUMBERLESS -1


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
public:
	bool isComplex() const;

private:

public:
	std::string name;
	std::set<int> references;

	//Only has content if complex entity
	std::vector<Entity> leafs;

private:
	
};

bool Entity::isComplex() const {
	return leafs.size() > 0;
}

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

	bool bParsingNum = false;
	bool bNumFound = false;
	bool bParsingName = false;
	bool bComplexEntity = false;

	for (char c : sLine) 
	{
		if (!bNumFound)
		{

			if ('0' <= c && c <= '9') 
			{
				if (!bParsingNum) bParsingNum = true;
				num += c;
				continue;
			}
			else if (bParsingNum) 
			{
				bParsingNum = false;
				bNumFound = true;
				continue;
			}

			//Either the # or a space ? in both cases skip to try to find a number
			else {
				continue;
			}
		}

		if (bNumFound && !bParsingName && c == '(') {
			bComplexEntity = true;
			break;
		}

		//First character of name must be a capital letters
		if (('A' <= c && c <= 'Z') && !bParsingName) 
		{
			name += c;
			bParsingName = true;
			continue;
		}
		//Otherwise skip to try to find first character of entity name
		else if (!bParsingName) continue;

		//Allows capital letters, numbers and underscores as part of entity name
		if (('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_')
		{
			name += c;
		}
		else {
			bParsingName = false;
			break;
		}
		
	}

	if (bComplexEntity) return {std::stoi(num),"C P L X"}; //Should not in theory a name that could belong to a real STEP entity

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

void parseComplexEntity(std::map<int,Entity>& entities, int iNum, const std::string& sLine) {
	std::string::size_type szParPos = sLine.find_first_of('(');

	std::size_t luParDepth = 1;

	bool bParsingName = false;
	bool bParsingReference = false;
	bool bAlreadyPushed = false;

	std::string sName = "";
	std::string sRef = "";

	auto eComplexPair = entities.find(iNum);
	if (eComplexPair == entities.end())
	{
#ifdef DEBUG
		std::cerr << "Error: Entity #" << iNum << " not found. Something went probably very wrong." << std::endl;
#endif // DEBUG
		return;
	}

	Entity& eComplex = eComplexPair->second;
	Entity eSavedEntity;

	for (std::string::size_type szIdx = szParPos+1; szIdx < sLine.size(); ++szIdx)
	{
		char c = sLine[szIdx];

		if (c == '(') ++luParDepth;

		if (!bParsingName && 'A' <= c && c <= 'Z' && luParDepth == 1)
		{
			bParsingName = true;
			sName += c;
			continue;
		}

		if (bParsingName && (('A' <= c && c <= 'Z') || c == '_') && luParDepth == 1)
		{
			sName += c;
			continue;
		}
		else if (bParsingName && luParDepth == 1)
		{
			bParsingName = false;
			eSavedEntity.name = sName;
			sName = "";
			continue;
		}


		if (luParDepth >= 2 && c == '#') 
		{
			bParsingReference = true;
			continue;
		}

		if (bParsingReference && luParDepth >= 2 && '0' <= c && c <= '9')
		{
			sRef += c;
			continue;
		}
		else if (bParsingReference && luParDepth >= 2)
		{
			bParsingReference = false;
			eSavedEntity.references.insert(std::stoi(sRef));
			sRef = "";
		}

		if (c == ')')
		{
			if (luParDepth == 0) throw std::runtime_error("Ill-formed complex entity: Unmatched closing parenthese.");
			if (luParDepth >= 2 && !bAlreadyPushed)
			{
				eComplex.leafs.push_back(eSavedEntity);
				bAlreadyPushed = true;
			}
			--luParDepth;

			if (luParDepth == 1) {
				bAlreadyPushed = false;
			}
		}
	}

	if (luParDepth > 0) throw std::runtime_error("Ill-formed complex entity: Unmatched opening parenthese.");
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

void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth = 0);

void printNumberlessEntity(const std::map<int,Entity>& toRelay, const Entity& e, int iDepth) {
	printNTabs(iDepth);

	std::cout << "Numberless Entity (" << e.name << ")" << (e.references.size() > 0 ? " references:" : "") << std::endl;
	for (int iRef : e.references)
	{
		printEntity(toRelay, iRef, iDepth + 1);
	}
}

void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth) {
	printNTabs(iDepth);
	auto entity = entities.find(iNum);

	if (entity != entities.end()) 
	{
		if (entity->second.isComplex()) 
		{
			std::cout << "Complex Entity #" << entity->first << " contains: " << std::endl;
			for (const Entity& e : entity->second.leafs)
			{
				printNumberlessEntity(entities, e, iDepth + 1);
			}

		} else
		{
			std::cout << "Entity #" << entity->first << " (" << entity->second.name << ")" << (entity->second.references.size() > 0 ? " refrences:" : "") << std::endl;

			for (int iRef : entity->second.references)
			{
				printEntity(entities, iRef, iDepth + 1);
			}
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

			if (infos.second == "C P L X") 
			{
				if (!addEntity(entityReferences, infos.first, ""))
				{
					std::cerr << "Warning: Redefinition of entity #" << infos.first << " at Line " << iLineNum << ". Data ignored: " << sSavedLine << std::endl;
					continue;
				}

				parseComplexEntity(entityReferences, infos.first, sSavedLine);
			}
			else {
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
	}
	else {
		std::cerr << "Couldn't open file " << sFile << std::endl;
		return 2;
	}

	printEntityTree(entityReferences);

	ifsFile.close();
	return 0;
}