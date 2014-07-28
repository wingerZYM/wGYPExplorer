#pragma once

#include <map>
#include <vector>
#include <deque>
#include <stack>
#include <string>
#include <iostream>

namespace gyp {

	// writer.h
	class FastWriter;
	class StyledWriter;

	// reader.h
	class Reader;

	// features.h
	class Features;

	// value.h
	typedef int Int;
	typedef unsigned int UInt;
	class StaticString;
	class Path;
	class PathArgument;
	class Value;
	class ValueIteratorBase;
	class ValueIterator;
	class ValueConstIterator;


	enum ValueType
	{
		nullValue = 0, ///< 'null' value
		intValue,      ///< signed integer value
		uintValue,     ///< unsigned integer value
		realValue,     ///< double value
		stringValue,   ///< UTF-8 string value
		booleanValue,  ///< bool value
		arrayValue,    ///< array value (ordered list)
		objectValue    ///< object value (collection of name/value pairs).
	};

	enum CommentPlacement
	{
		commentBefore = 0,        ///< a comment placed on the line before a value
		commentAfterOnSameLine,   ///< a comment just after a value on the same line
		commentAfter,             ///< a comment on the line after a value (only make sense for root value)
		numberOfCommentPlacement
	};

	template<typename AllocatedType
		, const unsigned int objectPerAllocation>
	class BatchAllocator
	{
	public:
		typedef AllocatedType Type;

		BatchAllocator(unsigned int objectsPerPage = 255)
			: freeHead_(0)
			, objectsPerPage_(objectsPerPage)
		{
			//      printf( "Size: %d => %s\n", sizeof(AllocatedType), typeid(AllocatedType).name() );
			assert(sizeof(AllocatedType) * objectPerAllocation >= sizeof(AllocatedType *)); // We must be able to store a slist in the object free space.
			assert(objectsPerPage >= 16);
			batches_ = allocateBatch(0);   // allocated a dummy page
			currentBatch_ = batches_;
		}

		~BatchAllocator()
		{
			for (BatchInfo *batch = batches_; batch;)
			{
				BatchInfo *nextBatch = batch->next_;
				free(batch);
				batch = nextBatch;
			}
		}

		/// allocate space for an array of objectPerAllocation object.
		/// @warning it is the responsability of the caller to call objects constructors.
		AllocatedType *allocate()
		{
			if (freeHead_) // returns node from free list.
			{
				AllocatedType *object = freeHead_;
				freeHead_ = *(AllocatedType **)object;
				return object;
			}
			if (currentBatch_->used_ == currentBatch_->end_)
			{
				currentBatch_ = currentBatch_->next_;
				while (currentBatch_  &&  currentBatch_->used_ == currentBatch_->end_)
					currentBatch_ = currentBatch_->next_;

				if (!currentBatch_) // no free batch found, allocate a new one
				{
					currentBatch_ = allocateBatch(objectsPerPage_);
					currentBatch_->next_ = batches_; // insert at the head of the list
					batches_ = currentBatch_;
				}
			}
			AllocatedType *allocated = currentBatch_->used_;
			currentBatch_->used_ += objectPerAllocation;
			return allocated;
		}

		/// Release the object.
		/// @warning it is the responsability of the caller to actually destruct the object.
		void release(AllocatedType *object)
		{
			assert(object != 0);
			*(AllocatedType **)object = freeHead_;
			freeHead_ = object;
		}

	private:
		struct BatchInfo
		{
			BatchInfo *next_;
			AllocatedType *used_;
			AllocatedType *end_;
			AllocatedType buffer_[objectPerAllocation];
		};

		// disabled copy constructor and assignement operator.
		BatchAllocator(const BatchAllocator &);
		void operator =(const BatchAllocator &);

		static BatchInfo *allocateBatch(unsigned int objectsPerPage)
		{
			const unsigned int mallocSize = sizeof(BatchInfo) - sizeof(AllocatedType)* objectPerAllocation
				+ sizeof(AllocatedType) * objectPerAllocation * objectsPerPage;
			BatchInfo *batch = static_cast<BatchInfo*>(malloc(mallocSize));
			batch->next_ = 0;
			batch->used_ = batch->buffer_;
			batch->end_ = batch->buffer_ + objectsPerPage;
			return batch;
		}

		BatchInfo *batches_;
		BatchInfo *currentBatch_;
		/// Head of a single linked list within the allocated space of freeed object
		AllocatedType *freeHead_;
		unsigned int objectsPerPage_;
	};

	class StaticString
	{
	public:
		explicit StaticString(const char *czstring)
			: str_(czstring)
		{
		}

		operator const char *() const
		{
			return str_;
		}

		const char *c_str() const
		{
			return str_;
		}

	private:
		const char *str_;
	};

	class Value
	{
		friend class ValueIteratorBase;
	public:
		typedef std::vector<std::string> Members;
		typedef ValueIterator iterator;
		typedef ValueConstIterator const_iterator;
		typedef gyp::UInt UInt;
		typedef gyp::Int Int;
		typedef UInt ArrayIndex;

		static const Value null;
		static const Int minInt;
		static const Int maxInt;
		static const UInt maxUInt;

	private:
		class CZString
		{
		public:
			enum DuplicationPolicy
			{
				noDuplication = 0,
				duplicate,
				duplicateOnCopy
			};
			CZString(int index);
			CZString(const char *cstr, DuplicationPolicy allocate);
			CZString(const CZString &other);
			~CZString();
			CZString &operator =(const CZString &other);
			bool operator<(const CZString &other) const;
			bool operator==(const CZString &other) const;
			int index() const;
			const char *c_str() const;
			bool isStaticString() const;
		private:
			void swap(CZString &other);
			const char *cstr_;
			int index_;
		};

	public:
		typedef std::map<CZString, Value> ObjectValues;

		Value(ValueType type = nullValue);
		Value(Int value);
		Value(UInt value);
		Value(double value);
		Value(const char *value);
		Value(const char *beginValue, const char *endValue);
		Value(const StaticString &value);
		Value(const std::string &value);
		Value(bool value);
		Value(const Value &other);
		~Value();

		Value &operator=(const Value &other);
		/// Swap values.
		/// \note Currently, comments are intentionally not swapped, for
		/// both logic and efficiency.
		void swap(Value &other);

		ValueType type() const;

		bool operator <(const Value &other) const;
		bool operator <=(const Value &other) const;
		bool operator >=(const Value &other) const;
		bool operator >(const Value &other) const;

		bool operator ==(const Value &other) const;
		bool operator !=(const Value &other) const;

		int compare(const Value &other);

		const char *asCString() const;
		std::string asString() const;
		Int asInt() const;
		UInt asUInt() const;
		double asDouble() const;
		bool asBool() const;

		bool isNull() const;
		bool isBool() const;
		bool isInt() const;
		bool isUInt() const;
		bool isIntegral() const;
		bool isDouble() const;
		bool isNumeric() const;
		bool isString() const;
		bool isArray() const;
		bool isObject() const;

		bool isConvertibleTo(ValueType other) const;

		/// Number of values in array or object
		UInt size() const;

		/// \brief Return true if empty array, empty object, or null;
		/// otherwise, false.
		bool empty() const;

		/// Return isNull()
		bool operator!() const;

		/// Remove all object members and array elements.
		/// \pre type() is arrayValue, objectValue, or nullValue
		/// \post type() is unchanged
		void clear();

		/// Resize the array to size elements. 
		/// New elements are initialized to null.
		/// May only be called on nullValue or arrayValue.
		/// \pre type() is arrayValue or nullValue
		/// \post type() is arrayValue
		void resize(UInt size);

		/// Access an array element (zero based index ).
		/// If the array contains less than index element, then null value are inserted
		/// in the array so that its size is index+1.
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		Value &operator[](UInt index);
		/// Access an array element (zero based index )
		/// (You may need to say 'value[0u]' to get your compiler to distinguish
		///  this from the operator[] which takes a string.)
		const Value &operator[](UInt index) const;
		/// If the array contains at least index+1 elements, returns the element value, 
		/// otherwise returns defaultValue.
		Value get(UInt index,
			const Value &defaultValue) const;
		/// Return true if index < size().
		bool isValidIndex(UInt index) const;
		/// \brief Append value to array at the end.
		///
		/// Equivalent to jsonvalue[jsonvalue.size()] = value;
		Value &append(const Value &value);

		/// Access an object value by name, create a null member if it does not exist.
		Value &operator[](const char *key);
		/// Access an object value by name, returns null if there is no member with that name.
		const Value &operator[](const char *key) const;
		/// Access an object value by name, create a null member if it does not exist.
		Value &operator[](const std::string &key);
		/// Access an object value by name, returns null if there is no member with that name.
		const Value &operator[](const std::string &key) const;
		/** \brief Access an object value by name, create a null member if it does not exist.

		* If the object as no entry for that name, then the member name used to store
		* the new entry is not duplicated.
		* Example of use:
		* \code
		* gyp::Value object;
		* static const StaticString code("code");
		* object[code] = 1234;
		* \endcode
		*/
		Value &operator[](const StaticString &key);

		/// Return the member named key if it exist, defaultValue otherwise.
		Value get(const char *key,
			const Value &defaultValue) const;
		/// Return the member named key if it exist, defaultValue otherwise.
		Value get(const std::string &key,
			const Value &defaultValue) const;

		/// \brief Remove and return the named member.  
		///
		/// Do nothing if it did not exist.
		/// \return the removed Value, or null.
		/// \pre type() is objectValue or nullValue
		/// \post type() is unchanged
		Value removeMember(const char* key);
		/// Same as removeMember(const char*)
		Value removeMember(const std::string &key);

		/// Return true if the object has a member named key.
		bool isMember(const char *key) const;
		/// Return true if the object has a member named key.
		bool isMember(const std::string &key) const;

		/// \brief Return a list of the member names.
		///
		/// If null, return an empty list.
		/// \pre type() is objectValue or nullValue
		/// \post if type() was nullValue, it remains nullValue
		Members getMemberNames() const;

		/// Comments must be //... or /* ... */
		void setComment(const char *comment,
			CommentPlacement placement);
		/// Comments must be //... or /* ... */
		void setComment(const std::string &comment,
			CommentPlacement placement);
		bool hasComment(CommentPlacement placement) const;
		/// Include delimiters and embedded newlines.
		std::string getComment(CommentPlacement placement) const;

		std::string toStyledString() const;

		const_iterator begin() const;
		const_iterator end() const;

		iterator begin();
		iterator end();

	private:
		Value &resolveReference(const char *key,
			bool isStatic);

		struct CommentInfo
		{
			CommentInfo();
			~CommentInfo();

			void setComment(const char *text);

			char *comment_;
		};


		union ValueHolder
		{
			Int int_;
			UInt uint_;
			double real_;
			bool bool_;
			char *string_;
			ObjectValues *map_;

		} value_;
		ValueType type_ : 8;
		int allocated_ : 1;     // Notes: if declared as bool, bitfield is useless.

		CommentInfo *comments_;
	};

	class PathArgument
	{
	public:
		friend class Path;

		PathArgument();
		PathArgument(UInt index);
		PathArgument(const char *key);
		PathArgument(const std::string &key);

	private:
		enum Kind
		{
			kindNone = 0,
			kindIndex,
			kindKey
		};
		std::string key_;
		UInt index_;
		Kind kind_;
	};

	class Path
	{
	public:
		Path(const std::string &path,
			const PathArgument &a1 = PathArgument(),
			const PathArgument &a2 = PathArgument(),
			const PathArgument &a3 = PathArgument(),
			const PathArgument &a4 = PathArgument(),
			const PathArgument &a5 = PathArgument());

		const Value &resolve(const Value &root) const;
		Value resolve(const Value &root,
			const Value &defaultValue) const;
		/// Creates the "path" to access the specified node and returns a reference on the node.
		Value &make(Value &root) const;

	private:
		typedef std::vector<const PathArgument *> InArgs;
		typedef std::vector<PathArgument> Args;

		void makePath(const std::string &path,
			const InArgs &in);
		void addPathInArg(const std::string &path,
			const InArgs &in,
			InArgs::const_iterator &itInArg,
			PathArgument::Kind kind);
		void invalidPath(const std::string &path,
			int location);

		Args args_;
	};

	class ValueAllocator
	{
	public:
		enum { unknown = (unsigned)-1 };

		virtual ~ValueAllocator();

		virtual char *makeMemberName(const char *memberName) = 0;
		virtual void releaseMemberName(char *memberName) = 0;
		virtual char *duplicateStringValue(const char *value,
			unsigned int length = unknown) = 0;
		virtual void releaseStringValue(char *value) = 0;
	};

	class ValueIteratorBase
	{
	public:
		typedef unsigned int size_t;
		typedef int difference_type;
		typedef ValueIteratorBase SelfType;

		ValueIteratorBase();
		explicit ValueIteratorBase(const Value::ObjectValues::iterator &current);

		bool operator ==(const SelfType &other) const
		{
			return isEqual(other);
		}

		bool operator !=(const SelfType &other) const
		{
			return !isEqual(other);
		}

		difference_type operator -(const SelfType &other) const
		{
			return computeDistance(other);
		}

		/// Return either the index or the member name of the referenced value as a Value.
		Value key() const;

		/// Return the index of the referenced Value. -1 if it is not an arrayValue.
		UInt index() const;

		/// Return the member name of the referenced Value. "" if it is not an objectValue.
		const char *memberName() const;

	protected:
		Value &deref() const;

		void increment();

		void decrement();

		difference_type computeDistance(const SelfType &other) const;

		bool isEqual(const SelfType &other) const;

		void copy(const SelfType &other);

	private:
		Value::ObjectValues::iterator current_;
		// Indicates that iterator is for a null value.
		bool isNull_;
	};

	class ValueConstIterator : public ValueIteratorBase
	{
		friend class Value;
	public:
		typedef unsigned int size_t;
		typedef int difference_type;
		typedef const Value &reference;
		typedef const Value *pointer;
		typedef ValueConstIterator SelfType;

		ValueConstIterator();
	private:
		explicit ValueConstIterator(const Value::ObjectValues::iterator &current);
	public:
		SelfType &operator =(const ValueIteratorBase &other);

		SelfType operator++(int)
		{
			SelfType temp(*this);
			++*this;
			return temp;
		}

		SelfType operator--(int)
		{
			SelfType temp(*this);
			--*this;
			return temp;
		}

		SelfType &operator--()
		{
			decrement();
			return *this;
		}

		SelfType &operator++()
		{
			increment();
			return *this;
		}

		reference operator *() const
		{
			return deref();
		}
	};

	class ValueIterator : public ValueIteratorBase
	{
		friend class Value;
	public:
		typedef unsigned int size_t;
		typedef int difference_type;
		typedef Value &reference;
		typedef Value *pointer;
		typedef ValueIterator SelfType;

		ValueIterator();
		ValueIterator(const ValueConstIterator &other);
		ValueIterator(const ValueIterator &other);
	private:
		explicit ValueIterator(const Value::ObjectValues::iterator &current);
	public:

		SelfType &operator =(const SelfType &other);

		SelfType operator++(int)
		{
			SelfType temp(*this);
			++*this;
			return temp;
		}

		SelfType operator--(int)
		{
			SelfType temp(*this);
			--*this;
			return temp;
		}

		SelfType &operator--()
		{
			decrement();
			return *this;
		}

		SelfType &operator++()
		{
			increment();
			return *this;
		}

		reference operator *() const
		{
			return deref();
		}
	};

	class Features
	{
	public:
		/** \brief A configuration that allows all features and assumes all strings are UTF-8.
		* - C & C++ comments are allowed
		* - Root object can be any JSON value
		* - Assumes Value strings are encoded in UTF-8
		*/
		static Features all();

		/** \brief A configuration that is strictly compatible with the JSON specification.
		* - Comments are forbidden.
		* - Root object must be either an array or an object value.
		* - Assumes Value strings are encoded in UTF-8
		*/
		static Features strictMode();

		/** \brief Initialize the configuration like JsonConfig::allFeatures;
		*/
		Features();

		/// \c true if comments are allowed. Default: \c true.
		bool allowComments_;

		/// \c true if root must be either an array or an object value. Default: \c false.
		bool strictRoot_;
	};

	class Reader
	{
	public:
		typedef char Char;
		typedef const Char *Location;

		/** \brief Constructs a Reader allowing all features
		* for parsing.
		*/
		Reader();

		/** \brief Constructs a Reader allowing the specified feature set
		* for parsing.
		*/
		Reader(const Features &features);

		/** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
		* \param document UTF-8 encoded string containing the document to read.
		* \param root [out] Contains the root value of the document if it was
		*             successfully parsed.
		* \param collectComments \c true to collect comment and allow writing them back during
		*                        serialization, \c false to discard comments.
		*                        This parameter is ignored if Features::allowComments_
		*                        is \c false.
		* \return \c true if the document was successfully parsed, \c false if an error occurred.
		*/
		bool parse(const std::string &document,
			Value &root,
			bool collectComments = true);

		/** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
		* \param document UTF-8 encoded string containing the document to read.
		* \param root [out] Contains the root value of the document if it was
		*             successfully parsed.
		* \param collectComments \c true to collect comment and allow writing them back during
		*                        serialization, \c false to discard comments.
		*                        This parameter is ignored if Features::allowComments_
		*                        is \c false.
		* \return \c true if the document was successfully parsed, \c false if an error occurred.
		*/
		bool parse(const char *beginDoc, const char *endDoc,
			Value &root,
			bool collectComments = true);

		/// \brief Parse from input stream.
		/// \see Json::operator>>(std::istream&, Json::Value&).
		bool parse(std::istream &is,
			Value &root,
			bool collectComments = true);

		/** \brief Returns a user friendly string that list errors in the parsed document.
		* \return Formatted error message with the list of errors with their location in
		*         the parsed document. An empty string is returned if no error occurred
		*         during parsing.
		*/
		std::string getFormatedErrorMessages() const;

	private:
		enum TokenType
		{
			tokenEndOfStream = 0,
			tokenObjectBegin,
			tokenObjectEnd,
			tokenArrayBegin,
			tokenArrayEnd,
			tokenString,
			tokenNumber,
			tokenTrue,
			tokenFalse,
			tokenNull,
			tokenArraySeparator,
			tokenMemberSeparator,
			tokenComment,
			tokenError
		};

		class Token
		{
		public:
			TokenType type_;
			Location start_;
			Location end_;
		};

		class ErrorInfo
		{
		public:
			Token token_;
			std::string message_;
			Location extra_;
		};

		typedef std::deque<ErrorInfo> Errors;

		bool expectToken(TokenType type, Token &token, const char *message);
		bool readToken(Token &token);
		void skipSpaces();
		bool match(Location pattern,
			int patternLength);
		bool readComment();
		bool readString();
		void readNumber();
		bool readValue();
		bool readObject(Token &token);
		bool readArray(Token &token);
		bool decodeNumber(Token &token);
		bool decodeString(Token &token);
		bool decodeString(Token &token, std::string &decoded);
		bool decodeDouble(Token &token);
		bool decodeUnicodeCodePoint(Token &token,
			Location &current,
			Location end,
			unsigned int &unicode);
		bool decodeUnicodeEscapeSequence(Token &token,
			Location &current,
			Location end,
			unsigned int &unicode);
		bool addError(const std::string &message,
			Token &token,
			Location extra = 0);
		bool recoverFromError(TokenType skipUntilToken);
		bool addErrorAndRecover(const std::string &message,
			Token &token,
			TokenType skipUntilToken);
		void skipUntilSpace();
		Value &currentValue();
		Char getNextChar();
		void getLocationLineAndColumn(Location location,
			int &line,
			int &column) const;
		std::string getLocationLineAndColumn(Location location) const;
		void addComment(Location begin,
			Location end,
			CommentPlacement placement);
		void skipCommentTokens(Token &token);

		typedef std::stack<Value *> Nodes;
		Nodes nodes_;
		Errors errors_;
		std::string document_;
		Location begin_;
		Location end_;
		Location current_;
		Location lastValueEnd_;
		Value *lastValue_;
		std::string commentsBefore_;
		Features features_;
		bool collectComments_;
	};

	std::istream& operator>>(std::istream&, Value&);

	class Writer
	{
	public:
		virtual ~Writer();

		virtual std::string write(const Value &root) = 0;
	};

	class FastWriter : public Writer
	{
	public:
		FastWriter();
		virtual ~FastWriter(){}

		void enableYAMLCompatibility();

	public: // overridden from Writer
		virtual std::string write(const Value &root);

	private:
		void writeValue(const Value &value);

		std::string document_;
		bool yamlCompatiblityEnabled_;
	};

	class StyledWriter : public Writer
	{
	public:
		StyledWriter();
		virtual ~StyledWriter(){}

	public: // overridden from Writer
		/** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
		* \param root Value to serialize.
		* \return String containing the JSON document that represents the root value.
		*/
		virtual std::string write(const Value &root);

	private:
		void writeValue(const Value &value);
		void writeArrayValue(const Value &value);
		bool isMultineArray(const Value &value);
		void pushValue(const std::string &value);
		void writeIndent();
		void writeWithIndent(const std::string &value);
		void indent();
		void unindent();
		void writeCommentBeforeValue(const Value &root);
		void writeCommentAfterValueOnSameLine(const Value &root);
		bool hasCommentForValue(const Value &value);
		std::string normalizeEOL(const std::string &text);

		typedef std::vector<std::string> ChildValues;

		ChildValues childValues_;
		std::string document_;
		std::string indentString_;
		int rightMargin_;
		int indentSize_;
		bool addChildValues_;
	};

	class StyledStreamWriter
	{
	public:
		StyledStreamWriter(std::string indentation = "\t");
		~StyledStreamWriter(){}

	public:
		/** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
		* \param out Stream to write to. (Can be ostringstream, e.g.)
		* \param root Value to serialize.
		* \note There is no point in deriving from Writer, since write() should not return a value.
		*/
		void write(std::ostream &out, const Value &root);

	private:
		void writeValue(const Value &value);
		void writeArrayValue(const Value &value);
		bool isMultineArray(const Value &value);
		void pushValue(const std::string &value);
		void writeIndent();
		void writeWithIndent(const std::string &value);
		void indent();
		void unindent();
		void writeCommentBeforeValue(const Value &root);
		void writeCommentAfterValueOnSameLine(const Value &root);
		bool hasCommentForValue(const Value &value);
		std::string normalizeEOL(const std::string &text);

		typedef std::vector<std::string> ChildValues;

		ChildValues childValues_;
		std::ostream* document_;
		std::string indentString_;
		int rightMargin_;
		std::string indentation_;
		bool addChildValues_;
	};


	std::string valueToString(Int value);
	std::string valueToString(UInt value);
	std::string valueToString(double value);
	std::string valueToString(bool value);
	std::string valueToQuotedString(const char *value);

	std::ostream& operator<<(std::ostream&, const Value &root);

} // namespace gyp
