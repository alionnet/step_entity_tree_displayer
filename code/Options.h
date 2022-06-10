
#include <iostream>
#include <string>

#include <set>

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


		bool bError;
	};

	

} //opt

#endif //OPTIONS_H