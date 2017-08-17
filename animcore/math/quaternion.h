#pragma once
#include <math.h>
#include "utils.h"

namespace animengine
{
	class Quaternion
	{
	public:
		union 
		{
			struct
			{
				float m_X;
				float m_Y;
				float m_Z;
				float m_W;
			};
			float m_V[4];
		};

		Quaternion()
			: m_X(0)
			, m_Y(0)
			, m_Z(0)
			, m_W(0)
		{
		}

		Quaternion(float x, float y, float z, float w)
			: m_X(x)
			, m_Y(y)
			, m_Z(z)
			, m_W(w)
		{
		}

		Quaternion& operator*=(float s)
		{
			m_X *= s;
			m_Y *= s;
			m_Z *= s;
			m_W *= s;
			return *this;
		}

		Quaternion operator*(float s)
		{
			Quaternion q(*this);
			q *= s;
			return q;
		}

		Quaternion& operator/=(float s)
		{
			m_X /= s;
			m_Y /= s;
			m_Z /= s;
			m_W /= s;
			return *this;
		}

		Quaternion operator/(float s)
		{
			Quaternion q(*this);
			q /= s;
			return q;
		}

		Quaternion& operator*=(const Quaternion& q)
		{
			/*auto x = m_X*q.m_W + m_Y*q.m_Z - m_Z*q.m_Y + m_W*q.m_X;
			auto y = -m_X*q.m_Z + m_Y*q.m_W + m_Z*q.m_X + m_W*q.m_Y;
			auto z = m_X*q.m_Y - m_Y*q.m_X + m_Z*q.m_W + m_W*q.m_Z;
			auto w = -m_X*q.m_X - m_Y*q.m_X - m_Z*q.m_X + m_W*q.m_W;*/

			auto w = m_W*q.m_W - m_X*q.m_X - m_Y*q.m_Y - m_Z*q.m_Z;
			auto x = m_W*q.m_X + m_X*q.m_W + m_Y*q.m_Z - m_Z*q.m_Y;
			auto y = m_W*q.m_Y - m_X*q.m_Z + m_Y*q.m_W + m_Z*q.m_X;
			auto z = m_W*q.m_Z + m_X*q.m_Y - m_Y*q.m_X + m_Z*q.m_W;
			
 			m_X = x;
			m_Y = y;
			m_Z = z;
			m_W = w;
			return *this;
		}

		Quaternion operator*(const Quaternion &q)
		{
			Quaternion q2(*this);
			q2 *= q;
			return q2;
		}

		Quaternion Inverse() const
		{
			auto lengthSq = m_X*m_X + m_Y*m_Y + m_Z*m_Z + m_W*m_W;
			return (Quaternion(-m_X, -m_Y, -m_Z, m_W) / lengthSq);
		}

		static Quaternion FromEuler(float pitch, float roll, float yaw)
		{
			float t0 = cosf(yaw * 0.5f);
			float t1 = sinf(yaw * 0.5f);
			float t2 = cosf(roll * 0.5f);
			float t3 = sinf(roll * 0.5f);
			float t4 = cosf(pitch * 0.5f);
			float t5 = sinf(pitch * 0.5f);

			Quaternion q;
			q.m_W = t0*t2*t4 + t1*t3*t5;
			q.m_X = t0*t3*t4 - t1*t2*t5;
			q.m_Y = t0*t2*t5 + t1*t3*t4;
			q.m_Z = t1*t2*t4 - t0*t3*t5;
			return q;
		}

		void ToEuler(float& pitch, float& roll, float& yaw) const
		{
			auto xsq = m_X*m_X;
			auto ysq = m_Y*m_Y;
			auto zsq = m_Z*m_Z;
			auto wsq = m_W*m_W;

			float t0 = 2.0f * (m_W*m_X + m_Y*m_Z);
			float t1 = -xsq - ysq + zsq + wsq;
			roll = atan2f(t0, t1);

			float t2 = 2.0f * (m_W*m_Y - m_Z*m_X);
			t2 = CLAMP(t2, -1.0f, 1.0f);
			pitch = asinf(t2);

			float t3 = 2.0f * (m_W*m_Z + m_X*m_Y);
			float t4 = xsq - ysq - zsq + wsq;
			yaw = atan2f(t3, t4);
		}

		void Normalize()
		{
			auto length = sqrtf(m_X*m_X + m_Y*m_Y + m_Z*m_Z + m_W*m_W);
			m_X /= length;
			m_Y /= length;
			m_Z /= length;
			m_W /= length;
		}
	};
}