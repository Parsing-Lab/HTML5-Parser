
// <meta/> no accepted?, <meta>

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
	// clear!
};

Header ParseHTML(std::string str);
Attribute ParseAttribute(std::string str);
std::pair<size_t, size_t> FindTag(const std::string& str, size_t start, size_t& tag_name_last);
	// if not found, then first > last!

void Print(Tag* tag) {

}

int main(void)
{
	std::string test_data1 = "<test> abcd </test>";
	std::string test_data2 = "<test a = \"1\" b = 3> aa <WOW a=\"2\" b=4>cc </WOW> dd <TEST> ! </TEST> ww </test>";
	Header h1;
	Header h2;

	{
		h1 = ParseHTML(std::move(test_data1));
	}
	std::cout << "-------";
	{
		h2 = ParseHTML(std::move(test_data2));
	}
	
	return 0;
}


std::pair<Tag*, size_t> _ParseHTML(std::string str, std::stack<std::string>& tag_stack) // check attribute?
{
	std::vector<Tag*> nestedTag;
	Tag* tag = new Tag();
	size_t start = 0;
	size_t last = 0;
	nestedTag.push_back(tag);
	
	// non tag?
	{
		auto pos = FindTag(str, start, last);

		if (pos.first > pos.second) { // tag does not exist. 
			tag->data.push_back(str);
			return { tag, str.size() - 1 };
		}
	}
	//
	while (true) {
		auto pos = FindTag(str, start, last);

		if (pos.first > pos.second) { // tag does not exist. 
			break;
		}
		else { // tag exists.
			// check attribute!
			//										<abc
			std::string tagName = str.substr(pos.first, last - pos.first + 1);
			tagName = tagName.substr(1, tagName.size() - 1);
			//										<abc /> <abc/>
			Attribute attr;
			
			if (last + 1 < pos.second) {
				attr = ParseAttribute(str.substr(last + 1, pos.second-1 - last));
			}

			if (tag_stack.size() > 0) {
				if (tagName == "/" + tag_stack.top()) { // temp is start tag, and tag_stack.top() is end tag?
					if (pos.first - start > 0) {
						nestedTag.back()->data.push_back(str.substr(start, pos.first - start)); // 0 : start, pos.first : end
					}
					tag_stack.pop();
					nestedTag.pop_back();
					if (attr.data.empty() == false) {
						// throw error.
						throw "/~, attr is not empty.";
					}
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
					nestedTag.back()->attr = attr;

					start = pos.second + 1;
					tag_stack.push(std::move(tagName));

					/// todo
					// if tagName is just <tagName> -> tag_stack.pop();
					//   and nestedTag.pop_back();
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
				nestedTag.back()->attr = attr;

				start = pos.second + 1;
				tag_stack.push(std::move(tagName));

				/// todo
				// if tagName is just <tagName> or <tagName/> - no??? -> tag_stack.pop();
				//   and nestedTag.pop_back();
			}

			if (0 >= str.size() - (pos.second + 1)) {
				return { tag, pos.second };
			}
		}
	}

	return { tag, start - 1 };
}

inline Header ParseHTML(std::string str)
{
	std::stack<std::string> tag_stack;
	Header header;
	header.head = _ParseHTML(std::move(str), tag_stack).first;
	return header;
}
inline int checkDelimiter(const std::string& statement, const int start, const std::vector<std::string>& delimiter)
{
	int sum = 0;
	const int _size = delimiter.size();

	for (int delim_num = 0; delim_num < _size; ++delim_num) {
		sum = 0;

		// size check
		if (start + delimiter[delim_num].size() - 1 > statement.size() - 1) {
			continue;
		}

		const int last = start + delimiter[delim_num].size() - 1;
		for (int i = start; last >= start && i <= last; ++i) {
			if (statement[i] == delimiter[delim_num][i - start]) {

			}
			else {
				sum--;
				break;
			}
		}
		sum++;

		if (sum > 0) {
			return delim_num;
		}
	}

	return -1;
}
std::pair<size_t, size_t> FindTag(const std::string& str, size_t start, size_t& tag_name_last)
{
	size_t a = start;
	size_t b;

	// check '~' or check "~" ??, in html '~' ??
	int state = 0;
	bool is_in_comment = false;
	bool tag_name_last_checked = false;

	for (size_t i = start; i < str.size(); ++i) {
		if (is_in_comment) {
			if (checkDelimiter(str, i, { "-->" }) > 0) {
				i += 2;
				is_in_comment = false;
			}
		}
		else if (0 == state && str[i] == '<') {
			if (checkDelimiter(str, i + 1, { "!--" }) > 0) {
				is_in_comment = true;
				i += 3;
			}
			else {
				state = 1;
				a = i;
			}
		}
		else if (0 == state && str[i] == '\"') { // cf) '\'' ?
			state = 2;
		}
		else if (2 == state && str[i] == '\"') { // cf \\" ?
			state = 0;
		}
		else if (1 == state && checkDelimiter(str, i, { "/>" }) > 0) { // pass?
			state = 0;
			b = i; 
			if (!tag_name_last_checked) {
				tag_name_last = i - 1;
			}
			tag_name_last_checked = true;
			return { a, b };
		}
		else if (1 == state && str[i] == '>') {
			state = 0;
			b = i;
			if (!tag_name_last_checked) {
				tag_name_last = i - 1;
			}
			tag_name_last_checked = true;
			return { a, b };
		}
		else if (1 == state && false == tag_name_last_checked && isspace(str[i])) {
			tag_name_last = i - 1;
			tag_name_last_checked = true;
		}
	}

	return { 2, 1 };
}


Attribute ParseAttribute(std::string str)
{
	if (str.empty()) { return Attribute(); }
	Attribute attr;

	// to do!
	int state = 0;
	size_t start = 0; // name or value `s start
	size_t last = 0;  // name or vlaue `s last
	std::string name;
	std::string value;
	bool check = false;

	for (size_t i = 0; i < str.size(); ++i) {
		// name = 
		if (state == 0 && !isspace(str[i])) {
			state = 1;

			start = i;
			last = i;
		}
		else if (state == 1 && str[i] == '\"') {
			state = 2;
		}
		else if (state == 2 && str[i] == '\"') {
			state = 1;
		}
		else if (state == 1 && isspace(str[i])) {
			last = i - 1;
			state = 3;
			check = true;
			name = str.substr(start, last - start + 1);
		}
		else if (state == 1 && str[i] == '=') {
			if (check == false) {
				last = i - 1;
				name = str.substr(start, last - start + 1);
			}
			state = 4;
			check = true;
		}
		else if (state == 3 && str[i] == '=') {
			state = 4;
		}
		// value
		else if (state == 4 && !isspace(str[i])) {
			state = 5;

			start = i;
			last = i;
			
			check = false;
			
			if (check == false && i == str.size() - 1) {
				last = i;
				value = str.substr(start, last - start + 1);

				attr.data.insert(std::make_pair(name, value));

				state = 0;
				check = false;
			}
			else {
				--i;
			}
		}
		else if (state == 5 && str[i] == '\"') {
			state = 6;
		}
		else if (state == 6 && str[i] == '\"') { //  \\" ?
			state = 5;

			if (i == str.size() - 1) {
				if (check == false) {
					last = i;
					value = str.substr(start, last - start + 1);

					attr.data.insert(std::make_pair(name, value));
				}
				state = 0;
				check = false;
			}
		}
		else if (state == 5 && isspace(str[i])) {
			last = i - 1;
			state = 0;
			check = false;
			value = str.substr(start, last - start + 1);

			attr.data.insert(std::make_pair(name, value));
		}
		else if (state == 5 && i == str.size() - 1) {
			if (check == false) {
				last = i;
				value = str.substr(start, last - start + 1);

				attr.data.insert(std::make_pair(name, value));
			}
			state = 0;
			check = false;
		}
	}

	return attr;
}

