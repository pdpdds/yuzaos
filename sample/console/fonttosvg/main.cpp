// main.cpp font_to_svg - public domain

#include "font_to_svg.hpp"

int main( int argc, char * argv[] )
{
	if (argc!=3) {
		std::cout << "usage: " << argv[0] << " file.ttf 0x0042\n";
		exit( 1 );
	}

	font2svg::glyph g( argv[1], argv[2] );
	std::cout << g.svgheader().c_str()
		<< g.svgborder().c_str()
		<< g.svgtransform().c_str()
		<< g.axes().c_str()
		<< g.typography_box().c_str()
		<< g.points().c_str()
		<< g.pointlines().c_str()
		<< g.outline().c_str()
		<< g.labelpts().c_str()
		<< g.svgfooter().c_str();

	g.free();

  return 0;
}
