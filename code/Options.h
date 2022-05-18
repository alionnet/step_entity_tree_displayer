
#include <iostream>
#include <string>

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


		bool bError;
	};

	

} //opt

#endif //OPTIONS_H