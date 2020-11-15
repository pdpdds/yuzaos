// main.cpp font_to_svg - public domain

#include <fstream>
#include "font_to_svg.hpp"

void genSvg(std::string fontName, std::string charCode) 
{
	font2svg::glyph g(fontName.c_str(), charCode);
	std::string fname = std::string("Output/");
	fname += charCode;
	fname += ".svg";
	std::ofstream file(fname.c_str());
	file << g.svgheader().c_str()
		<< g.svgborder().c_str()
		<< g.svgtransform().c_str()
		<< g.axes().c_str()
		<< g.typography_box().c_str()
		<< g.points().c_str()
		<< g.pointlines().c_str()
		<< g.outline().c_str()
		<< g.labelpts().c_str()
		<< g.svgfooter().c_str();

	//file << g.svgheader().c_str() << g.svgtransform().c_str() << g.outline().c_str() << g.svgfooter().c_str();

	g.free();
	file.close();
}

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		std::cout << "usage: " << argv[0] << " file.ttf 0x0042\n";
		exit(1);
	}

	genSvg(argv[1], argv[2]);

	return 0;
}
