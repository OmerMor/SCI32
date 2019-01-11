#ifndef RGB_HPP
#define RGB_HPP

class Rgb24
{
	public:
	Rgb24(uchar r, uchar g, uchar b) : red(r), green(g), blue(b) {}
	Rgb24() {}

	Bool	operator==(const Rgb24&) const;
	Bool	operator!=(const Rgb24& src) const;

	uchar	Lum() const;
	int	Dist(const Rgb24&) const;

	uchar red;
	uchar green;
	uchar blue;
};



class Rgb24F : public Rgb24
{
	public:
	Rgb24F(uchar r, uchar g, uchar b, uchar f) : Rgb24(r, g, b), flag(f) {}
	Rgb24F() {}

	int	operator==(const Rgb24F&) const;
	int	operator!=(const Rgb24F& src) const		{return !operator==(src);}

	uchar flag;
};





inline int
Rgb24F::operator==(const Rgb24F& src) const
{
	if (flag != src.flag)
		return False;
	if (red == src.red  &&  green == src.green  &&  blue == src.blue)
		return True;
	return False;
}

inline Bool
Rgb24::operator==(const Rgb24& src) const
{
	return red == src.red  &&  green == src.green  &&  blue == src.blue;
}

inline Bool
Rgb24::operator!=(const Rgb24& src) const
{
	return red != src.red  ||  green != src.green  || blue != src.blue;
}

inline uchar
Rgb24::Lum() const
{
	return ((red * 77L) + (green * 151L) + (blue * 28L)) >> 8;
}

inline int
Rgb24::Dist(const Rgb24& src) const
{
	int dif = red - src.red;
	int total = dif * dif;
	dif = green - src.green;
	total += dif * dif;
	dif = blue - src.blue;
	total += dif * dif;

	return total;
}

#endif
