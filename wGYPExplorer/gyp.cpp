#include "stdafx.h"
#include "gyp.h"

#include <utility>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <iomanip>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

#define GYP_ASSERT_UNREACHABLE assert( false )
#define GYP_ASSERT( condition ) assert( condition );  // @todo <= change this into an exception throw
#define GYP_ASSERT_MESSAGE( condition, message ) if (!( condition )) throw std::runtime_error( message );

namespace gyp {

const Value Value::null;
const Int Value::minInt = Int(~(UInt(-1) / 2));
const Int Value::maxInt = Int(UInt(-1) / 2);
const UInt Value::maxUInt = UInt(-1);


ValueAllocator::~ValueAllocator()
{
}

class DefaultValueAllocator : public ValueAllocator
{
public:
	virtual ~DefaultValueAllocator()
	{
	}

	virtual char *makeMemberName(const char *memberName)
	{
		return duplicateStringValue(memberName);
	}

	virtual void releaseMemberName(char *memberName)
	{
		releaseStringValue(memberName);
	}

	virtual char *duplicateStringValue(const char *value,
		unsigned int length = unknown)
	{
		if (length == unknown)
			length = (unsigned int)strlen(value);
		char *newString = static_cast<char *>(malloc(length + 1));
		memcpy(newString, value, length);
		newString[length] = 0;
		return newString;
	}

	virtual void releaseStringValue(char *value)
	{
		if (value)
			free(value);
	}
};

static ValueAllocator *&valueAllocator()
{
	static DefaultValueAllocator defaultAllocator;
	static ValueAllocator *valueAllocator = &defaultAllocator;
	return valueAllocator;
}

static struct DummyValueAllocatorInitializer {
	DummyValueAllocatorInitializer()
	{
		valueAllocator();      // ensure valueAllocator() statics are initialized before main().
	}
} dummyValueAllocatorInitializer;

// class ValueIteratorBase
ValueIteratorBase::ValueIteratorBase()
	: current_()
	, isNull_(true)
{
}

ValueIteratorBase::ValueIteratorBase(const Value::ObjectValues::iterator &current)
	: current_(current)
	, isNull_(false)
{
}

Value &
ValueIteratorBase::deref() const
{
	return current_->second;
}

void
ValueIteratorBase::increment()
{
	++current_;
}


void
ValueIteratorBase::decrement()
{
	--current_;
}

ValueIteratorBase::difference_type
ValueIteratorBase::computeDistance(const SelfType &other) const
{
	if (isNull_  &&  other.isNull_)
	{
		return 0;
	}


	// Usage of std::distance is not portable (does not compile with Sun Studio 12 RogueWave STL,
	// which is the one used by default).
	// Using a portable hand-made version for non random iterator instead:
	//   return difference_type( std::distance( current_, other.current_ ) );
	difference_type myDistance = 0;
	for (Value::ObjectValues::iterator it = current_; it != other.current_; ++it)
	{
		++myDistance;
	}
	return myDistance;
}

bool
ValueIteratorBase::isEqual(const SelfType &other) const
{
	if (isNull_)
	{
		return other.isNull_;
	}
	return current_ == other.current_;
}

void
ValueIteratorBase::copy(const SelfType &other)
{
	current_ = other.current_;
}

Value
ValueIteratorBase::key() const
{
	const Value::CZString czstring = (*current_).first;
	if (czstring.c_str())
	{
		if (czstring.isStaticString())
			return Value(StaticString(czstring.c_str()));
		return Value(czstring.c_str());
	}
	return Value(czstring.index());
}

UInt
ValueIteratorBase::index() const
{
	const Value::CZString czstring = (*current_).first;
	if (!czstring.c_str())
		return czstring.index();
	return Value::UInt(-1);
}

const char *
ValueIteratorBase::memberName() const
{
	const char *name = (*current_).first.c_str();
	return name ? name : "";
}


// class ValueConstIterator
ValueConstIterator::ValueConstIterator()
{
}

ValueConstIterator::ValueConstIterator(const Value::ObjectValues::iterator &current)
	: ValueIteratorBase(current)
{
}

ValueConstIterator &
ValueConstIterator::operator =(const ValueIteratorBase &other)
{
	copy(other);
	return *this;
}


// class ValueIterator
ValueIterator::ValueIterator()
{
}

ValueIterator::ValueIterator(const Value::ObjectValues::iterator &current)
	: ValueIteratorBase(current)
{
}

ValueIterator::ValueIterator(const ValueConstIterator &other)
	: ValueIteratorBase(other)
{
}

ValueIterator::ValueIterator(const ValueIterator &other)
	: ValueIteratorBase(other)
{
}

ValueIterator &
ValueIterator::operator =(const SelfType &other)
{
	copy(other);
	return *this;
}


// class Value::CommentInfo
Value::CommentInfo::CommentInfo()
	: comment_(0)
{
}

Value::CommentInfo::~CommentInfo()
{
	if (comment_)
		valueAllocator()->releaseStringValue(comment_);
}

void
Value::CommentInfo::setComment(const char *text)
{
	if (comment_)
		valueAllocator()->releaseStringValue(comment_);
	GYP_ASSERT(text);
	GYP_ASSERT_MESSAGE(text[0] == '\0' || text[0] == '#', "Comments must start with #");
	// It seems that /**/ style comments are acceptable as well.
	comment_ = valueAllocator()->duplicateStringValue(text);
}


// class Value::CZString
Value::CZString::CZString(int index)
	: cstr_(0)
	, index_(index)
{
}

Value::CZString::CZString(const char *cstr, DuplicationPolicy allocate)
	: cstr_(allocate == duplicate ? valueAllocator()->makeMemberName(cstr)
	: cstr)
	, index_(allocate)
{
}

Value::CZString::CZString(const CZString &other)
	: cstr_(other.index_ != noDuplication &&  other.cstr_ != 0
	? valueAllocator()->makeMemberName(other.cstr_)
	: other.cstr_)
	, index_(other.cstr_ ? (other.index_ == noDuplication ? noDuplication : duplicate)
	: other.index_)
{
}

Value::CZString::~CZString()
{
	if (cstr_  &&  index_ == duplicate)
		valueAllocator()->releaseMemberName(const_cast<char *>(cstr_));
}

void
Value::CZString::swap(CZString &other)
{
	std::swap(cstr_, other.cstr_);
	std::swap(index_, other.index_);
}

Value::CZString &
Value::CZString::operator =(const CZString &other)
{
	CZString temp(other);
	swap(temp);
	return *this;
}

bool
Value::CZString::operator<(const CZString &other) const
{
	if (cstr_)
		return strcmp(cstr_, other.cstr_) < 0;
	return index_ < other.index_;
}

bool
Value::CZString::operator==(const CZString &other) const
{
	if (cstr_)
		return strcmp(cstr_, other.cstr_) == 0;
	return index_ == other.index_;
}


int
Value::CZString::index() const
{
	return index_;
}

const char *
Value::CZString::c_str() const
{
	return cstr_;
}

bool
Value::CZString::isStaticString() const
{
	return index_ == noDuplication;
}


// class Value::Value
Value::Value(ValueType type)
	: type_(type)
	, allocated_(0)
	, comments_(0)
{
	switch (type)
	{
	case nullValue:
		break;
	case intValue:
	case uintValue:
		value_.int_ = 0;
		break;
	case realValue:
		value_.real_ = 0.0;
		break;
	case stringValue:
		value_.string_ = 0;
		break;
	case arrayValue:
	case objectValue:
		value_.map_ = new ObjectValues();
		break;
	case booleanValue:
		value_.bool_ = false;
		break;
	default:
		GYP_ASSERT_UNREACHABLE;
	}
}

Value::Value(Int value)
	: type_(intValue)
	, comments_(0)
{
	value_.int_ = value;
}

Value::Value(UInt value)
	: type_(uintValue)
	, comments_(0)
{
	value_.uint_ = value;
}

Value::Value(double value)
	: type_(realValue)
	, comments_(0)
{
	value_.real_ = value;
}

Value::Value(const char *value)
	: type_(stringValue)
	, allocated_(true)
	, comments_(0)
{
	value_.string_ = valueAllocator()->duplicateStringValue(value);
}

Value::Value(const char *beginValue, const char *endValue)
	: type_(stringValue)
	, allocated_(true)
	, comments_(0)
{
	value_.string_ = valueAllocator()->duplicateStringValue(beginValue,
		UInt(endValue - beginValue));
}

Value::Value(const std::string &value)
	: type_(stringValue)
	, allocated_(true)
	, comments_(0)
{
	value_.string_ = valueAllocator()->duplicateStringValue(value.c_str(),
		(unsigned int)value.length());
}

Value::Value(const StaticString &value)
	: type_(stringValue)
	, allocated_(false)
	, comments_(0)
{
	value_.string_ = const_cast<char *>(value.c_str());
}

Value::Value(bool value)
	: type_(booleanValue)
	, comments_(0)
{
	value_.bool_ = value;
}

Value::Value(const Value &other)
	: type_(other.type_)
	, comments_(0)
{
	switch (type_)
	{
	case nullValue:
	case intValue:
	case uintValue:
	case realValue:
	case booleanValue:
		value_ = other.value_;
		break;
	case stringValue:
		if (other.value_.string_)
		{
			value_.string_ = valueAllocator()->duplicateStringValue(other.value_.string_);
			allocated_ = true;
		}
		else
			value_.string_ = 0;
		break;
	case arrayValue:
	case objectValue:
		value_.map_ = new ObjectValues(*other.value_.map_);
		break;
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	if (other.comments_)
	{
		comments_ = new CommentInfo[numberOfCommentPlacement];
		for (int comment = 0; comment < numberOfCommentPlacement; ++comment)
		{
			const CommentInfo &otherComment = other.comments_[comment];
			if (otherComment.comment_)
				comments_[comment].setComment(otherComment.comment_);
		}
	}
}

Value::~Value()
{
	switch (type_)
	{
	case nullValue:
	case intValue:
	case uintValue:
	case realValue:
	case booleanValue:
		break;
	case stringValue:
		if (allocated_)
			valueAllocator()->releaseStringValue(value_.string_);
		break;
	case arrayValue:
	case objectValue:
		delete value_.map_;
		break;
	default:
		GYP_ASSERT_UNREACHABLE;
	}

	if (comments_)
		delete[] comments_;
}

Value &
Value::operator=(const Value &other)
{
	Value temp(other);
	swap(temp);
	return *this;
}

void
Value::swap(Value &other)
{
	ValueType temp = type_;
	type_ = other.type_;
	other.type_ = temp;
	std::swap(value_, other.value_);
	int temp2 = allocated_;
	allocated_ = other.allocated_;
	other.allocated_ = temp2;
}

ValueType
Value::type() const
{
	return type_;
}


int
Value::compare(const Value &other)
{
	return 0;  // unreachable
}

bool
Value::operator <(const Value &other) const
{
	int typeDelta = type_ - other.type_;
	if (typeDelta)
		return typeDelta < 0 ? true : false;
	switch (type_)
	{
	case nullValue:
		return false;
	case intValue:
		return value_.int_ < other.value_.int_;
	case uintValue:
		return value_.uint_ < other.value_.uint_;
	case realValue:
		return value_.real_ < other.value_.real_;
	case booleanValue:
		return value_.bool_ < other.value_.bool_;
	case stringValue:
		return (value_.string_ == 0 && other.value_.string_)
			|| (other.value_.string_
			&&  value_.string_
			&& strcmp(value_.string_, other.value_.string_) < 0);
	case arrayValue:
	case objectValue:
	{
		int delta = int(value_.map_->size() - other.value_.map_->size());
		if (delta)
			return delta < 0;
		return (*value_.map_) < (*other.value_.map_);
	}
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0;  // unreachable
}

bool
Value::operator <=(const Value &other) const
{
	return !(other > *this);
}

bool
Value::operator >=(const Value &other) const
{
	return !(*this < other);
}

bool
Value::operator >(const Value &other) const
{
	return other < *this;
}

bool
Value::operator ==(const Value &other) const
{
	int temp = other.type_;
	if (type_ != temp)
		return false;
	switch (type_)
	{
	case nullValue:
		return true;
	case intValue:
		return value_.int_ == other.value_.int_;
	case uintValue:
		return value_.uint_ == other.value_.uint_;
	case realValue:
		return value_.real_ == other.value_.real_;
	case booleanValue:
		return value_.bool_ == other.value_.bool_;
	case stringValue:
		return (value_.string_ == other.value_.string_)
			|| (other.value_.string_
			&&  value_.string_
			&& strcmp(value_.string_, other.value_.string_) == 0);
	case arrayValue:
	case objectValue:
		return value_.map_->size() == other.value_.map_->size()
			&& (*value_.map_) == (*other.value_.map_);
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0;  // unreachable
}


bool
Value::operator !=(const Value &other) const
{
	return !(*this == other);
}

const char *
Value::asCString() const
{
	GYP_ASSERT(type_ == stringValue);
	return value_.string_;
}


std::string
Value::asString() const
{
	switch (type_)
	{
	case nullValue:
		return "";
	case stringValue:
		return value_.string_ ? value_.string_ : "";
	case booleanValue:
		return value_.bool_ ? "true" : "false";
	case intValue:
	case uintValue:
	case realValue:
	case arrayValue:
	case objectValue:
		GYP_ASSERT_MESSAGE(false, "Type is not convertible to string");
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return ""; // unreachable
}

Value::Int
Value::asInt() const
{
	switch (type_)
	{
	case nullValue:
		return 0;
	case intValue:
		return value_.int_;
	case uintValue:
		GYP_ASSERT_MESSAGE(value_.uint_ < (unsigned)maxInt, "integer out of signed integer range");
		return value_.uint_;
	case realValue:
		GYP_ASSERT_MESSAGE(value_.real_ >= minInt  &&  value_.real_ <= maxInt, "Real out of signed integer range");
		return Int(value_.real_);
	case booleanValue:
		return value_.bool_ ? 1 : 0;
	case stringValue:
	case arrayValue:
	case objectValue:
		GYP_ASSERT_MESSAGE(false, "Type is not convertible to int");
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0; // unreachable;
}

Value::UInt
Value::asUInt() const
{
	switch (type_)
	{
	case nullValue:
		return 0;
	case intValue:
		GYP_ASSERT_MESSAGE(value_.int_ >= 0, "Negative integer can not be converted to unsigned integer");
		return value_.int_;
	case uintValue:
		return value_.uint_;
	case realValue:
		GYP_ASSERT_MESSAGE(value_.real_ >= 0 && value_.real_ <= maxUInt, "Real out of unsigned integer range");
		return UInt(value_.real_);
	case booleanValue:
		return value_.bool_ ? 1 : 0;
	case stringValue:
	case arrayValue:
	case objectValue:
		GYP_ASSERT_MESSAGE(false, "Type is not convertible to uint");
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0; // unreachable;
}

double
Value::asDouble() const
{
	switch (type_)
	{
	case nullValue:
		return 0.0;
	case intValue:
		return value_.int_;
	case uintValue:
		return value_.uint_;
	case realValue:
		return value_.real_;
	case booleanValue:
		return value_.bool_ ? 1.0 : 0.0;
	case stringValue:
	case arrayValue:
	case objectValue:
		GYP_ASSERT_MESSAGE(false, "Type is not convertible to double");
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0; // unreachable;
}

bool
Value::asBool() const
{
	switch (type_)
	{
	case nullValue:
		return false;
	case intValue:
	case uintValue:
		return value_.int_ != 0;
	case realValue:
		return value_.real_ != 0.0;
	case booleanValue:
		return value_.bool_;
	case stringValue:
		return value_.string_  &&  value_.string_[0] != 0;
	case arrayValue:
	case objectValue:
		return value_.map_->size() != 0;
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return false; // unreachable;
}


bool
Value::isConvertibleTo(ValueType other) const
{
	switch (type_)
	{
	case nullValue:
		return true;
	case intValue:
		return (other == nullValue  &&  value_.int_ == 0)
			|| other == intValue
			|| (other == uintValue  && value_.int_ >= 0)
			|| other == realValue
			|| other == stringValue
			|| other == booleanValue;
	case uintValue:
		return (other == nullValue  &&  value_.uint_ == 0)
			|| (other == intValue  && value_.uint_ <= (unsigned)maxInt)
			|| other == uintValue
			|| other == realValue
			|| other == stringValue
			|| other == booleanValue;
	case realValue:
		return (other == nullValue  &&  value_.real_ == 0.0)
			|| (other == intValue  &&  value_.real_ >= minInt  &&  value_.real_ <= maxInt)
			|| (other == uintValue  &&  value_.real_ >= 0 && value_.real_ <= maxUInt)
			|| other == realValue
			|| other == stringValue
			|| other == booleanValue;
	case booleanValue:
		return (other == nullValue  &&  value_.bool_ == false)
			|| other == intValue
			|| other == uintValue
			|| other == realValue
			|| other == stringValue
			|| other == booleanValue;
	case stringValue:
		return other == stringValue
			|| (other == nullValue && (!value_.string_ || value_.string_[0] == 0));
	case arrayValue:
		return other == arrayValue
			|| (other == nullValue  &&  value_.map_->size() == 0);
	case objectValue:
		return other == objectValue
			|| (other == nullValue  &&  value_.map_->size() == 0);
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return false; // unreachable;
}

Value::UInt
Value::size() const
{
	switch (type_)
	{
	case nullValue:
	case intValue:
	case uintValue:
	case realValue:
	case booleanValue:
	case stringValue:
		return 0;
	case arrayValue:  // size of the array is highest index + 1
		if (!value_.map_->empty())
		{
			ObjectValues::const_iterator itLast = value_.map_->end();
			--itLast;
			return (*itLast).first.index() + 1;
		}
		return 0;
	case objectValue:
		return Int(value_.map_->size());
	default:
		GYP_ASSERT_UNREACHABLE;
	}
	return 0; // unreachable;
}

bool
Value::empty() const
{
	if (isNull() || isArray() || isObject())
		return size() == 0u;
	else
		return false;
}

bool
Value::operator!() const
{
	return isNull();
}

void
Value::clear()
{
	GYP_ASSERT(type_ == nullValue || type_ == arrayValue || type_ == objectValue);

	switch (type_)
	{
	case arrayValue:
	case objectValue:
		value_.map_->clear();
		break;
	default:
		break;
	}
}

void
Value::resize(UInt newSize)
{
	GYP_ASSERT(type_ == nullValue || type_ == arrayValue);
	if (type_ == nullValue)
		*this = Value(arrayValue);
	UInt oldSize = size();
	if (newSize == 0)
		clear();
	else if (newSize > oldSize)
		(*this)[newSize - 1];
	else
	{
		for (UInt index = newSize; index < oldSize; ++index)
			value_.map_->erase(index);
		assert(size() == newSize);
	}
}

Value &
Value::operator[](UInt index)
{
	GYP_ASSERT(type_ == nullValue || type_ == arrayValue);
	if (type_ == nullValue)
		*this = Value(arrayValue);
	CZString key(index);
	ObjectValues::iterator it = value_.map_->lower_bound(key);
	if (it != value_.map_->end() && (*it).first == key)
		return (*it).second;

	ObjectValues::value_type defaultValue(key, null);
	it = value_.map_->insert(it, defaultValue);
	return (*it).second;
}

const Value &
Value::operator[](UInt index) const
{
	GYP_ASSERT(type_ == nullValue || type_ == arrayValue);
	if (type_ == nullValue)
		return null;
	CZString key(index);
	ObjectValues::const_iterator it = value_.map_->find(key);
	if (it == value_.map_->end())
		return null;
	return (*it).second;
}

Value &
Value::operator[](const char *key)
{
	return resolveReference(key, false);
}

Value &
Value::resolveReference(const char *key,
bool isStatic)
{
	GYP_ASSERT(type_ == nullValue || type_ == objectValue);
	if (type_ == nullValue)
		*this = Value(objectValue);
	CZString actualKey(key, isStatic ? CZString::noDuplication
		: CZString::duplicateOnCopy);
	ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
	if (it != value_.map_->end() && (*it).first == actualKey)
		return (*it).second;

	ObjectValues::value_type defaultValue(actualKey, null);
	it = value_.map_->insert(it, defaultValue);
	Value &value = (*it).second;
	return value;
}

Value
Value::get(UInt index,
const Value &defaultValue) const
{
	const Value *value = &((*this)[index]);
	return value == &null ? defaultValue : *value;
}

bool
Value::isValidIndex(UInt index) const
{
	return index < size();
}

const Value &
Value::operator[](const char *key) const
{
	GYP_ASSERT(type_ == nullValue || type_ == objectValue);
	if (type_ == nullValue)
		return null;
	CZString actualKey(key, CZString::noDuplication);
	ObjectValues::const_iterator it = value_.map_->find(actualKey);
	if (it == value_.map_->end())
		return null;
	return (*it).second;
}

Value &
Value::operator[](const std::string &key)
{
	return (*this)[key.c_str()];
}

const Value &
Value::operator[](const std::string &key) const
{
	return (*this)[key.c_str()];
}

Value &
Value::operator[](const StaticString &key)
{
	return resolveReference(key, true);
}

Value &
Value::append(const Value &value)
{
	return (*this)[size()] = value;
}

Value
Value::get(const char *key,
const Value &defaultValue) const
{
	const Value *value = &((*this)[key]);
	return value == &null ? defaultValue : *value;
}

Value
Value::get(const std::string &key,
const Value &defaultValue) const
{
	return get(key.c_str(), defaultValue);
}

Value
Value::removeMember(const char* key)
{
	GYP_ASSERT(type_ == nullValue || type_ == objectValue);
	if (type_ == nullValue)
		return null;
	CZString actualKey(key, CZString::noDuplication);
	ObjectValues::iterator it = value_.map_->find(actualKey);
	if (it == value_.map_->end())
		return null;
	Value old(it->second);
	value_.map_->erase(it);
	return old;
}

Value
Value::removeMember(const std::string &key)
{
	return removeMember(key.c_str());
}

bool
Value::isMember(const char *key) const
{
	const Value *value = &((*this)[key]);
	return value != &null;
}

bool
Value::isMember(const std::string &key) const
{
	return isMember(key.c_str());
}

Value::Members
Value::getMemberNames() const
{
	GYP_ASSERT(type_ == nullValue || type_ == objectValue);
	if (type_ == nullValue)
		return Value::Members();
	Members members;
	members.reserve(value_.map_->size());
	ObjectValues::const_iterator it = value_.map_->begin();
	ObjectValues::const_iterator itEnd = value_.map_->end();
	for (; it != itEnd; ++it)
		members.push_back(std::string((*it).first.c_str()));
	return members;
}

bool
Value::isNull() const
{
	return type_ == nullValue;
}

bool
Value::isBool() const
{
	return type_ == booleanValue;
}

bool
Value::isInt() const
{
	return type_ == intValue;
}

bool
Value::isUInt() const
{
	return type_ == uintValue;
}

bool
Value::isIntegral() const
{
	return type_ == intValue
		|| type_ == uintValue
		|| type_ == booleanValue;
}

bool
Value::isDouble() const
{
	return type_ == realValue;
}

bool
Value::isNumeric() const
{
	return isIntegral() || isDouble();
}

bool
Value::isString() const
{
	return type_ == stringValue;
}

bool
Value::isArray() const
{
	return type_ == nullValue || type_ == arrayValue;
}

bool
Value::isObject() const
{
	return type_ == nullValue || type_ == objectValue;
}

void
Value::setComment(const char *comment,
CommentPlacement placement)
{
	if (!comments_)
		comments_ = new CommentInfo[numberOfCommentPlacement];
	comments_[placement].setComment(comment);
}

void
Value::setComment(const std::string &comment,
CommentPlacement placement)
{
	setComment(comment.c_str(), placement);
}

bool
Value::hasComment(CommentPlacement placement) const
{
	return comments_ != 0 && comments_[placement].comment_ != 0;
}

std::string
Value::getComment(CommentPlacement placement) const
{
	if (hasComment(placement))
		return comments_[placement].comment_;
	return "";
}

std::string
Value::toStyledString() const
{
	StyledWriter writer;
	return writer.write(*this);
}

Value::const_iterator
Value::begin() const
{
	switch (type_)
	{
	case arrayValue:
	case objectValue:
		if (value_.map_)
			return const_iterator(value_.map_->begin());
		break;
	default:
		break;
	}
	return const_iterator();
}

Value::const_iterator
Value::end() const
{
	switch (type_)
	{
	case arrayValue:
	case objectValue:
		if (value_.map_)
			return const_iterator(value_.map_->end());
		break;
	default:
		break;
	}
	return const_iterator();
}

Value::iterator
Value::begin()
{
	switch (type_)
	{
	case arrayValue:
	case objectValue:
		if (value_.map_)
			return iterator(value_.map_->begin());
		break;
	default:
		break;
	}
	return iterator();
}

Value::iterator
Value::end()
{
	switch (type_)
	{
	case arrayValue:
	case objectValue:
		if (value_.map_)
			return iterator(value_.map_->end());
		break;
	default:
		break;
	}
	return iterator();
}


// class PathArgument
PathArgument::PathArgument()
	: kind_(kindNone)
{
}

PathArgument::PathArgument(Value::UInt index)
	: index_(index)
	, kind_(kindIndex)
{
}

PathArgument::PathArgument(const char *key)
	: key_(key)
	, kind_(kindKey)
{
}

PathArgument::PathArgument(const std::string &key)
	: key_(key.c_str())
	, kind_(kindKey)
{
}


// class Path
Path::Path(const std::string &path,
	const PathArgument &a1,
	const PathArgument &a2,
	const PathArgument &a3,
	const PathArgument &a4,
	const PathArgument &a5)
{
	InArgs in;
	in.push_back(&a1);
	in.push_back(&a2);
	in.push_back(&a3);
	in.push_back(&a4);
	in.push_back(&a5);
	makePath(path, in);
}

void
Path::makePath(const std::string &path,
const InArgs &in)
{
	const char *current = path.c_str();
	const char *end = current + path.length();
	InArgs::const_iterator itInArg = in.begin();
	while (current != end)
	{
		if (*current == '[')
		{
			++current;
			if (*current == '%')
				addPathInArg(path, in, itInArg, PathArgument::kindIndex);
			else
			{
				Value::UInt index = 0;
				for (; current != end && *current >= '0' && *current <= '9'; ++current)
					index = index * 10 + Value::UInt(*current - '0');
				args_.push_back(index);
			}
			if (current == end || *current++ != ']')
				invalidPath(path, int(current - path.c_str()));
		}
		else if (*current == '%')
		{
			addPathInArg(path, in, itInArg, PathArgument::kindKey);
			++current;
		}
		else if (*current == '.')
		{
			++current;
		}
		else
		{
			const char *beginName = current;
			while (current != end  &&  !strchr("[.", *current))
				++current;
			args_.push_back(std::string(beginName, current));
		}
	}
}

void
Path::addPathInArg(const std::string &path,
const InArgs &in,
InArgs::const_iterator &itInArg,
PathArgument::Kind kind)
{
	if (itInArg == in.end())
	{
		// Error: missing argument %d
	}
	else if ((*itInArg)->kind_ != kind)
	{
		// Error: bad argument type
	}
	else
	{
		args_.push_back(**itInArg);
	}
}

void
Path::invalidPath(const std::string &path,
int location)
{
	// Error: invalid path.
}

const Value &
Path::resolve(const Value &root) const
{
	const Value *node = &root;
	for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
	{
		const PathArgument &arg = *it;
		if (arg.kind_ == PathArgument::kindIndex)
		{
			if (!node->isArray() || node->isValidIndex(arg.index_))
			{
				// Error: unable to resolve path (array value expected at position...
			}
			node = &((*node)[arg.index_]);
		}
		else if (arg.kind_ == PathArgument::kindKey)
		{
			if (!node->isObject())
			{
				// Error: unable to resolve path (object value expected at position...)
			}
			node = &((*node)[arg.key_]);
			if (node == &Value::null)
			{
				// Error: unable to resolve path (object has no member named '' at position...)
			}
		}
	}
	return *node;
}

Value
Path::resolve(const Value &root,
const Value &defaultValue) const
{
	const Value *node = &root;
	for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
	{
		const PathArgument &arg = *it;
		if (arg.kind_ == PathArgument::kindIndex)
		{
			if (!node->isArray() || node->isValidIndex(arg.index_))
				return defaultValue;
			node = &((*node)[arg.index_]);
		}
		else if (arg.kind_ == PathArgument::kindKey)
		{
			if (!node->isObject())
				return defaultValue;
			node = &((*node)[arg.key_]);
			if (node == &Value::null)
				return defaultValue;
		}
	}
	return *node;
}

Value &
Path::make(Value &root) const
{
	Value *node = &root;
	for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
	{
		const PathArgument &arg = *it;
		if (arg.kind_ == PathArgument::kindIndex)
		{
			if (!node->isArray())
			{
				// Error: node is not an array at position ...
			}
			node = &((*node)[arg.index_]);
		}
		else if (arg.kind_ == PathArgument::kindKey)
		{
			if (!node->isObject())
			{
				// Error: node is not an object at position...
			}
			node = &((*node)[arg.key_]);
		}
	}
	return *node;
}


// Implementation of class Features
Features::Features()
	: allowComments_(true)
	, strictRoot_(false)
{
}

Features
Features::all()
{
	return Features();
}

Features
Features::strictMode()
{
	Features features;
	features.allowComments_ = false;
	features.strictRoot_ = true;
	return features;
}


// Implementation of class Reader
static inline bool
in(Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4)
{
	return c == c1 || c == c2 || c == c3 || c == c4;
}

static inline bool
in(Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4, Reader::Char c5)
{
	return c == c1 || c == c2 || c == c3 || c == c4 || c == c5;
}

static bool
containsNewLine(Reader::Location begin,
Reader::Location end)
{
	for (; begin < end; ++begin)
		if (*begin == '\n' || *begin == '\r')
			return true;
	return false;
}

static std::string codePointToUTF8(unsigned int cp)
{
	std::string result;

	// based on description from http://en.wikipedia.org/wiki/UTF-8

	if (cp <= 0x7f)
	{
		result.resize(1);
		result[0] = static_cast<char>(cp);
	}
	else if (cp <= 0x7FF)
	{
		result.resize(2);
		result[1] = static_cast<char>(0x80 | (0x3f & cp));
		result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
	}
	else if (cp <= 0xFFFF)
	{
		result.resize(3);
		result[2] = static_cast<char>(0x80 | (0x3f & cp));
		result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
		result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
	}
	else if (cp <= 0x10FFFF)
	{
		result.resize(4);
		result[3] = static_cast<char>(0x80 | (0x3f & cp));
		result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
		result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
		result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
	}

	return result;
}


// Class Reader
Reader::Reader()
	: features_(Features::all())
{
}

Reader::Reader(const Features &features)
	: features_(features)
{
}

bool
Reader::parse(const std::string &document,
Value &root,
bool collectComments)
{
	document_ = document;
	const char *begin = document_.c_str();
	const char *end = begin + document_.length();
	return parse(begin, end, root, collectComments);
}

bool
Reader::parse(std::istream& sin,
Value &root,
bool collectComments)
{
	std::string doc;
	std::getline(sin, doc, (char)EOF);
	return parse(doc, root, collectComments);
}

bool
Reader::parse(const char *beginDoc, const char *endDoc,
Value &root,
bool collectComments)
{
	if (!features_.allowComments_)
	{
		collectComments = false;
	}

	begin_ = beginDoc;
	end_ = endDoc;
	collectComments_ = collectComments;
	current_ = begin_;
	lastValueEnd_ = 0;
	lastValue_ = 0;
	commentsBefore_ = "";
	errors_.clear();
	while (!nodes_.empty())
		nodes_.pop();
	nodes_.push(&root);

	bool successful = readValue();
	Token token;
	skipCommentTokens(token);
	if (collectComments_  &&  !commentsBefore_.empty())
		root.setComment(commentsBefore_, commentAfter);
	if (features_.strictRoot_)
	{
		if (!root.isArray() && !root.isObject())
		{
			// Set error location to start of doc, ideally should be first token found in doc
			token.type_ = tokenError;
			token.start_ = beginDoc;
			token.end_ = endDoc;
			addError("A valid JSON document must be either an array or an object value.",
				token);
			return false;
		}
	}
	return successful;
}

bool
Reader::readValue()
{
	Token token;
	skipCommentTokens(token);
	bool successful = true;

	if (collectComments_  &&  !commentsBefore_.empty())
	{
		currentValue().setComment(commentsBefore_, commentBefore);
		commentsBefore_ = "";
	}


	switch (token.type_)
	{
	case tokenObjectBegin:
		successful = readObject(token);
		break;
	case tokenArrayBegin:
		successful = readArray(token);
		break;
	case tokenNumber:
		successful = decodeNumber(token);
		break;
	case tokenString:
		successful = decodeString(token);
		break;
	case tokenTrue:
		currentValue() = true;
		break;
	case tokenFalse:
		currentValue() = false;
		break;
	case tokenNull:
		currentValue() = Value();
		break;
	default:
		return addError("Syntax error: value, object or array expected.", token);
	}

	if (collectComments_)
	{
		lastValueEnd_ = current_;
		lastValue_ = &currentValue();
	}

	return successful;
}

void
Reader::skipCommentTokens(Token &token)
{
	if (features_.allowComments_)
	{
		do
		{
			readToken(token);
		} while (token.type_ == tokenComment);
	}
	else
	{
		readToken(token);
	}
}

bool
Reader::expectToken(TokenType type, Token &token, const char *message)
{
	readToken(token);
	if (token.type_ != type)
		return addError(message, token);
	return true;
}

bool
Reader::readToken(Token &token)
{
	skipSpaces();
	token.start_ = current_;
	Char c = getNextChar();
	bool ok = true;
	switch (c)
	{
	case '{':
		token.type_ = tokenObjectBegin;
		break;
	case '}':
		token.type_ = tokenObjectEnd;
		break;
	case '[':
		token.type_ = tokenArrayBegin;
		break;
	case ']':
		token.type_ = tokenArrayEnd;
		break;
	case '\'':
		token.type_ = tokenString;
		ok = readString();
		break;
	case '#':
		token.type_ = tokenComment;
		ok = readComment();
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
		token.type_ = tokenNumber;
		readNumber();
		break;
	case 't':
		token.type_ = tokenTrue;
		ok = match("rue", 3);
		break;
	case 'f':
		token.type_ = tokenFalse;
		ok = match("alse", 4);
		break;
	case 'n':
		token.type_ = tokenNull;
		ok = match("ull", 3);
		break;
	case ',':// ��Ҫ�жϺ���һ���Ƿ������
		skipSpaces();// �����ո�
		switch (*current_)
		{
		case '}':
			token.type_ = tokenObjectEnd;
			token.start_ = current_++;
			break;
		case ']':
			token.type_ = tokenArrayEnd;
			token.start_ = current_++;
			break;
		case '#':// ע���浰�ۡ���
			do
			{
				++current_;
				ok = readComment();
				skipSpaces();
			} while (ok && *current_ == '#');

			switch (*current_)
			{
			case '}':
				token.type_ = tokenObjectEnd;
				token.start_ = current_++;
				break;
			case ']':
				token.type_ = tokenArrayEnd;
				token.start_ = current_++;
				break;
			default:
				token.type_ = tokenArraySeparator;
				break;
			}
			break;
		default:
			token.type_ = tokenArraySeparator;
			break;
		}
		break;
	case ':':
		token.type_ = tokenMemberSeparator;
		break;
	case 0:
		token.type_ = tokenEndOfStream;
		break;
	default:
		ok = false;
		break;
	}
	if (!ok)
		token.type_ = tokenError;
	token.end_ = current_;
	return true;
}

void
Reader::skipSpaces()
{
	while (current_ != end_)
	{
		Char c = *current_;
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			++current_;
		else
			break;
	}
}

bool
Reader::match(Location pattern,
int patternLength)
{
	if (end_ - current_ < patternLength)
		return false;
	int index = patternLength;
	while (index--)
		if (current_[index] != pattern[index])
			return false;
	current_ += patternLength;
	return true;
}

bool
Reader::readComment()
{
	Location commentBegin = current_ - 1;

	while (current_ != end_)
	{
		Char c = getNextChar();
		if (c == '\r' || c == '\n')
			break;
	}

	if (collectComments_)
	{
		CommentPlacement placement = commentBefore;
		if (lastValueEnd_  &&  !containsNewLine(lastValueEnd_, commentBegin))
		{
			if (!containsNewLine(commentBegin, current_))
				placement = commentAfterOnSameLine;
		}

		addComment(commentBegin, current_, placement);
	}
	return true;
}

void
Reader::addComment(Location begin,
Location end,
CommentPlacement placement)
{
	assert(collectComments_);
	if (placement == commentAfterOnSameLine)
	{
		assert(lastValue_ != 0);
		lastValue_->setComment(std::string(begin, end), placement);
	}
	else
	{
		commentsBefore_ += std::string(begin, end);
	}
}

void
Reader::readNumber()
{
	while (current_ != end_)
	{
		if (!(*current_ >= '0'  &&  *current_ <= '9') &&
			!in(*current_, '.', 'e', 'E', '+', '-'))
			break;
		++current_;
	}
}

bool
Reader::readString()
{
	Char c = 0;
	while (current_ != end_)
	{
		c = getNextChar();
		if (c == '\\')
			getNextChar();
		else if (c == '\'')
			break;
	}
	return c == '\'';
}

bool
Reader::readObject(Token &tokenStart)
{
	Token tokenName;
	std::string name;
	currentValue() = Value(objectValue);
	while (readToken(tokenName))
	{
		bool initialTokenOk = true;
		while (tokenName.type_ == tokenComment  &&  initialTokenOk)
			initialTokenOk = readToken(tokenName);
		if (!initialTokenOk)
			break;
		if (tokenName.type_ == tokenObjectEnd/*  &&  name.empty()*/)  // empty object
			return true;
		if (tokenName.type_ != tokenString)
			break;

		name = "";
		if (!decodeString(tokenName, name))
			return recoverFromError(tokenObjectEnd);

		Token colon;
		if (!readToken(colon) || colon.type_ != tokenMemberSeparator)
		{
			return addErrorAndRecover("Missing ':' after object member name",
				colon,
				tokenObjectEnd);
		}
		Value &value = currentValue()[name];
		nodes_.push(&value);
		bool ok = readValue();
		nodes_.pop();
		if (!ok) // error already set
			return recoverFromError(tokenObjectEnd);

		Token comma;
		if (!readToken(comma)
			|| (comma.type_ != tokenObjectEnd  &&
			comma.type_ != tokenArraySeparator &&
			comma.type_ != tokenComment))
		{
			return addErrorAndRecover("Missing ',' or '}' in object declaration",
				comma,
				tokenObjectEnd);
		}
		bool finalizeTokenOk = true;
		while (comma.type_ == tokenComment &&
			finalizeTokenOk)
			finalizeTokenOk = readToken(comma);
		if (comma.type_ == tokenObjectEnd)
			return true;
	}
	return addErrorAndRecover("Missing '}' or object member name",
		tokenName,
		tokenObjectEnd);
}

bool
Reader::readArray(Token &tokenStart)
{
	currentValue() = Value(arrayValue);
	skipSpaces();
	if (*current_ == ']') // empty array
	{
		Token endArray;
		readToken(endArray);
		return true;
	}
	int index = 0;
	while (true)
	{
		Value &value = currentValue()[index++];
		nodes_.push(&value);
		bool ok = readValue();
		nodes_.pop();
		if (!ok) // error already set
			return recoverFromError(tokenArrayEnd);

		Token token;
		// Accept Comment after last item in the array.
		ok = readToken(token);
		while (token.type_ == tokenComment  &&  ok)
		{
			ok = readToken(token);
		}
		bool badTokenType = (token.type_ == tokenArraySeparator  &&
			token.type_ == tokenArrayEnd);
		if (!ok || badTokenType)
		{
			return addErrorAndRecover("Missing ',' or ']' in array declaration",
				token,
				tokenArrayEnd);
		}
		if (token.type_ == tokenArrayEnd)
			break;
	}
	return true;
}

bool
Reader::decodeNumber(Token &token)
{
	bool isDouble = false;
	for (Location inspect = token.start_; inspect != token.end_; ++inspect)
	{
		isDouble = isDouble
			|| in(*inspect, '.', 'e', 'E', '+')
			|| (*inspect == '-'  &&  inspect != token.start_);
	}
	if (isDouble)
		return decodeDouble(token);
	Location current = token.start_;
	bool isNegative = *current == '-';
	if (isNegative)
		++current;
	Value::UInt threshold = (isNegative ? Value::UInt(-Value::minInt)
		: Value::maxUInt) / 10;
	Value::UInt value = 0;
	while (current < token.end_)
	{
		Char c = *current++;
		if (c < '0' || c > '9')
			return addError("'" + std::string(token.start_, token.end_) + "' is not a number.", token);
		if (value >= threshold)
			return decodeDouble(token);
		value = value * 10 + Value::UInt(c - '0');
	}
	if (isNegative)
		currentValue() = -Value::Int(value);
	else if (value <= Value::UInt(Value::maxInt))
		currentValue() = Value::Int(value);
	else
		currentValue() = value;
	return true;
}

bool
Reader::decodeDouble(Token &token)
{
	double value = 0;
	const int bufferSize = 32;
	int count;
	int length = int(token.end_ - token.start_);
	if (length <= bufferSize)
	{
		Char buffer[bufferSize];
		memcpy(buffer, token.start_, length);
		buffer[length] = 0;
		count = sscanf(buffer, "%lf", &value);
	}
	else
	{
		std::string buffer(token.start_, token.end_);
		count = sscanf(buffer.c_str(), "%lf", &value);
	}

	if (count != 1)
		return addError("'" + std::string(token.start_, token.end_) + "' is not a number.", token);
	currentValue() = value;
	return true;
}

bool
Reader::decodeString(Token &token)
{
	std::string decoded;
	if (!decodeString(token, decoded))
		return false;
	currentValue() = decoded;
	return true;
}

bool
Reader::decodeString(Token &token, std::string &decoded)
{
	decoded.reserve(token.end_ - token.start_ - 2);
	Location current = token.start_ + 1; // skip '"'
	Location end = token.end_ - 1;      // do not include '"'
	while (current != end)
	{
		Char c = *current++;
		if (c == '"')
			break;
		else if (c == '\\')
		{
			if (current == end)
				return addError("Empty escape sequence in string", token, current);
			Char escape = *current++;
			switch (escape)
			{
			case '"': decoded += '"'; break;
			case '/': decoded += '/'; break;
			case '\\': decoded += '\\'; break;
			case 'b': decoded += '\b'; break;
			case 'f': decoded += '\f'; break;
			case 'n': decoded += '\n'; break;
			case 'r': decoded += '\r'; break;
			case 't': decoded += '\t'; break;
			case 'u':
			{
				unsigned int unicode;
				if (!decodeUnicodeCodePoint(token, current, end, unicode))
					return false;
				decoded += codePointToUTF8(unicode);
			}
				break;
			default:
				return addError("Bad escape sequence in string", token, current);
			}
		}
		else
		{
			decoded += c;
		}
	}
	return true;
}

bool
Reader::decodeUnicodeCodePoint(Token &token,
Location &current,
Location end,
unsigned int &unicode)
{

	if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
		return false;
	if (unicode >= 0xD800 && unicode <= 0xDBFF)
	{
		// surrogate pairs
		if (end - current < 6)
			return addError("additional six characters expected to parse unicode surrogate pair.", token, current);
		unsigned int surrogatePair;
		if (*(current++) == '\\' && *(current++) == 'u')
		{
			if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair))
			{
				unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
			}
			else
				return false;
		}
		else
			return addError("expecting another \\u token to begin the second half of a unicode surrogate pair", token, current);
	}
	return true;
}

bool
Reader::decodeUnicodeEscapeSequence(Token &token,
Location &current,
Location end,
unsigned int &unicode)
{
	if (end - current < 4)
		return addError("Bad unicode escape sequence in string: four digits expected.", token, current);
	unicode = 0;
	for (int index = 0; index < 4; ++index)
	{
		Char c = *current++;
		unicode *= 16;
		if (c >= '0'  &&  c <= '9')
			unicode += c - '0';
		else if (c >= 'a'  &&  c <= 'f')
			unicode += c - 'a' + 10;
		else if (c >= 'A'  &&  c <= 'F')
			unicode += c - 'A' + 10;
		else
			return addError("Bad unicode escape sequence in string: hexadecimal digit expected.", token, current);
	}
	return true;
}

bool
Reader::addError(const std::string &message,
Token &token,
Location extra)
{
	ErrorInfo info;
	info.token_ = token;
	info.message_ = message;
	info.extra_ = extra;
	errors_.push_back(info);
	return false;
}

bool
Reader::recoverFromError(TokenType skipUntilToken)
{
	int errorCount = int(errors_.size());
	Token skip;
	while (true)
	{
		if (!readToken(skip))
			errors_.resize(errorCount); // discard errors caused by recovery
		if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
			break;
	}
	errors_.resize(errorCount);
	return false;
}

bool
Reader::addErrorAndRecover(const std::string &message,
Token &token,
TokenType skipUntilToken)
{
	addError(message, token);
	return recoverFromError(skipUntilToken);
}


Value &
Reader::currentValue()
{
	return *(nodes_.top());
}

Reader::Char
Reader::getNextChar()
{
	if (current_ == end_)
		return 0;
	return *current_++;
}

void
Reader::getLocationLineAndColumn(Location location,
int &line,
int &column) const
{
	Location current = begin_;
	Location lastLineStart = current;
	line = 0;
	while (current < location  &&  current != end_)
	{
		Char c = *current++;
		if (c == '\r')
		{
			if (*current == '\n')
				++current;
			lastLineStart = current;
			++line;
		}
		else if (c == '\n')
		{
			lastLineStart = current;
			++line;
		}
	}
	// column & line start at 1
	column = int(location - lastLineStart) + 1;
	++line;
}

std::string
Reader::getLocationLineAndColumn(Location location) const
{
	int line, column;
	getLocationLineAndColumn(location, line, column);
	char buffer[18 + 16 + 16 + 1];
	sprintf(buffer, "Line %d, Column %d", line, column);
	return buffer;
}

std::string
Reader::getFormatedErrorMessages() const
{
	std::string formattedMessage;
	for (Errors::const_iterator itError = errors_.begin();
		itError != errors_.end();
		++itError)
	{
		const ErrorInfo &error = *itError;
		formattedMessage += "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
		formattedMessage += "  " + error.message_ + "\n";
		if (error.extra_)
			formattedMessage += "See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
	}
	return formattedMessage;
}

std::istream& operator>>(std::istream &sin, Value &root)
{
	gyp::Reader reader;
	bool ok = reader.parse(sin, root, true);
	//JSON_ASSERT( ok );
	if (!ok) throw std::runtime_error(reader.getFormatedErrorMessages());
	return sin;
}


static bool isControlCharacter(char ch)
{
	return ch > 0 && ch <= 0x1F;
}

static bool containsControlCharacter(const char* str)
{
	while (*str)
	{
		if (isControlCharacter(*(str++)))
			return true;
	}
	return false;
}
static void uintToString(unsigned int value,
	char *&current)
{
	*--current = 0;
	do
	{
		*--current = (value % 10) + '0';
		value /= 10;
	} while (value != 0);
}

std::string valueToString(Int value)
{
	char buffer[32];
	char *current = buffer + sizeof(buffer);
	bool isNegative = value < 0;
	if (isNegative)
		value = -value;
	uintToString(UInt(value), current);
	if (isNegative)
		*--current = '-';
	assert(current >= buffer);
	return current;
}

std::string valueToString(UInt value)
{
	char buffer[32];
	char *current = buffer + sizeof(buffer);
	uintToString(value, current);
	assert(current >= buffer);
	return current;
}

std::string valueToString(double value)
{
	char buffer[32];
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__) // Use secure version with visual studio 2005 to avoid warning. 
	sprintf_s(buffer, sizeof(buffer), "%#.16g", value);
#else	
	sprintf(buffer, "%#.16g", value);
#endif
	char* ch = buffer + strlen(buffer) - 1;
	if (*ch != '0') return buffer; // nothing to truncate, so save time
	while (ch > buffer && *ch == '0'){
		--ch;
	}
	char* last_nonzero = ch;
	while (ch >= buffer){
		switch (*ch){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			--ch;
			continue;
		case '.':
			// Truncate zeroes to save bytes in output, but keep one.
			*(last_nonzero + 2) = '\0';
			return buffer;
		default:
			return buffer;
		}
	}
	return buffer;
}

std::string valueToString(bool value)
{
	return value ? "true" : "false";
}

std::string valueToQuotedString(const char *value)
{
	// Not sure how to handle unicode...
	if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter(value))
		return std::string("'") + value + "'";
	// We have to walk value and escape any special characters.
	// Appending to std::string is not efficient, but this should be rare.
	// (Note: forward slashes are *not* rare, but I am not escaping them.)
	unsigned maxsize = strlen(value) * 2 + 3; // allescaped+quotes+NULL
	std::string result;
	result.reserve(maxsize); // to avoid lots of mallocs
	result += "'";
	for (const char* c = value; *c != 0; ++c)
	{
		switch (*c)
		{
		case '"':
			result += "\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		case '\b':
			result += "\\b";
			break;
		case '\f':
			result += "\\f";
			break;
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		case '\t':
			result += "\\t";
			break;
			//case '/':
			// Even though \/ is considered a legal escape in JSON, a bare
			// slash is also legal, so I see no reason to escape it.
			// (I hope I am not misunderstanding something.
			// blep notes: actually escaping \/ may be useful in javascript to avoid </ 
			// sequence.
			// Should add a flag to allow this compatibility mode and prevent this 
			// sequence from occurring.
		default:
			if (isControlCharacter(*c))
			{
				std::ostringstream oss;
				oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(*c);
				result += oss.str();
			}
			else
			{
				result += *c;
			}
			break;
		}
	}
	result += "'";
	return result;
}


// Class Writer
Writer::~Writer()
{
}


// Class FastWriter
FastWriter::FastWriter()
	: yamlCompatiblityEnabled_(false)
{
}

void
FastWriter::enableYAMLCompatibility()
{
	yamlCompatiblityEnabled_ = true;
}

std::string
FastWriter::write(const Value &root)
{
	document_ = "";
	writeValue(root);
	document_ += "\n";
	return document_;
}

void
FastWriter::writeValue(const Value &value)
{
	switch (value.type())
	{
	case nullValue:
		document_ += "null";
		break;
	case intValue:
		document_ += valueToString(value.asInt());
		break;
	case uintValue:
		document_ += valueToString(value.asUInt());
		break;
	case realValue:
		document_ += valueToString(value.asDouble());
		break;
	case stringValue:
		document_ += valueToQuotedString(value.asCString());
		break;
	case booleanValue:
		document_ += valueToString(value.asBool());
		break;
	case arrayValue:
	{
		document_ += "[";
		int size = value.size();
		for (int index = 0; index < size; ++index)
		{
			writeValue(value[index]);
			document_ += ",";
		}
		document_ += "]";
	}
		break;
	case objectValue:
	{
		Value::Members members(value.getMemberNames());
		document_ += "{";
		for (Value::Members::iterator it = members.begin();
			it != members.end();
			++it)
		{
			const std::string &name = *it;
			document_ += valueToQuotedString(name.c_str());
			document_ += yamlCompatiblityEnabled_ ? ": " : ":";
			writeValue(value[name]);
			document_ += ",";
		}
		document_ += "}";
	}
		break;
	}
}


// Class StyledWriter
StyledWriter::StyledWriter()
	: rightMargin_(74)
	, indentSize_(3)
{
}

std::string
StyledWriter::write(const Value &root)
{
	document_ = "";
	addChildValues_ = false;
	indentString_ = "";
	writeCommentBeforeValue(root);
	writeValue(root);
	writeCommentAfterValueOnSameLine(root);
	document_ += "\n";
	return document_;
}

void
StyledWriter::writeValue(const Value &value)
{
	switch (value.type())
	{
	case nullValue:
		pushValue("null");
		break;
	case intValue:
		pushValue(valueToString(value.asInt()));
		break;
	case uintValue:
		pushValue(valueToString(value.asUInt()));
		break;
	case realValue:
		pushValue(valueToString(value.asDouble()));
		break;
	case stringValue:
		pushValue(valueToQuotedString(value.asCString()));
		break;
	case booleanValue:
		pushValue(valueToString(value.asBool()));
		break;
	case arrayValue:
		writeArrayValue(value);
		break;
	case objectValue:
	{
		Value::Members members(value.getMemberNames());
		if (members.empty())
			pushValue("{}");
		else
		{
			writeWithIndent("{");
			indent();
			for (Value::Members::iterator it = members.begin(); it != members.end(); ++it)
			{
				const std::string &name = *it;
				const Value &childValue = value[name];
				writeCommentBeforeValue(childValue);
				writeWithIndent(valueToQuotedString(name.c_str()));
				document_ += " : ";
				writeValue(childValue);
				document_ += ",";
				writeCommentAfterValueOnSameLine(childValue);
			}
			unindent();
			writeWithIndent("}");
		}
	}
		break;
	}
}

void
StyledWriter::writeArrayValue(const Value &value)
{
	unsigned size = value.size();
	if (size == 0)
		pushValue("[]");
	else
	{
		bool isArrayMultiLine = isMultineArray(value);
		if (isArrayMultiLine)
		{
			writeWithIndent("[");
			indent();
			bool hasChildValue = !childValues_.empty();
			for (unsigned index = 0; index < size; ++index)
			{
				const Value &childValue = value[index];
				writeCommentBeforeValue(childValue);
				if (hasChildValue)
					writeWithIndent(childValues_[index]);
				else
				{
					writeIndent();
					writeValue(childValue);
				}
				document_ += ",\n";
				writeCommentAfterValueOnSameLine(childValue);
			}
			unindent();
			writeWithIndent("]");
		}
		else // output on a single line
		{
			assert(childValues_.size() == size);
			document_ += "[ ";
			for (unsigned index = 0; index < size; ++index)
			{
				document_ += childValues_[index];
				document_ += ", ";
			}
			document_ += " ]";
		}
	}
}

bool
StyledWriter::isMultineArray(const Value &value)
{
	int size = value.size();
	bool isMultiLine = size >= 2;
	childValues_.clear();
	for (int index = 0; index < size && !isMultiLine; ++index)
	{
		const Value &childValue = value[index];
		isMultiLine = isMultiLine ||
			((childValue.isArray() || childValue.isObject()) &&
			childValue.size() > 0);
	}
	if (!isMultiLine) // check if line length > max line length
	{
		childValues_.reserve(size);
		addChildValues_ = true;
		int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
		for (int index = 0; index < size && !isMultiLine; ++index)
		{
			writeValue(value[index]);
			lineLength += int(childValues_[index].length());
			isMultiLine = isMultiLine  &&  hasCommentForValue(value[index]);
		}
		addChildValues_ = false;
		isMultiLine = isMultiLine || lineLength >= rightMargin_;
	}
	return isMultiLine;
}

void
StyledWriter::pushValue(const std::string &value)
{
	if (addChildValues_)
		childValues_.push_back(value);
	else
		document_ += value;
}

void
StyledWriter::writeIndent()
{
	if (!document_.empty())
	{
		char last = document_[document_.length() - 1];
		if (last == ' ')     // already indented
			return;
		if (last != '\n')    // Comments may add new-line
			document_ += '\n';
	}
	document_ += indentString_;
}

void
StyledWriter::writeWithIndent(const std::string &value)
{
	writeIndent();
	document_ += value;
}

void
StyledWriter::indent()
{
	indentString_ += std::string(indentSize_, ' ');
}

void
StyledWriter::unindent()
{
	assert(int(indentString_.size()) >= indentSize_);
	indentString_.resize(indentString_.size() - indentSize_);
}

void
StyledWriter::writeCommentBeforeValue(const Value &root)
{
	if (!root.hasComment(commentBefore))
		return;
	writeIndent();
	document_ += normalizeEOL(root.getComment(commentBefore));
}

void
StyledWriter::writeCommentAfterValueOnSameLine(const Value &root)
{
	if (root.hasComment(commentAfterOnSameLine))
		document_ += " " + normalizeEOL(root.getComment(commentAfterOnSameLine));

	if (root.hasComment(commentAfter))
	{
		document_ += "\n";
		document_ += normalizeEOL(root.getComment(commentAfter));
		document_ += "\n";
	}
}

bool
StyledWriter::hasCommentForValue(const Value &value)
{
	return value.hasComment(commentBefore)
		|| value.hasComment(commentAfterOnSameLine)
		|| value.hasComment(commentAfter);
}

std::string
StyledWriter::normalizeEOL(const std::string &text)
{
	std::string normalized;
	normalized.reserve(text.length());
	const char *begin = text.c_str();
	const char *end = begin + text.length();
	const char *current = begin;
	while (current != end)
	{
		char c = *current++;
		if (c == '\r') // mac or dos EOL
		{
			if (*current == '\n') // convert dos EOL
				++current;
			normalized += '\n';
		}
		else // handle unix EOL & other char
			normalized += c;
	}
	return normalized;
}


// Class StyledStreamWriter
StyledStreamWriter::StyledStreamWriter(std::string indentation)
	: document_(NULL)
	, rightMargin_(74)
	, indentation_(indentation)
{
}

void
StyledStreamWriter::write(std::ostream &out, const Value &root)
{
	document_ = &out;
	addChildValues_ = false;
	indentString_ = "";
	writeCommentBeforeValue(root);
	writeValue(root);
	writeCommentAfterValueOnSameLine(root);
	*document_ << "\n";
	document_ = NULL; // Forget the stream, for safety.
}

void
StyledStreamWriter::writeValue(const Value &value)
{
	switch (value.type())
	{
	case nullValue:
		pushValue("null");
		break;
	case intValue:
		pushValue(valueToString(value.asInt()));
		break;
	case uintValue:
		pushValue(valueToString(value.asUInt()));
		break;
	case realValue:
		pushValue(valueToString(value.asDouble()));
		break;
	case stringValue:
		pushValue(valueToQuotedString(value.asCString()));
		break;
	case booleanValue:
		pushValue(valueToString(value.asBool()));
		break;
	case arrayValue:
		writeArrayValue(value);
		break;
	case objectValue:
	{
		Value::Members members(value.getMemberNames());
		if (members.empty())
			pushValue("{}");
		else
		{
			writeWithIndent("{");
			indent();
			Value::Members::iterator it = members.begin();
			while (true)
			{
				const std::string &name = *it;
				const Value &childValue = value[name];
				writeCommentBeforeValue(childValue);
				writeWithIndent(valueToQuotedString(name.c_str()));
				*document_ << " : ";
				writeValue(childValue);
				if (++it == members.end())
				{
					writeCommentAfterValueOnSameLine(childValue);
					break;
				}
				*document_ << ",";
				writeCommentAfterValueOnSameLine(childValue);
			}
			unindent();
			writeWithIndent("}");
		}
	}
		break;
	}
}

void
StyledStreamWriter::writeArrayValue(const Value &value)
{
	unsigned size = value.size();
	if (size == 0)
		pushValue("[]");
	else
	{
		bool isArrayMultiLine = isMultineArray(value);
		if (isArrayMultiLine)
		{
			writeWithIndent("[");
			indent();
			bool hasChildValue = !childValues_.empty();
			unsigned index = 0;
			while (true)
			{
				const Value &childValue = value[index];
				writeCommentBeforeValue(childValue);
				if (hasChildValue)
					writeWithIndent(childValues_[index]);
				else
				{
					writeIndent();
					writeValue(childValue);
				}
				if (++index == size)
				{
					writeCommentAfterValueOnSameLine(childValue);
					break;
				}
				*document_ << ",";
				writeCommentAfterValueOnSameLine(childValue);
			}
			unindent();
			writeWithIndent("]");
		}
		else // output on a single line
		{
			assert(childValues_.size() == size);
			*document_ << "[ ";
			for (unsigned index = 0; index < size; ++index)
			{
				if (index > 0)
					*document_ << ", ";
				*document_ << childValues_[index];
			}
			*document_ << " ]";
		}
	}
}

bool
StyledStreamWriter::isMultineArray(const Value &value)
{
	int size = value.size();
	bool isMultiLine = size * 3 >= rightMargin_;
	childValues_.clear();
	for (int index = 0; index < size && !isMultiLine; ++index)
	{
		const Value &childValue = value[index];
		isMultiLine = isMultiLine ||
			((childValue.isArray() || childValue.isObject()) &&
			childValue.size() > 0);
	}
	if (!isMultiLine) // check if line length > max line length
	{
		childValues_.reserve(size);
		addChildValues_ = true;
		int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
		for (int index = 0; index < size && !isMultiLine; ++index)
		{
			writeValue(value[index]);
			lineLength += int(childValues_[index].length());
			isMultiLine = isMultiLine  &&  hasCommentForValue(value[index]);
		}
		addChildValues_ = false;
		isMultiLine = isMultiLine || lineLength >= rightMargin_;
	}
	return isMultiLine;
}

void
StyledStreamWriter::pushValue(const std::string &value)
{
	if (addChildValues_)
		childValues_.push_back(value);
	else
		*document_ << value;
}

void
StyledStreamWriter::writeIndent()
{
	*document_ << '\n' << indentString_;
}

void
StyledStreamWriter::writeWithIndent(const std::string &value)
{
	writeIndent();
	*document_ << value;
}

void
StyledStreamWriter::indent()
{
	indentString_ += indentation_;
}

void
StyledStreamWriter::unindent()
{
	assert(indentString_.size() >= indentation_.size());
	indentString_.resize(indentString_.size() - indentation_.size());
}

void
StyledStreamWriter::writeCommentBeforeValue(const Value &root)
{
	if (!root.hasComment(commentBefore))
		return;
	*document_ << normalizeEOL(root.getComment(commentBefore));
	*document_ << "\n";
}

void
StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value &root)
{
	if (root.hasComment(commentAfterOnSameLine))
		*document_ << " " + normalizeEOL(root.getComment(commentAfterOnSameLine));

	if (root.hasComment(commentAfter))
	{
		*document_ << "\n";
		*document_ << normalizeEOL(root.getComment(commentAfter));
		*document_ << "\n";
	}
}

bool
StyledStreamWriter::hasCommentForValue(const Value &value)
{
	return value.hasComment(commentBefore)
		|| value.hasComment(commentAfterOnSameLine)
		|| value.hasComment(commentAfter);
}

std::string
StyledStreamWriter::normalizeEOL(const std::string &text)
{
	std::string normalized;
	normalized.reserve(text.length());
	const char *begin = text.c_str();
	const char *end = begin + text.length();
	const char *current = begin;
	while (current != end)
	{
		char c = *current++;
		if (c == '\r') // mac or dos EOL
		{
			if (*current == '\n') // convert dos EOL
				++current;
			normalized += '\n';
		}
		else // handle unix EOL & other char
			normalized += c;
	}
	return normalized;
}

std::ostream& operator<<(std::ostream &sout, const Value &root)
{
	gyp::StyledStreamWriter writer;
	writer.write(sout, root);
	return sout;
}


}// namespace gyp
