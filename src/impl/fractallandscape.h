#include "../core/algebra.h"
#include "../core/array2.h"
#include "../rt/basic_definitions.h"
#include "../rt/bvh.h"
#include "../rt/shading_basics.h"
#include "../rt/texture.h"

// class for generating mountain like objects. output is a set of faces. 
// Shader is shared between triangles
// limitation: mountain grows only in positive Z axis, textures are aligned with x and y coords
// Note: when computing vertex normals on the edges, we assume that surface around
//       the square has normal = (0,0,1)
// Warning: textures are only experimantal. scaling somehow does not work

class FractalLandscape
{
private:
	//This is the structure that is filled from the intersection
	//	routine and is than passed to the getShader routine, in case
	//	the face is the closest to the origin of the ray.
	struct ExtHitPoint : RefCntBase
	{
		//The barycentric coordinate (in .x, .y, .z) + the distance (in .w)
		float4 intResult;
	};
	Array2<float> heights; //height map
	Array2<std::pair<Vector, Vector> > squareNormals; //height map consists of squares, 
	//and each square has two triangle normals.
	Array2<Vector> vertexNormals;
	Array2<int> heightsSet; // did we already perturbated the height?
	std::vector<Vector> normals; //vertex normal are in the end of generating copyed here 
	// for easy indexing
	Array2<Point> vertices; 
	uint iterations; // number of iterations - how many times we want
	//to recursively perturbate
	// texture coordinates
	Array2<float2> textCoords; 
	// widht is width of a square that is given as 2 points
	float width; 
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	float one_square_width;
	Point corner;
	float h; // constant that defines roughnes. from interval (0,1)
	float textureScale;

	SmartPtr<PluggableShader> shader;
public:

	class Face;
	std::vector<Face> faces;
	
	//A face definition (triangle)
	class Face : public Primitive
	{
		FractalLandscape *m_fractal;
	public:
		//these indices point to vertices
		size_t vert1x, vert1y;
		size_t vert2x, vert2y;
		size_t vert3x, vert3y;
		// triangle normals
		size_t normal;

		Face(FractalLandscape * _obj) : m_fractal(_obj) {}

		virtual IntRet intersect(const Ray& _ray, float _previousBestDistance ) const;

		virtual BBox getBBox() const;

		virtual SmartPtr<Shader> getShader(IntRet _intData) const;
	};
	
	// Creates perturbated surface given by:
	// a, b are opposite cornes of a flat square (Z coordinate should be the samer)
	// _iterations - number of iterations
	// _h roughnes constant from interval (0,1)
	// shader -  shader is shared between faces. fractal does not support texture shader

	FractalLandscape(Point a, Point b, uint _iterations, float _h, SmartPtr<PluggableShader> _shader, float _textureScale) {
		//initialization
		corner = Point(std::min(a[0], b[0]), std::min(a[1], b[1]), a[2]);
		width = abs(a[0] - b[0]);
		number_of_squares_in_one_axis = pow(2, _iterations);
		number_of_vertices_in_one_axis = number_of_squares_in_one_axis + 1;
		heights = Array2<float>(number_of_vertices_in_one_axis);
		vertexNormals = Array2<Vector>(number_of_vertices_in_one_axis);
		squareNormals = Array2<std::pair<Vector, Vector> >(number_of_squares_in_one_axis);
		vertices = Array2<Point>(number_of_vertices_in_one_axis);
		heightsSet = Array2<int>(number_of_vertices_in_one_axis);
		textCoords = Array2<float2>(number_of_vertices_in_one_axis);
		one_square_width = width/number_of_squares_in_one_axis;
		iterations = _iterations;
		textureScale = _textureScale;
		h = _h;
		shader = _shader;
		
		//generating perturbated surface
		generateTextureCoordinates();
		resetHeights(0.0f);
		perturbateSurface(0, 0, number_of_vertices_in_one_axis - 1, number_of_vertices_in_one_axis - 1, width);
		generateNormals();
		generateVertexNormals();
		generateTriangles();
	}
	//Adds all triangles contained in this object to a scene
	void addReferencesToScene(std::vector<Primitive*> &_scene) const 
	{
		for(std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); it++)
			_scene.push_back((Primitive*)&*it);	
	}
protected:
	// sets texture coordinates of all vertices. scale is controlled by textureScale
	void generateTextureCoordinates() 
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				textCoords(x, y) = float2((float(x) / (float) (number_of_vertices_in_one_axis - 1) *textureScale),
						       (float(y) / (float) (number_of_vertices_in_one_axis - 1) *textureScale));

			}
	}
	
	// sets height in height map to h
	void resetHeights(float _h)
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				heights(x, y) = _h;
			}
	}
	
	// we recursively perturbate 5 points between input coordinates
	// if there are is no more points between, we terminate
	// we use Diamond-Square-like algorithm here
	// actual_square_width determines amount of perturbation - the bigger square the bigger pert.
	void perturbateSurface(uint x1, uint y1, uint x2, uint y2,  float actual_square_width)
	{
		uint midx = (x1 + x2) / 2;
		uint midy = (y1 + y2) / 2;
		// we got point in the middle
		if(midx > x1 && midy > y1) { // are there any points to perturbate?
			float diagonal_square_width = sqrt(2.0f) * actual_square_width;
			heights(midx, midy) = (heights(x1,y1) + heights(x2, y2)+ heights(x1,y2) + heights(x2, y1)  )/4.0 + h*normalRandom() * diagonal_square_width;
			
			//here in these conditionals we check whether we are not on berder of fractal. 
			// we don't want to perturbate edges
			// also we don't want to perturbate poins that were already perturbated 
			if(x1 != 0 && !heightsSet(x1, midy)) {
				heights(x1, midy) =  (heights(x1,y1) + heights(x1, y2) + heights(midx, midy))/3.0 + h*normalRandom() * actual_square_width;
				heightsSet(x1, midy) = 1;
			}
			if(x2 != (number_of_vertices_in_one_axis - 1)  && !heightsSet(x2, midy)) {
				heights(x2, midy) =  (heights(x2,y1) + heights(x2, y2)  + heights(midx, midy))/3.0 +   h*normalRandom() * actual_square_width;
				heightsSet(x2, midy) = 1;
			}
			if(y1 != 0  && !heightsSet(midx, y1)) {
				heights(midx, y1) =(heights(x1,y1) + heights(x2, y1) + heights(midx, midy))/3.0+  h*normalRandom() * actual_square_width;
				heightsSet(midx, y1) = 1;
			}
			if(y2 != (number_of_vertices_in_one_axis - 1) && !heightsSet(midx, y2)) {
				heights(midx, y2) =  (heights(x1,y2) + heights(x2, y2) + heights(midx, midy))/3.0 +   h*normalRandom() * actual_square_width;
				heightsSet(midx, y2) = 1;
			}
			actual_square_width = actual_square_width / 2.0f; 
			// recursive call on small regions
			perturbateSurface(x1,y1, midx, midy, actual_square_width);
			perturbateSurface(midx,y1, x2, midy, actual_square_width);
			perturbateSurface(x1,midy, midx, y2, actual_square_width);
			perturbateSurface(midx, midy, x2, y2, actual_square_width);
		}
	}
	
	// we iterate over all square and compute 2 triangle normals of each square
	void generateNormals() 
	{
		for(uint y = 0; y < number_of_squares_in_one_axis; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis ; x++) {
				Vector vx(one_square_width,
					 0,
					 heights(x + 1,y) - heights(x,y));
				Vector vy(0,
					 one_square_width,
					 heights(x,y + 1) - heights(x,y));
				Vector n = ~(vx % vy);
				squareNormals(x,y).first = n;

				vx = Vector(one_square_width,
					 0,
					 heights(x + 1,y + 1) - heights(x,y + 1));
				vy = Vector(0,
					 one_square_width,
					 heights(x + 1,y + 1) - heights(x + 1,y));
				n = ~(vx % vy);
				squareNormals(x,y).second = n;
				
			}
	}
	
	// by adding neighbour triangle normals we get vertex normal
	// If we are on the egde where is no triangle on a side, we add (0,0,1) instead
	void generateVertexNormals()
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				Vector n(0,0,0);
				//left-up
				if(x == 0 || y == number_of_squares_in_one_axis)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x - 1, y).first + squareNormals(x - 1, y).second;
				//right-up
				if(x == number_of_squares_in_one_axis || y == number_of_squares_in_one_axis)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x, y).first + squareNormals(x, y).second;				
				//right-down
				if(x == number_of_squares_in_one_axis || y == 0)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x, y - 1).first + squareNormals(x, y - 1).second;			
				//left-down
				if(x == 0 || y == 0)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x - 1, y - 1).first + squareNormals(x - 1, y -1).second;
				vertexNormals(x, y) = ~n;
			}		
	}
	
	// basically it generates final faces
	void generateTriangles()
	{

		// compute vertices by adding to corner point
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				vertices(x, y) = Point(corner[0] + (x / (float) (number_of_vertices_in_one_axis - 1)) * width,
						       corner[1] + (y / (float) (number_of_vertices_in_one_axis - 1)) * width, 
						       heights(x, y) + corner[2]);

			}
		// for easy indexing we copy face normals to an array
		int i = 0;
		for(uint y = 0; y < number_of_squares_in_one_axis; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis; x++) {	
				Face f1(this);
				Face f2(this);
				f1.vert1x = f2.vert1x = f2.vert2x = x;
				f1.vert1y = f2.vert1y = f1.vert2y = y;
				f1.vert3x = f2.vert3x = f1.vert2x = x + 1;
				f1.vert3y = f2.vert3y = f2.vert2y = y + 1;

				normals.push_back(squareNormals(x,y).first);
				f1.normal = i;
				faces.push_back(f1);
				i++;
				
				normals.push_back(squareNormals(x,y).second);
				f2.normal = i;
				faces.push_back(f2);
				i++;
			}
	}
	
	// generates number with gaussian propability distribution
	// Details: http://www.taygeta.com/random/gaussian.html
	float normalRandom() {
			const double PI = 3.141592;
		float r1 = rand() / (float)RAND_MAX;
		float r2 = rand() / (float)RAND_MAX;
		
		float y1 = sqrt( - 2 * log(r1) ) * cos( 2 * PI * r2 );
		//y2 = sqrt( - 2 ln(x1) ) sin( 2 pi x2 )
	 return y1;
	}
	

};

