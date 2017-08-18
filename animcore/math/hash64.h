#pragma once
#include <stdint.h>
#include <string.h>

namespace animengine
{
	namespace HashUtils
	{
		const uint64_t FNV_prime = 1099511628211UL;
		const uint64_t FNV_offset_basis = 14695981039346656037UL;

		//------------------------------------------------------------------------------
		uint64_t Compute(const void * data, size_t length)
		{
			const char * input = static_cast<const char *>(data);
			uint64_t result = FNV_offset_basis;
			for (size_t i = 0; i < length; ++i)
			{
				result = (result ^ input[i]) * FNV_prime;
			}
			return result;
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const char * value)
		{
			const char * input = value;
			uint64_t result = FNV_offset_basis;
			while (*input)
			{
				result = (result ^ *input) * FNV_prime;
				++input;
			}
			return result;
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const wchar_t * value)
		{
			size_t strLen = wcslen(value);
			return Compute(reinterpret_cast<const char *>(value), strLen * sizeof(wchar_t));
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const int & value)
		{
			return Compute(&value, sizeof(value));
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const unsigned int & value)
		{
			return Compute(&value, sizeof(value));
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const int64_t & value)
		{
			return Compute((const void *)&value, sizeof(value));
		}

		//------------------------------------------------------------------------------
		uint64_t Compute(const uint64_t & value)
		{
			return Compute((const void *)&value, sizeof(value));
		}

		//------------------------------------------------------------------------------
		template <typename T>
		void combine(uint64_t & seed, const T & value)
		{
			seed ^= Compute(value) +
				// 2^64/phi.
				0x9E3779B97F4A7C15 +
				// make sure bits spread across the output even if input hashes
				// have a small output range.
				(seed << 5) + (seed >> 3);
		}
	}
}