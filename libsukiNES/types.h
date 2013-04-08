#pragma once

#include <algorithm>

/**
 * @brief A wrapper around endian aware word(16 bit unsigned integer)
 *
 * The class works the same way as a normal unsigned short except you
 * can get the high byte and the low byte of the integer independent
 * of the endianess of the processor.
 *
 * A word typedef is available.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class ManagedWord
{
public:
	/**
	 * @brief Create a new instance of ManagedWord
	 */
	ManagedWord()
	{
		m_impl.Word = 0;
	}

	/**
	 * @brief Create a new instance of ManagedWord with initial value
	 * @param value unsigned short value
	 */
	ManagedWord(uint16 value)
	{
		m_impl.Word = value;
	}

	/**
	 * @brief Copy constructor
	 * @param copy Instance to copy
	 */
	ManagedWord(const ManagedWord &copy)
	{
		this->m_impl = copy.m_impl;
	}

	/**
	 * @brief Copy-assignment operator
	 * @param other Instance to assign.
	 */
	ManagedWord &operator=(const ManagedWord &other)
	{
		if( this != &other )
		{
			this->m_impl = other.m_impl;
		}

		return *this;
	}

	/**
	 * @brief Get the lower byte of the word
	 * @return Lower byte
	 */
	byte lowByte() const
	{
		return m_impl.Byte.low;
	}

	/**
	 * @brief Get the high byte of the word
	 * @return High byte
	 */
	byte highByte() const
	{
		return m_impl.Byte.high;
	}

	/**
	 * @brief Set the low byte of the word
	 * @param value Low byte value
	 */
	void setLowByte(byte value)
	{
		m_impl.Byte.low = value;
	}

	/**
	 * @brief Set the high byte of the word
	 * @param value High byte value
	 */
	void setHighByte(byte value)
	{
		m_impl.Byte.high = value;
	}

	///**
	// * @brief Convert word to short
	// * @return short
	// */
	//inline operator short()
	//{
	//	return static_cast<short>(m_impl.Word);
	//}

	///**
	// * @brief Convert word to unsigned short
	// * @return unsigned short
	// */
	//inline operator unsigned short()
	//{
	//	return m_impl.Word;
	//}

	// NOTE: There are no operator to unsigned int
	// because its cause too many ambiguity for
	// the compiler

	/**
	 * @brief Convert word to unsigned int
	 * @return unsigned int
	 */
	inline operator int()
	{
		return static_cast<int>(m_impl.Word);
	}

	/**
	 * @brief Assign value to word
	 * @param value unsigned short value to assign
	 * @return this instance
	 */
	inline ManagedWord &operator=(uint16 value)
	{
		m_impl.Word = value;
		return *this;
	}

	/**
	 * @brief Prefix operator++, increment now the value of the word
	 * @return this instance
	 */
	inline ManagedWord &operator++()
	{
		++m_impl.Word;
		return *this;
	}

	/**
	 * @brief Postfix operator++, increment the value after the instruction
	 * @return this instance
	 */
	inline ManagedWord &operator++(int)
	{
		// Postfix version
		m_impl.Word++;
		return *this;
	}

	/**
	 * @brief Prefix operator--, decrement now the value of the word
	 * @return this instance
	 */
	inline ManagedWord &operator--()
	{
		--m_impl.Word;
		return *this;
	}

	/**
	 * @brief Postfix operator--, decrement the value after the instruction
	 */
	inline ManagedWord &operator--(int)
	{
		m_impl.Word--;
		return *this;
	}

	bool operator!=(const ManagedWord& other)
	{
		return m_impl.Word != other.m_impl.Word;
	}

	friend bool operator>(const ManagedWord& left, int right);

	friend bool operator<(const ManagedWord& left, int right);

	friend bool operator>=(const ManagedWord& left, int right);

	friend bool operator<=(const ManagedWord& left, int right);

private:
	// An union represent a single value in memory.
	// The Byte and Word contain the same value
	// except with Byte you can get the high and low byte
	union
	{
		struct
		{
#ifdef SUKINES_LSB
			byte low, high;
#else
			byte high, low;
#endif
		} Byte;
		uint16 Word;
	} m_impl;
};

// From bisqwit nesemu1
template<unsigned bit, unsigned nbits=1, typename T=u8>
struct RegBit
{
	T data;
	enum { mask = (1u << nbits) - 1u };
	template<typename T2>
	RegBit& operator=(T2 val)
	{
		data = (data & ~(mask << bit)) | ((nbits > 1 ? val & mask : !!val) << bit);
		return *this;
	}

	operator unsigned() const { return (data >> bit) & mask; }
	RegBit& operator++() { return *this = *this + 1; }
	unsigned operator++(int) { unsigned r = *this; ++*this; return r; }
};

template<typename T>
class DynamicArray
{
public:
	DynamicArray()
	: _data(nullptr)
	, _size(0)
	{
	}

	DynamicArray(size_t size)
	: _data(nullptr)
	, _size(size)
	{
		_data = new T[size];
	}

	DynamicArray(const DynamicArray& other)
	: _data(nullptr)
	, _size(other._size)
	{
		operator=(other);
	}

	DynamicArray(DynamicArray&& move)
	: _data(std::move<T*>(move._data))
	, _size(move._size)
	{
		move._data = nullptr;
	}

	DynamicArray& operator=(const DynamicArray& other)
	{
		if (this != &other)
		{
			if (_data)
			{
				delete[] _data;
				_data = nullptr;
			}

			_data = new T[other._size];
			std::memcpy(_data, other._data, other._size);
		}

		return *this;
	}

	DynamicArray& operator=(DynamicArray&& other)
	{
		if (this != &other)
		{
			_data = other._data;
			_size = other._size;

			other._data = nullptr;
		}

		return *this;
	}

	~DynamicArray()
	{
		if (_data)
		{
			delete[] _data;
			_data = nullptr;
		}
	}

	T* get() const
	{
		return _data;
	}

	T operator[](size_t index) const
	{
		return _data[index];
	}

	size_t size() const
	{
		return _size;
	}

private:
	T* _data;
	size_t _size;
};