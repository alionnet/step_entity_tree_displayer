#include "Product.h"

/*
* @brief Creates a product using the identifier and adds it to a ProductMap
*/
void addProduct(ProductMap& products, const std::string& identifier) {
	Product p(identifier);

	products.insert({ identifier,p });
}

/*
* @brief Indicates that a given entity uses the product identifier
* @param products:		ProductMap referencing all products
* @param identifier:	Found identifier
* @param id:			id of the entity using the identifier
*/
void addEntityUsingProduct(ProductMap& products, const std::string& identifier, int id) {
	auto& prod = products.find(identifier);

	if (prod != products.end()) {
		prod->second.siEntitiesUsing.insert(id);
	}
}