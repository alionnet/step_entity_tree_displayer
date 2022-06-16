#include <iostream>
#include <string>

#include <set>
#include <vector>

#ifndef OPTIONS_H
#define OPTIONS_H

namespace opt {

	struct Options {
	public:
		Options();
		void parseArgs(int argc, char* argv[]);

	public:
		std::string sFileName;

		bool bMaxDepth;
		int iMaxDepth;


		bool bNoDuplicates;
		std::set<int> treated;


		bool bFilterId;
		int iFilterId;
		std::set<int> containingId;


		bool bFilterType;
		std::string sFilterType;
		std::vector<int> ofFilteredType;


		bool bError;
	};

	

} //opt

#endif //OPTIONS_H