#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <vector>
#include <set>
#include <map>

/*
* @brief Checks if last character on a line which is not a spacing character is a semi-colon
*/
bool isLastNonSpaceSemiColon(std::string sLine) {
	std::size_t iLen = sLine.size();
	for (long long int i = iLen-1; i >= 0; --i) 
	{
		if (sLine[i] == ' ' || sLine[i] == '\t' || sLine[i] == '\r' || sLine[i] == '\n') continue;

		if (sLine[i] == ';') return true;

		return false;
	}

	return false;
}

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

bool Entity::isComplex() const {
	return leaves.size() > 0;
}

/*
* @brief Adds an entity to the map of known entities (without any references)
*/
bool addEntity(std::map<int,Entity>& entities, int num, std::string sName) 
{
	Entity e;
	e.name = sName;
	std::set<int> set;
	e.references = set;
	return entities.insert({num,e}).second;
}

/*
* @brief Adds a reference from a given existing entity to another entity (existing or not)
*/
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

		//If a parenthesis if found before the name starts, it's a complex entity, and will receive a different parsing
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

	if (bComplexEntity) return {std::stoi(num),"C P L X"}; //Arbitrary name to indicate a complex entity later on -- could not in theory belong to a real STEP entity

	return {std::stoi(num),name};
}

/*
* @brief Extracts all entities that are referenced by the entity defined on line sLine
*/
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

/*
* @brief Parses a complex entity and adds it all internal entities and their references
*/
void parseComplexEntity(std::map<int,Entity>& entities, int iNum, const std::string& sLine, std::set<int> &allReferences) {
	std::string::size_type szParPos = sLine.find_first_of('(');

	//Parenthesis depth: allows to know when to look for entity names or references, and as a bonus, to throw exception if an unexpected parenthesis is encountered
	std::size_t luParDepth = 1;

	bool bParsingName = false;
	bool bParsingReference = false;

	//True if current entity has already been pushed (avoids duplication on deep parenthesis levels)
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

	//Start after first '(', as complex entity number has already been parsed
	for (std::string::size_type szIdx = szParPos+1; szIdx < sLine.size(); ++szIdx)
	{
		char c = sLine[szIdx];

		//One parenthesis level deeper
		if (c == '(') ++luParDepth;

		//First character of a name must be a capital letter + we only look for entities in the initial complex entity (depth 1)
		if (!bParsingName && 'A' <= c && c <= 'Z' && luParDepth == 1)
		{
			bParsingName = true;
			sName += c;
			eSavedEntity.name = "";
			eSavedEntity.references.empty();
			continue;
		}

		if (bParsingName && (('A' <= c && c <= 'Z') || c == '_') && luParDepth == 1)
		{
			sName += c;
			continue;
		}
		//Not a character allowed in a name --> stop parsing name and save it
		//Either we are at depth 1 (general case) or at depth 2 if current char is a '('
		else if (bParsingName && (luParDepth == 1 || (luParDepth == 2 && c == '(')))
		{
			bParsingName = false;
			eSavedEntity.name += sName;
			sName = "";
			continue;
		}

		//Encounters a reference at any depth
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
		//End of reference number
		else if (bParsingReference && luParDepth >= 2)
		{
			bParsingReference = false;
			int iRef = std::stoi(sRef);
			eSavedEntity.references.insert(iRef);
			allReferences.insert(iRef);
			sRef = "";
		}

		if (c == ')')
		{
			//One extra closing parenthsis
			if (luParDepth == 0) throw std::runtime_error("Ill-formed complex entity: Unmatched closing parenthesis.");
			//Push entity and its references to complex entity leaves
			if (luParDepth >= 2 && !bAlreadyPushed)
			{
				eComplex.leaves.push_back(eSavedEntity);
				bAlreadyPushed = true;
			}
			--luParDepth;

			//Out of current entity
			if (luParDepth == 1) {
				bAlreadyPushed = false;
			}
		}
	}

	//Missing closing parenthesis
	if (luParDepth > 0) throw std::runtime_error("Ill-formed complex entity: Unmatched opening parenthesis.");
}

/*
* @brief Returns true if an entity is referenced by another one
*/
bool isEntityReferenced(const std::map<int, Entity>& entities, int iNum,const std::set<int> &allReferences)
{
	return allReferences.find(iNum) != allReferences.end();
}

/*
* @brief Returns all entities which are not referenced by any other one
*/
std::vector<int> unreferencedEntities(const std::map<int, Entity>& entities, const std::set<int> &allReferences) {
	std::vector<int> res;

	for (auto entity : entities)
	{
		if (!isEntityReferenced(entities, entity.first,allReferences)) res.push_back(entity.first);
	}

	return res;
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

void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth = 0);


void printNumberlessEntity(const std::map<int,Entity>& toRelay, const Entity& e, int iDepth) {
	printNTabs(iDepth);

	std::cout << "Numberless Entity (" << e.name << ")" << (e.references.size() > 0 ? " references:" : "") << std::endl;
	for (int iRef : e.references)
	{
		printEntity(toRelay, iRef, iDepth + 1);
	}
}

/*
* @brief Prints an entity (simple or complex)
*/
void printEntity(const std::map<int, Entity>& entities, int iNum, int iDepth) {
	printNTabs(iDepth);
	auto entity = entities.find(iNum);

	if (entity != entities.end()) 
	{
		if (entity->second.isComplex()) 
		{
			std::cout << "Complex Entity #" << entity->first << " contains: " << std::endl;
			for (const Entity& e : entity->second.leaves)
			{
				printNumberlessEntity(entities, e, iDepth + 1);
			}

		} else
		{
			std::cout << "Entity #" << entity->first << " (" << entity->second.name << ")" << (entity->second.references.size() > 0 ? " references:" : "") << std::endl;

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

void printEntityTree(const std::map<int, Entity>& entities, const std::set<int> &allReferences) {
	std::vector<int> aiUnref = unreferencedEntities(entities, allReferences);

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

	printEntityTree(entityReferences, allReferences);

	ifsFile.close();
	return 0;
}