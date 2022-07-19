#include "Options.h"

namespace opt {
	std::set<std::string> ssFilteredOutTypes({
		
		});



	Options::Options() : sFileName(""), bMaxDepth(false), iMaxDepth(0), bError(false), bNoDuplicates(false), treated(std::set<int>()),
	bFilterId(false), iFilterId(0), containingId(std::set<int>()), bFilterType(false), sFilterType(""), siFilteredIn(std::set<int>()),
	bFilterOutTypes(false), ssFilteredTypes(std::set<std::string>()), siFilteredOut(std::set<int>()),
	bOnlyTypes(false), ssTypes(std::set<std::string>()),
	bAP242Products(false),
	bAP242ProductNameFilter(false), sProductName("") {

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
					continue;
				}

				if (sArg == "-n" || sArg == "--noDuplicates") {
					bNoDuplicates = true;
					continue;
				}

				if (sArg == "-i" || sArg == "--filterID") {
					if (bFilterType) {
						std::cerr << "Error: ID filter and type filter are incompatible (for now)" << std::endl;
						bError = true;
						return;
					}

					bFilterId = true;

					if (!argv[iArg + 1]) {
						bError = true;
						return;
					}

					iFilterId = std::stoi(std::string(argv[iArg + 1]));
					bSkipOne = true;
					continue;
				}

				if (sArg == "-t" || sArg == "--filterType") {
					if (bFilterId) {
						std::cerr << "Error: ID filter and type filter are incompatible (for now)" << std::endl;
						bError = true;
						return;
					}

					bFilterType = true;

					if (!argv[iArg + 1]) {
						bError = true;
						return;
					}

					sFilterType = std::string(argv[iArg + 1]);
					bSkipOne = true;
					continue;
				}

				if (sArg == "-T" || sArg == "--filterTypesOut") {

					bFilterOutTypes = true;
					ssFilteredTypes = ssFilteredOutTypes;
					continue;
				}

				if (sArg == "-o" || sArg == "--onlyTypes") {
					bOnlyTypes = true;
					continue;
				}


				if (sArg == "--AP242Products") {
					if (bFilterId || bFilterType) {
						std::cerr << "AP242 product display is not compatible with id or type filter" << std::endl;
						bError = true;
						return;
					}
					bAP242Products = true;
					continue;
				}

				if (sArg == "--AP242ProductsFilterName") {
					bAP242ProductNameFilter = true;

					if (!argv[iArg + 1]) {
						bError = true;
						return;
					}

					sProductName = std::string(argv[iArg + 1]);
					bSkipOne = true;
					continue;
				}

				std::cerr << "Unrecognized option: `" << "`" << std::endl;
				bError = true;
				return;

			}
			catch (...) {
				std::cerr << "Error while parsing options" << std::endl;
				bError = true;
				return;
			}
		}

		if (bAP242ProductNameFilter && !bAP242Products) {
			std::cerr << "Error: cannot filter by AP242 product name if not filtering by AP242 product" << std::endl;
			bError = true;
		}
	}
} //opt