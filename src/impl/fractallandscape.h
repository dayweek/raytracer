#include "../core/algebra.h"
#include "../core/array2.h"
class FractalLandscape
{
	Array2<float> heights;
	Array2<Vector> normals;
	float width, height;
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	float one_square_width;
	Point corner;
	
	FractalLandscape(Point &a, Point &b, uint iterations) {
		//init
		corner = Point(std::min(a[0], b[0]), std::min(a[1], b[1]), a[2]);
		width = abs(a[0] - b[0]);
		height = abs(a[1] - b[1]);
		number_of_squares_in_one_axis = pow(2, iterations);
		number_of_vertices_in_one_axis = number_of_squares_in_one_axis + 1;
		heights = Array2<float>(number_of_squares_in_one_axis);
		one_square_width = width/number_of_squares_in_one_axis;
		
		resetHeights(0.0f);
		perturbateSurface();
		generateNormals();
		generateVertexNormals();
		generateTriangles();
	}
protected:
	void resetHeights(float h)
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) 
				heights(x, y) = h;
	}
	
	void perturbateSurface();
	
	void generateNormals() 
	{
		for(uint y = 0; y < number_of_squares_in_one_axis - 1; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis - 1; x++) {
				Vector vx(one_square_width,
					 0,
					 heights(x + 1,y) - heights(x,y));
				Vector vy(0,
					 one_square_width,
					 heights(x,y + 1) - heights(x,y));
				normals(x,y) = vx % vy;
			}
	}
	
	void generateVertexNormals();
	void generateTriangles();
	//Adds all triangles contained in this object to a scene
	void addReferencesToScene(std::vector<Primitive*> &_scene) const;
};

