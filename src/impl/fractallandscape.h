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
	Array2<Point> vertices;
	float width;
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	float one_square_width;
	Point corner;
	

	
	//A face definition (triangle)
	class Face : public Primitive
	{
		FractalLandscape *m_fractal;
	public:
		size_t vert1x, vert1y, tex1, norm1x, norm1y;
		size_t vert2x, vert2y, tex2, norm2x, norm2y;
		size_t vert3x, vert3y, tex3, norm3x, norm3y;

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
	
	FractalLandscape(Point a, Point b, uint iterations) {
		//init
		corner = Point(std::min(a[0], b[0]), std::min(a[1], b[1]), a[2]);
		width = abs(a[0] - b[0]);
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
			for(uint x = 0; x < number_of_vertices_in_one_axis; x++) 
				heights(x, y) = h;
	}
	
	void perturbateSurface()
	{
	}
	
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
				squareNormals(x,y) = ~(vx % vy);
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
				n = ~n;
			}		
	}
	
	void generateTriangles()
	{
		// compute vertices
		for(uint y = 0; y < number_of_squares_in_one_axis; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis; x++)
				vertices(x, y) = Point(one_square_width * x, one_square_width * y, heights(x, y));

		for(uint y = 0; y < number_of_squares_in_one_axis; y++) 
			for(uint x = 0; x < number_of_squares_in_one_axis; x++) {	
				Face f1(this);
				Face f2(this);
				f1.norm1x = f1.vert1x = f2.norm1x = f2.vert1x = f1.norm2x = f1.vert2x = x;
				f1.norm1y = f1.vert1y = f2.norm1y = f2.vert1y = f1.norm2y = f1.vert2y = y;
				f1.norm3x = f1.vert3x = f2.norm3x = f2.vert3x = f1.norm2x = f1.vert2x = x + 1;
				f1.norm3y = f1.vert3y = f2.norm3y = f2.vert3y = f1.norm2y = f1.vert2y = y + 1;
				faces.push_back(f1);
				faces.push_back(f2);
			}
	}
	

};

