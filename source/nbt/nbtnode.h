#ifndef NBT_NODE_INCLUDED
#define NBT_NODE_INCLUDED

#include "../tools/typesbysize.h"
#include "compressed.h"
#include <string>
#include <vector>
#include <map>


class Node
{
public:
	virtual ~Node() {}

	virtual const Node* get(const std::string&) const { return 0; }
	virtual const Node* get(size_t) const { return 0; }
	virtual size_t size() const { return 0; }
};



class NodeCompound : public Node
{
public:
	NodeCompound() {}
	~NodeCompound();
	explicit NodeCompound(gzFile file);
	explicit NodeCompound(CompressedNBT& input);

	virtual const Node* get(const std::string& key) const;

private:
	typedef std::map<const std::string, Node*> ElementTable;
	ElementTable elements;
};



class NodeList : public Node
{
public:
	explicit NodeList(gzFile file);
	explicit NodeList(CompressedNBT& input);
	~NodeList();

	virtual const Node* get(size_t index) const;
	virtual size_t size() const { return entries_.size(); }
	
private:
	std::vector<Node*> entries_;
};



class NodeArray : public Node
{
public:
	explicit NodeArray(gzFile file);
	explicit NodeArray(CompressedNBT& input);
	const std::vector<uint8>& data() const { return data_; }
	virtual size_t size() const { return data_.size(); }

private:
	std::vector<uint8> data_;	
};



class NodeString : public Node
{
public:
	explicit NodeString(gzFile file);
	explicit NodeString(CompressedNBT& input);
	const std::string& text() const { return text_; } 

private:
	std::string text_;
};



template<typename T> 
class NodeValue : public Node
{
public:
	explicit NodeValue(gzFile file);
	explicit NodeValue(CompressedNBT& input);
	T value() const { return value_; }

private:
	T value_;
};


#endif
