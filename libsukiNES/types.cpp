#include "types.h"

bool operator>(const ManagedWord& left, int right)
{
	return left.m_impl.Word > right;
}

bool operator<(const ManagedWord& left, int right)
{
	return left.m_impl.Word < right;
}

bool operator>=(const ManagedWord& left, int right)
{
	return left.m_impl.Word >= right;
}

bool operator<=(const ManagedWord& left, int right)
{
	return left.m_impl.Word <= right;
}
