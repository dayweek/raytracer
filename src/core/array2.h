template <class T>
class Array2
{
	std::vector<T> m_bits;
	uint m_width;
public:
	Array2()
	{
		m_width = 0;
	}
	Array2(uint _width)
		: m_width(_width)
	{
		m_bits.resize(_width * _width);
	}

	T& operator() (uint _x, uint _y)
	{
		_ASSERT(_x < m_width && _y < m_width);
		return m_bits[_y * m_width + _x];
	}

	const T& operator() (uint _x, uint _y) const
	{
		_ASSERT(_x < m_width && _y < m_width);
		return m_bits[_y * m_width + _x];
	}

};