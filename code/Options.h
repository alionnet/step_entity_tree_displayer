#include <iostream>
#include <string>

#include <set>
#include <vector>

#ifndef OPTIONS_H
#define OPTIONS_H

#define COMPLEX_TYPE "C P L X"

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
		std::set<int> siFilteredIn;

		bool bFilterOutTypes;
		std::set<std::string> ssFilteredTypes;
		std::set<int> siFilteredOut;

		bool bOnlyTypes;
		std::set<std::string> ssTypes;


		bool bAP242Products;

		bool bError;
	};

	

} //opt

#endif //OPTIONS_H