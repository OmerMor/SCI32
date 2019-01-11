#ifndef RATIO_HPP
#define RATIO_HPP

class Ratio
{
	public:
	Ratio()							:	numer(1), denom(1) {}
	Ratio(int num)					:  numer(num), denom(1) {}
	Ratio(int num, int denom)	:	numer(num), denom(denom) {}
	Ratio(const Ratio&);

	Bool	IsOne() const				{return numer == denom;}
	Bool	operator==(const Ratio&) const;
	Bool	operator!=(const Ratio&) const;
	Ratio	operator*(const Ratio&) const;
	#ifdef DEBUG
	Bool	Assert() const;
	#endif

	void	Normalize();

	int	numer;
	int	denom;
};

inline Ratio
Ratio::operator*(const Ratio& r) const
{
	Ratio r2;
	r2.numer = numer * r.numer;
	r2.denom = denom * r.denom;
	return r2;
}

inline int
operator/(int i, const Ratio& r)
{
	int val = (i * r.denom) / r.numer;
	return val;
}

inline int
operator/(const Ratio& r, int i)
{
	int val = (i * r.denom) / r.numer;
	return val;
}

inline int
operator*(int i, const Ratio& ratio)
{
	int total = i * ratio.numer;
	int val = total / ratio.denom;
	if (total > ratio.denom  &&  total % ratio.denom)
		val++;
	return val;
}

inline int
operator*(const Ratio& ratio, int i)
{
	int total = i * ratio.numer;
	int val = total / ratio.denom;
	if (total > ratio.denom  &&  total % ratio.denom)
		val++;
	return val;
}


inline int
operator^(int i, const Ratio& ratio)
{
	return (i * ratio.numer) / ratio.denom;
}

inline int
operator^(const Ratio& ratio, int i)
{
	return (i * ratio.numer) / ratio.denom;
}

#endif
