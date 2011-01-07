#include "../core/algebra.h"
class FractalLandscape
{
	std::vector<float> heights;
	FractalLandscape(Point *a, Point *b, uint iterations) {
		
	}

	//Adds all triangles contained in this object to a scene
	void addReferencesToScene(std::vector<Primitive*> &_scene) const;

};