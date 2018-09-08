
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <stack>


class Attribute
{
public:
	std::map<std::string, std::string> data;
};

class Tag {
public:
	std::string name;
	Attribute attr;
	std::vector<std::string> data; // valid : data.size() == inner_data.size() + 1
	std::vector<Tag*> inner_data;
public:
	virtual ~Tag() {

	}
};

// for tree structure.
class Header {
public:
	Tag* head = nullptr;
public:
	// merge(concat?)
	// split?
	// insert
	// remove?
	// extract.
	// change?
	// clear
};

Header Parse(std::string str);
std::pair<size_t, size_t> FindTag(const std::string& str, size_t start); 
	// if not found, then first > last!

void Print(Tag* tag) {

}

int main(void)
{
	std::string test_data1 = "<test> abcd </test>";
	std::string test_data2 = "<test> aa <WOW>cc </WOW> dd <TEST> ! </TEST> ww </test>";
	Header h1;
	Header h2;

	{
		h1 = Parse(std::move(test_data1));
	}
	std::cout << "-------";
	{
		h2 = Parse(std::move(test_data2));
	}
	
	getchar();
	return 0;
}

std::pair<Tag*, size_t> _Parse(std::string str, std::stack<std::string>& tag_stack) // check attribute?
{
	std::vector<Tag*> nestedTag;
	Tag* tag = new Tag();
	size_t start = 0;

	nestedTag.push_back(tag);
	
	// non tag?
	{
		auto pos = FindTag(str, start);

		if (pos.first > pos.second) { // tag does not exist. 
			tag->data.push_back(str);
			return { tag, str.size() - 1 };
		}
	}
	//
	while (true) {
		auto pos = FindTag(str, start);

		if (pos.first > pos.second) { // tag does not exist. 
			break;
		}
		else { // tag exists.
			// check attribute!
			//
			std::string tagName = str.substr(pos.first, pos.second - pos.first + 1);
			tagName = tagName.substr(1, tagName.size() - 2);

			if (tag_stack.size() > 0) {
				if (tagName == "/" + tag_stack.top()) { // temp is start tag, and tag_stack.top() is end tag?
					if (pos.first - start > 0) {
						nestedTag.back()->data.push_back(str.substr(start, pos.first - start)); // 0 : start, pos.first : end
					}
					tag_stack.pop();
					nestedTag.pop_back();
					
					start = pos.second + 1;
				}
				else {
					Tag* temp_ptr = new Tag();

					nestedTag.back()->inner_data.push_back(temp_ptr);
					
					if (pos.first - start > 0) {
						nestedTag.back()->data.push_back(str.substr(start, pos.first - start)); // 0 : start, pos.first : end
					}
					nestedTag.push_back(temp_ptr);
					nestedTag.back()->name = tagName;

					start = pos.second + 1;
					tag_stack.push(std::move(tagName));
				}
			}
			else {
				Tag* temp_ptr = new Tag();

				nestedTag.back()->inner_data.push_back(temp_ptr);
				
				if (pos.first - start > 0) {
					nestedTag.back()->data.push_back(str.substr(start, pos.first - start)); // 0 : start, pos.first : end
				}

				nestedTag.push_back(temp_ptr);
				nestedTag.back()->name = tagName;

				start = pos.second + 1;
				tag_stack.push(std::move(tagName));
			}

			if (0 >= str.size() - (pos.second + 1)) {
				size_t last = pos.second;
				return { tag, last };
			}
		}
	}

	return { tag, start - 1 };
}

Header Parse(std::string str)
{
	std::stack<std::string> tag_stack;
	Header header;
	header.head = _Parse(std::move(str), tag_stack).first;
	return header;
}

std::pair<size_t, size_t> FindTag(const std::string& str, size_t start)
{
	size_t a = start;
	size_t b;

	// check '~' or check "~" ??, in html '~' ??
	int state = 0;

	for (size_t i = start; i < str.size(); ++i) {
		if (0 == state && str[i] == '<') {
			state = 1;
			a = i;
		}
		else if (0 == state && str[i] == '\"') { // cf) '\'' ?
			state = 2;
		}
		else if (2 == state && str[i] == '\"') {
			state = 0;
		}
		else if (1 == state && str[i] == '>') {
			state = 0;
			b = i;
			return { a, b };
		}
	}

	return { 2, 1 };
}

