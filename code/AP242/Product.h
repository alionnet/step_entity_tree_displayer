#include <string>
#include <map>

#include "../EntityUtil.h"


#ifndef PRODUCT_H
#define PRODUCT_H

#define PRODUCT_ENTITY "PRODUCT"

class Product {
public:
	Product(std::string _identifier) : identifier(_identifier), siEntitiesUsing(std::set<int>()) {

	}

	std::string identifier;
	std::set<int> siEntitiesUsing;
};

typedef std::map<std::string, Product> ProductMap;

void addProduct(ProductMap& products, const std::string& identifier);
void addEntityUsingProduct(ProductMap& products, const std::string& identifier, int id);

#endif // !PRODUCT_H