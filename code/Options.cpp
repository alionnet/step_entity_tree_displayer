#include "Options.h"

namespace opt {
	Options::Options() : bMaxDepth(false), iMaxDepth(0), bError(false) {

	}

	void Options::parseArgs(int argc, char* argv[]) {
		if (argc < 2) {
			bError = true;
			return;
		}

		sFileName = std::string(argv[1]);

		bool bSkipOne = false;
		for (int iArg = 2; iArg < argc; ++iArg) {
			if (bSkipOne) {
				bSkipOne = false;
				continue;
			}

			std::string sArg = std::string(argv[iArg]);

			try {
				if (sArg == "-d") {
					bMaxDepth = true;

					if (!argv[iArg + 1]) {
						bError = true;
						return;
					}

					iMaxDepth = std::stoi(std::string(argv[iArg + 1]));
					bSkipOne = true;
				}

			}
			catch (...) {
				std::cerr << "Error while parsing options" << std::endl;
				bError = true;
				return;
			}
		}
	}
} //opt