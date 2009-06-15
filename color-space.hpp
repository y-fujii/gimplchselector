#pragma once
/*
	L*C*h* color selector for GIMP
	by y.fujii <y-fujii at mimosa-pudica.net>, public domain
*/

#include <algorithm>
#include <cmath>
#include <libgimpcolor/gimpcolor.h>


namespace color {

struct Lch {
	Lch() {}

	Lch( double l_, double c_, double h_ ):
		l( l_ ), c( c_ ), h( h_ ) {}

	double l, c, h;
};

struct Lab {
	Lab() {}

	Lab( double l_, double a_, double b_ ):
		l( l_ ), a( a_ ), b( b_ ) {}

	double l, a, b;
};


template<class T>
inline bool clip( T& dst, const T& src, const T& low, const T& upp ) {
	if( src < low ) {
		dst = low;
		return false;
	}
	else if( upp < src ) {
		dst = upp;
		return false;
	}
	else {
		dst = src;
		return true;
	}
}

inline bool convert( Lab& lab, const GimpRGB& srgb ) {
	using namespace std;
	double lr = pow( max( srgb.r, 0.0 ), 2.2 );
	double lg = pow( max( srgb.g, 0.0 ), 2.2 );
	double lb = pow( max( srgb.b, 0.0 ), 2.2 );
	double y = 0.2126 * lr + 0.7152 * lg + 0.0722 * lb;
	double x = (0.4124 / 0.9505) * lr + (0.3576 / 0.9505) * lg + (0.1805 / 0.9505) * lb;
	double z = (0.0193 / 1.0890) * lr + (0.1192 / 1.0890) * lg + (0.9505 / 1.0890) * lb;
	double fy = pow( y, 1.0 / 3.0 );
	double fx = pow( x, 1.0 / 3.0 );
	double fz = pow( z, 1.0 / 3.0 );
	lab.l = fy;
	lab.a = 5.0 * (fx - fy);
	lab.b = 2.0 * (fy - fz);

	return true;
}

inline bool convert( GimpRGB& srgb, const Lab& lab ) {
	using namespace std;
	double fy = lab.l;
	double fx = fy + lab.a / 5.0;
	double fz = fy - lab.b / 2.0;
	double y = fy * fy * fy;
	double x = fx * fx * fx;
	double z = fz * fz * fz;
	double lr = ( 3.2410 * 0.9505) * x + (-1.5374) * y + (-0.4986 * 1.0890) * z;
	double lg = (-0.9692 * 0.9505) * x + ( 1.8760) * y + ( 0.0416 * 1.0890) * z;
	double lb = ( 0.0556 * 0.9505) * x + (-0.2040) * y + ( 1.0570 * 1.0890) * z;
	bool inr =
		clip( lr, lr, 0.0, 1.0 ) &
		clip( lg, lg, 0.0, 1.0 ) &
		clip( lb, lb, 0.0, 1.0 );
	srgb.r = pow( lr, 1.0 / 2.2 );
	srgb.g = pow( lg, 1.0 / 2.2 );
	srgb.b = pow( lb, 1.0 / 2.2 );

	return inr;
}

inline bool convert( Lab& lab, const Lch& lch ) {
	using namespace std;
	lab.l = lch.l;
	lab.a = lch.c * cos( lch.h );
	lab.b = lch.c * sin( lch.h );

	return true;
}

inline bool convert( Lch& lch, const Lab& lab ) {
	using namespace std;
	lch.l = lab.l;
	lch.c = sqrt( lab.a * lab.a + lab.b * lab.b );
	lch.h = atan2( -lab.b, -lab.a ) + M_PI;

	return true;
}

}
