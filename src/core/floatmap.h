class FloatMap
{
	std::vector<float> floats;
	uint m_width;
public:
	FloatMap()
	{
		m_width = 0;
	}
	FloatMap(uint _width)
		: m_width(_width)
	{
		floats.resize(_width * _width);
	}

	float& operator() (uint _x, uint _y)
	{
		_ASSERT(_x < m_width && _y < m_width);
		return floats[_y * m_width + _x];
	}

	const float& operator() (uint _x, uint _y) const
	{
		_ASSERT(_x < m_width && _y < m_width);
		return floats[_y * m_width + _x];
	}

};