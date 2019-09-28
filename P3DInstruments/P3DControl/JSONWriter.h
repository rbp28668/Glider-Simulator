#pragma once
#include <string>
#include <stack>

// Class to write JSON to an output string.
class JSONWriter
{
	typedef enum entry_type_t {
		OBJECT,
		ARRAY
	} EntryType;

	typedef struct state_t {
		EntryType type;
		bool hasContent;
	} State;

	std::stack<State> state;

	std::string& out;
	
public:
	JSONWriter(std::string& output);
	~JSONWriter();

	JSONWriter& add(const std::string& name, const char* value);
	JSONWriter& add(const std::string& name, const std::string& value);
	JSONWriter& add(const std::string& name, int value);
	JSONWriter& add(const std::string& name, unsigned long value);
	JSONWriter& add(const std::string& name, float value);
	JSONWriter& add(const std::string& name, double value);
	JSONWriter& add(const std::string& name, bool value);
	JSONWriter& object(const std::string& name = "");
	JSONWriter& array(const std::string& name);
	JSONWriter& end();


};

