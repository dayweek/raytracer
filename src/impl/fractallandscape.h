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
	float width, height;
	uint number_of_squares_in_one_axis;
	uint number_of_vertices_in_one_axis;
	float one_square_width;
	Point corner;
	

	
	//A face definition (triangle)
	class Face : public Primitive
	{
		FractalLandscape *m_fractal;
	public:
		size_t vert1, tex1, norm1;
		size_t vert2, tex2, norm2;
		size_t vert3, tex3, norm3;

		Face(FractalLandscape * _obj) : m_fractal(_obj) {}

		virtual IntRet intersect(const Ray& _ray, float _previousBestDistance ) const;

		virtual BBox getBBox() const;

		virtual SmartPtr<Shader> getShader(IntRet _intData) const;
	};
	
	typedef std::vector<Point> t_pointVector;
	typedef std::vector<Vector> t_vectVector;
	typedef std::vector<Face> t_faceVector;
	typedef std::vector<float2> t_texCoordVector;

	t_pointVector vertices;
	t_vectVector normals;
	t_faceVector faces;
	t_texCoordVector texCoords;
	SmartPtr<PluggableShader> shader;
	
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
	//Adds all triangles contained in this object to a scene
	void addReferencesToScene(std::vector<Primitive*> &_scene) const;
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
	void generateTriangles();
	

};

