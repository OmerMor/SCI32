


class X  {
	public:
	X()							{cout << "1";}
	~X()							{cout << "2";}
	X(const X&)					{cout << "3";}
	X& operator=(const X&)	{cout << "4";}
};

class Y : public X  {
	public:
	Y()							{cout << "5";}
	~Y()							{cout << "6";}
	Y(const Y&)					{cout << "7";}
	Y& operator=(const Y&)	{cout << "8";}
};

X x;

main()
{
	cout << "9"
	Y y;
	X x = y;
	x = y;
	y = y;
}


