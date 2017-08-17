#pragma once
#include <math.h>

namespace animengine
{
	class Vector3
	{
	public:
		Vector3()
			: m_X(0)
			, m_Y(0)
			, m_Z(0)
		{
		}

		Vector3(float x, float y, float z)
			: m_X(x)
			, m_Y(y)
			, m_Z(z)
		{
		}

		inline Vector3& operator*=(float scalar)
		{
			m_X *= scalar;
			m_Y *= scalar;
			m_Z *= scalar;
			return *this;
		}

		inline Vector3 operator*(float scalar)
		{
			Vector3 v(*this);
			v *= scalar;
			return v;
		}

		inline Vector3& operator/=(float scalar)
		{
			m_X /= scalar;
			m_Y /= scalar;
			m_Z /= scalar;
		}

		inline Vector3 operator/(float scalar)
		{
			Vector3 v(*this);
			v /= scalar;
			return v;
		}

		inline Vector3& operator+=(const Vector3& v)
		{
			m_X += v.m_X;
			m_Y += v.m_Y;
			m_Z += v.m_Z;
		}

		inline Vector3 operator+(const Vector3& v)
		{
			Vector3 x(*this);
			x += v;
			return x;
		}

		inline Vector3& operator-=(const Vector3& v)
		{
			m_X -= v.m_X;
			m_Y -= v.m_Y;
			m_Z -= v.m_Z;
		}

		inline Vector3 operator-(const Vector3& v)
		{
			Vector3 x(*this);
			x -= v;
			return x;
		}

		inline float Length() const
		{
			return sqrtf(LengthSq());
		}

		inline float LengthSq() const
		{
			return m_X*m_X + m_Y*m_Y + m_Z*m_Z;
		}

		inline Vector3& Normalize()
		{
			auto len = Length();
			m_X /= len;
			m_Y /= len;
			m_Z /= len;
		}

		inline Vector3 GetNormalized()
		{
			Vector3 v(*this);
			v.Normalize();
			return v;
		}

		inline float Dot(const Vector3 & v)
		{
			return m_X*v.m_X + m_Y*v.m_Y + m_Z*v.m_Z;
		}

		inline Vector3 Cross(const Vector3& v)
		{
			return Vector3(m_Y*v.m_Z - m_Z*v.m_Y, m_Z*v.m_X - m_X*v.m_Z, m_X*v.m_Y - m_Y*v.m_X);
		}
		
		union
		{
			struct
			{
				float m_X;
				float m_Y;
				float m_Z;
			};
			float m_V[3];
		};

	};
}