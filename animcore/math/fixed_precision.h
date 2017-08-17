#pragma once
#include <type_traits>
#include <stdint.h>
#include "util/namespace.h"

ANIM_NAMESPACE_BEGIN

template<typename T>
struct DoubleSizeType
{
};

template<>
struct DoubleSizeType<int8_t>
{
	typedef int16_t value;
};

template<>
struct DoubleSizeType<int16_t>
{
	typedef int32_t value;
};

template<>
struct DoubleSizeType<int32_t>
{
	typedef int64_t value;
};

template <typename BaseType, uint32_t PRECISION>
class FixedPoint
{
public:
	typedef typename DoubleSizeType<BaseType>::value NextType;

	FixedPoint operator+(FixedPoint other)
	{
		other.m_Value += m_Value;
		return other;
	}

	FixedPoint& operator+=(FixedPoint other)
	{
		m_Value += other.m_Value;
		return *this;
	}

	FixedPoint operator-(FixedPoint other)
	{
		other.m_Value -= m_Value;
		return other;
	}

	FixedPoint& operator-=(FixedPoint other)
	{
		m_Value -= other.m_Value;
		return *this;
	}

	FixedPoint& operator*=(FixedPoint other)
	{
		m_Value = ((NextType)(m_Value)* other.m_Value) >> PRECISION;
		return *this;
	}

	FixedPoint operator*(FixedPoint other)
	{
		other *= *this;
		return other;
	}

	FixedPoint& operator/=(FixedPoint other)
	{
		m_Value = (m_Value << PRECISION) / other.m_Value;
		return *this;
	}

	FixedPoint operator/(FixedPoint other)
	{
		FixedPoint p(*this);
		p /= other;
		return p;
	}

	bool operator==(FixedPoint other) const
	{
		return m_Value == other.m_Value;
	}

	bool operator!=(FixedPoint other) const
	{
		return m_Value != other.m_Value;
	}

	bool operator>(FixedPoint other) const
	{
		return m_Value > other.m_Value;
	}

	bool operator>=(FixedPoint other) const
	{
		return m_Value >= other.m_Value;
	}

	bool operator<(FixedPoint other) const
	{
		return m_Value < other.m_Value;
	}

	bool operator<=(FixedPoint other) const
	{
		return m_Value <= other.m_Value;
	}

	static constexpr float EPSILON_F = 1 / (float)(1 << PRECISION);

	FixedPoint& operator=(float f)
	{
		FromFloat(f);
		return *this;
	}

	float ToFloat() const
	{
		return m_Value / (float)((NextType)1 << PRECISION);
	}

	void FromFloat(float value)
	{
		m_Value = (BaseType)(value * ((NextType)1 << PRECISION));
	}
private:
	BaseType m_Value;
};

using FPRatio32 = FixedPoint<int32_t, 31>;
using FPRatio64 = FixedPoint<int64_t, 63>;

static_assert(std::is_pod<FPRatio32>::value, "Not POD");
static_assert(std::is_pod<FPRatio64>::value, "Not POD");

ANIM_NAMESPACE_END
