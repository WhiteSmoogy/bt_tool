#include "BT.h"

std::string const BTDict::key_table[] = {
	"name", "name.utf-8",
	"path", "path.utf-8",
	"comment", "comment.utf-8",
	"publisher", "publisher-url", "publisher-url.utf-8", "publisher.utf-8"
};


void BTList::replace(const std::vector<std::pair<std::string, std::string>>& pairs) {
	for (auto & ptr : data) {
		if (auto dict_ptr = dynamic_cast<BTDict*>(ptr.get()))
			dict_ptr->replace(pairs);
	}
}