#include "ParsingUtil.h"

/*
* @brief Checks if last character on a line which is not a spacing character is a semi-colon
*/
bool isLastNonSpaceSemiColon(std::string sLine) {
	std::size_t iLen = sLine.size();
	for (long long int i = iLen - 1; i >= 0; --i)
	{
		if (sLine[i] == ' ' || sLine[i] == '\t' || sLine[i] == '\r' || sLine[i] == '\n') continue;

		if (sLine[i] == ';') return true;

		return false;
	}

	return false;
}

std::pair<int, std::string> entityNumberAndName(std::string sLine)
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

	if (bComplexEntity) return { std::stoi(num),COMPLEX_TYPE }; //Arbitrary name to indicate a complex entity later on -- could not in theory belong to a real STEP entity

	return { std::stoi(num),name };
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
void parseComplexEntity(EntityMap& entities, int iNum, const std::string& sLine, std::set<int>& allReferences)
{
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
	for (std::string::size_type szIdx = szParPos + 1; szIdx < sLine.size(); ++szIdx)
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