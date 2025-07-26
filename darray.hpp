#pragma once


namespace col
{
	class alloc_exception {};
	class index_exception {};
	class invalid_iterator {};

	template <typename T>
	class dArray
	{
		enum byte : char {};

		struct bufT
		{
			alignas(T) byte mem[sizeof(T)];
		};


	public:
		inline dArray() noexcept {}

		inline dArray(size_t capacity)
		{
			reserveCapacity(capacity);
		}

		inline dArray(const dArray& other)
		{
			*this = other;
		}

		inline dArray(dArray&& other) noexcept
		{
			*this = std::move(other);
		}

		inline dArray& operator=(const dArray& other)
		{
			clear();

			reserveCapacity(other.m_capacity);

			m_size = other.m_size;

			if constexpr (std::is_trivially_copyable_v<T>)
				std::copy(other.m_data, other.m_data + other.m_size, m_data);
			else
				for (size_t i = 0; i < m_size; ++i)
					new (AsBuf(i)) T(other.operator[](i));

			return *this;
		}

		inline dArray& operator=(dArray&& other) noexcept
		{
			if constexpr (std::is_trivially_move_assignable_v<T>)
			{
				m_data = std::exchange(other.m_data, nullptr);
				m_capacity = std::exchange(other.m_capacity, 0);
				m_size = std::exchange(other.m_size, 0);
			}
			else
			{
				reserveCapacity(other.m_capacity);

				for (size_t i = 0; i < other.m_size; ++i)
					*AsPtr(i) = std::move(*other.AsPtr(i));

				m_size = other.m_size;

				other.destroy();
			}

			return *this;
		}

		virtual ~dArray() noexcept
		{
			clear();
			destroy();
		}

		inline T& operator[](size_t index) noexcept
		{
			return *AsPtr(index);
		}

		inline const T& operator[](size_t index) const noexcept
		{
			return *AsPtr(index);
		}

		inline T& at(size_t index)
		{
			if (index > m_size) throw index_exception();
			return operator[](index);
		}

		inline const T* cbegin() noexcept
		{
			return begin();
		}

		inline const T* cend() noexcept
		{
			return end();
		}

		inline T* begin() noexcept
		{
			if (!m_size) return end();
			return AsPtr(0);
		}

		inline T* end() noexcept
		{
			return std::launder(reinterpret_cast<T*>(m_data + m_size));
		}

		inline void push_back(T item)
		{
			reserveCapacity(m_size + 1);
			*AsPtr(m_size++) = std::move(item);
		}

		template <class ...ARGS>
		inline T& emplace_back(ARGS&&... args)
		{
			reserveCapacity(m_size + 1);
			byte* mem = m_data[m_size++].mem;
			return *new(mem) T(std::forward<ARGS>(args)...);
		}

		inline bool empty() noexcept
		{
			return m_size == 0;
		}

		inline size_t size() noexcept
		{
			return m_size;
		}

		inline size_t capacity() noexcept
		{
			return m_capacity;
		}

		inline T* erase(const T* pos)
		{
			if (!m_size || pos < cbegin() || pos >= cend()) throw new invalid_iterator();

			size_t offset = (pos - cbegin()) / sizeof(bufT);

			AsPtr(offset)->~T();

			if (offset < m_size - 1)
				if constexpr (std::is_trivially_move_assignable_v<T>)
					memcpy(m_data[offset].mem, m_data[offset + 1].mem, (m_size - offset - 1) * sizeof(bufT));
				else
					for (size_t i = offset; i < m_size - 1; ++i)
						*AsPtr(i) = std::move(*AsPtr(i + 1));

			--m_size;

			if (m_size) return begin() + offset;
			return end();
		}

		inline void reserve(size_t amt)
		{
			reserveCapacity(amt);
		}

		inline void clear()
		{
			if (!m_capacity) return;

			for (size_t i = 0; i < m_size; i++)
			{
				AsPtr(i)->~T();
			}

			m_size = 0;
		}
		
	private:
		inline void destroy() noexcept
		{
			if (m_data)
			{
				delete[] m_data;
				m_data = nullptr;
			}
			m_capacity = 0;
			m_size = 0;
		}

		inline byte* AsBuf(size_t pos) noexcept
		{
			return &m_data[pos].mem;
		}

		inline T* AsPtr(size_t pos) const noexcept
		{
			return std::launder(reinterpret_cast<T*>(m_data[pos].mem));
		}

		inline T* AsPtr(size_t pos) noexcept
		{
			return std::launder(reinterpret_cast<T*>(m_data[pos].mem));
		}

		inline void reserveCapacity(size_t capacity)
		{
			if (m_size > capacity)
				capacity = m_size;

			if (m_capacity >= capacity)
				return;

			if (capacity < 2)
				capacity = 2;

			if ((m_capacity + (m_capacity >> 1)) > capacity)
				capacity = m_capacity + (m_capacity >> 1);

			bufT* newData = new bufT[capacity];

			if (!newData) throw new alloc_exception();

			if constexpr (std::is_trivially_move_assignable_v<T>)
				std::move(m_data, m_data + m_size, newData);
			else
				for (size_t i = 0; i < m_size; ++i)
				{
					T* ptr = std::launder(reinterpret_cast<T*>((newData + i)->mem));
					*ptr = std::move(*AsPtr(i));
				}

			std::swap(newData, m_data);

			delete[] newData;

			m_capacity = capacity;
		}

	private:
		bufT* m_data = nullptr;
		size_t m_size = 0;
		size_t m_capacity = 0;
	};
}
