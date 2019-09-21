#include "stdafx.h"
#include <sstream>
#include "JSONWriter.h"


JSONWriter::JSONWriter(std::string& output)
	: out(output)
{
	State current;
	current.hasContent = false;
	current.type = OBJECT;
	state.push(current);
	out.append("{");
}

JSONWriter::~JSONWriter()
{
	while (!state.empty()) {
		end();
	}
}

JSONWriter& JSONWriter::add(const std::string& name, const char* value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':'
		<< '"' << value << '"';

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, const std::string& value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' 
		<< '"' << value << '"';

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, int value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' << value;

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, unsigned long value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' << value;

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, float value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' << value;

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, double value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' << value;

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::add(const std::string& name, bool value)
{
	std::ostringstream os;

	os << '"' << name << '"'
		<< ':' << (value ? "true" : "false");

	if (state.top().hasContent) {
		out.append(",");
	}
	out.append(os.str());
	state.top().hasContent = true;
	return *this;
}

JSONWriter& JSONWriter::object(const std::string& name)
{
	if (state.top().hasContent) {
		out.append(",");
	}
	out.append("\"");
	out.append(name);
	out.append("\":{");

	State current;
	current.hasContent = false;
	current.type = OBJECT;
	state.push(current);

	return *this;
}

JSONWriter& JSONWriter::array(const std::string& name)
{
	if (state.top().hasContent) {
		out.append(",");
	}
	out.append("\"");
	out.append(name);
	out.append("\":[");

	State current;
	current.hasContent = false;
	current.type = ARRAY;
	state.push(current);

	return *this;
}

JSONWriter& JSONWriter::end()
{
	State s = state.top();
	switch (s.type) {
	case OBJECT: out.append("}"); break;
	case ARRAY: out.append("]"); break;
	}
	state.pop();
	if (!state.empty()) {
		state.top().hasContent = true; // as parent now has an object or array
	}
	return *this;
}
