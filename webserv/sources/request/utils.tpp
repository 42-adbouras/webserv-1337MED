template<typename T>

str toString(T n) {
	std::ostringstream ss;
	ss << n;
	return ss.str();
}
