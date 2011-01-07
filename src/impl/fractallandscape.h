#include "../core/algebra.h"
#include "../core/floatmap.h"
class FractalLandscape
{
	FloatMap heights;
	float width, height;
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	Point corner;
	
	FractalLandscape(Point &a, Point &b, uint iterations) {
		corner = Point(std::min(a[0], b[0]), std::min(a[1], b[1]), a[2]);
		width = abs(a[0] - b[0]);
		height = abs(a[1] - b[1]);
		number_of_squares_in_one_axis = pow(2, iterations);
		number_of_vertices_in_one_axis = number_of_squares_in_one_axis + 1;
		heights = FloatMap(number_of_squares_in_one_axis);
	}

	//Adds all triangles contained in this object to a scene
	void addReferencesToScene(std::vector<Primitive*> &_scene) const;
};

