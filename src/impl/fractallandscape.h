#include "../core/algebra.h"
#include "../core/array2.h"
#include "../rt/basic_definitions.h"
#include "../rt/bvh.h"
#include "../rt/shading_basics.h"
#include "../rt/texture.h"

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
public:
	Array2<float> heights;
	Array2<Vector> squareNormals;
	Array2<Vector> vertexNormals;
	Array2<int> heightsSet;
	std::vector<Vector> normals;
	Array2<Point> vertices;
	uint iterations;
	float width;
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	float one_square_width;
	Point corner;
	float h;

	
	//A face definition (triangle)
	class Face : public Primitive
	{
		FractalLandscape *m_fractal;
	public:
		size_t vert1x, vert1y, tex1, norm1x, norm1y;
		size_t vert2x, vert2y, tex2, norm2x, norm2y;
		size_t vert3x, vert3y, tex3, norm3x, norm3y;
		size_t normal;

		Face(FractalLandscape * _obj) : m_fractal(_obj) {}

		virtual IntRet intersect(const Ray& _ray, float _previousBestDistance ) const;

		virtual BBox getBBox() const;

		virtual SmartPtr<Shader> getShader(IntRet _intData) const;
	};
	
	typedef std::vector<Face> t_faceVector;
	typedef std::vector<float2> t_texCoordVector;

	t_faceVector faces;
	t_texCoordVector texCoords;
	SmartPtr<PluggableShader> shader;
	
	FractalLandscape(Point a, Point b, uint _iterations, float _h) {
		//init
		srand((unsigned)time(0)); 
		corner = Point(std::min(a[0], b[0]), std::min(a[1], b[1]), a[2]);
		width = abs(a[0] - b[0]);
		number_of_squares_in_one_axis = pow(2, _iterations);
		number_of_vertices_in_one_axis = number_of_squares_in_one_axis + 1;
		heights = Array2<float>(number_of_vertices_in_one_axis);
		vertexNormals = Array2<Vector>(number_of_vertices_in_one_axis);
		squareNormals = Array2<Vector>(number_of_squares_in_one_axis);
		vertices = Array2<Point>(number_of_vertices_in_one_axis);
		heightsSet = Array2<int>(number_of_vertices_in_one_axis);
		one_square_width = width/number_of_squares_in_one_axis;
		iterations = _iterations;
		h = _h;
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
	void resetHeights(float h)
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				heights(x, y) = h;
				heightsSet(x,y) = 0;
			}
	}
	
	void perturbateSurface(uint x1, uint y1, uint x2, uint y2,  float actual_square_width)
	{
		uint midx = (x1 + x2) / 2;
		uint midy = (y1 + y2) / 2;
		if(midx > x1 && midy > y1) {
			float diagonal_square_width = sqrt(2.0f) * actual_square_width;
			heights(midx, midy) = (heights(x1,y1) + heights(x2, y2)+ heights(x1,y2) + heights(x2, y1)  )/4.0 + h*normalRandom() * diagonal_square_width;
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
			perturbateSurface(x1,y1, midx, midy, actual_square_width);
			perturbateSurface(midx,y1, x2, midy, actual_square_width);
			perturbateSurface(x1,midy, midx, y2, actual_square_width);
			perturbateSurface(midx, midy, x2, y2, actual_square_width);
		}
		else return;
	}
	
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
				float vyska = n[2];
				squareNormals(x,y) = n;
				
			}
	}
	
	void generateVertexNormals()
	{
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				Vector n(0,0,0);
				//left-up
				if(x == 0 || y == number_of_squares_in_one_axis)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x - 1, y);
				//right-up
				if(x == number_of_squares_in_one_axis || y == number_of_squares_in_one_axis)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x, y);				
				//right-down
				if(x == number_of_squares_in_one_axis || y == 0)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x, y - 1);			
				//left-down
				if(x == 0 || y == 0)
					n = n + Vector(0,0,1);
				else
					n = n + squareNormals(x - 1, y - 1);
				vertexNormals(x, y) = ~n;
			}		
	}
	
	void generateTriangles()
	{
		float hh;
		float c;
		// compute vertices
		for(uint y = 0; y < number_of_vertices_in_one_axis; y++) 
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) {
				vertices(x, y) = Point(corner[0] + (x / (float) (number_of_vertices_in_one_axis - 1)) * width,
						       corner[1] + (y / (float) (number_of_vertices_in_one_axis - 1)) * width, 
						       heights(x, y) + corner[2]);

			}
		int i = 0;
		for(uint y = 0; y < number_of_squares_in_one_axis; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis; x++) {	
				Face f1(this);
				Face f2(this);
				f1.norm1x = f1.vert1x = f2.norm1x = f2.vert1x = f2.norm2x = f2.vert2x = x;
				f1.norm1y = f1.vert1y = f2.norm1y = f2.vert1y = f1.norm2y = f1.vert2y = y;
				f1.norm3x = f1.vert3x = f2.norm3x = f2.vert3x = f1.norm2x = f1.vert2x = x + 1;
				f1.norm3y = f1.vert3y = f2.norm3y = f2.vert3y = f2.norm2y = f2.vert2y = y + 1;

				normals.push_back(squareNormals(x,y));
				f1.normal = i;

				faces.push_back(f1);

				f2.normal = i;
				faces.push_back(f2);
				i++;
			}
	}
	
	float normalRandom() {
	const double PI = 3.141592;
	 float r1 = rand() / (float)RAND_MAX;
	 float r2 = rand() / (float)RAND_MAX;
	 
	 float y1 = sqrt( - 2 * log(r1) ) * cos( 2 * PI * r2 );
//          y2 = sqrt( - 2 ln(x1) ) sin( 2 pi x2 )
	 return y1;

	}
	

};

