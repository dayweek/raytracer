#ifndef __INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016
#define __INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016
#ifdef _MSC_VER
	#pragma once
#endif

#include "../rt/shading_basics.h"
#include "../rt/texture.h"
#include "../core/algebra.h"

struct DefaultAmbientShader : public PluggableShader
{
	float4 ambientCoefficient;

	virtual float4 getAmbientCoefficient() const { return ambientCoefficient; }

	_IMPLEMENT_CLONE(DefaultAmbientShader);

	virtual ~DefaultAmbientShader() {}
};

//A base class for a phong shader. 
class PhongShaderBase : public PluggableShader
{
public:

	virtual void getCoeff(float4 &_diffuseCoef, float4 &_specularCoef, float &_specularExponent) const = 0;
	virtual Vector getNormal() const = 0;

	virtual float4 getReflectance(const Vector &_outDir, const Vector &_inDir) const
	{
		float4 Cs, Cd;
		float Ce;

		getCoeff(Cd, Cs, Ce);

		Vector normal = getNormal();
		Vector halfVect = ~(~_inDir + ~_outDir);
		float specCoeff = std::max(halfVect * normal, 0.f);
		specCoeff = exp(log(specCoeff) * Ce);	
		float diffCoeff = std::max(normal * ~_inDir, 0.f);

		return float4::rep(diffCoeff) * Cd + float4::rep(specCoeff) * Cs;
	}

	virtual ~PhongShaderBase() {}
};


//The default phong shader
class DefaultPhongShader : public PhongShaderBase
{
protected:
	Vector m_normal; //Stored normalized

public:
	float4 diffuseCoef; 
	float4 specularCoef; 
	float4 ambientCoef;
	float specularExponent;

	//Get the ambient coefficient for the material
	virtual float4 getAmbientCoefficient() const { return ambientCoef; }
	virtual void getCoeff(float4 &_diffuseCoef, float4 &_specularCoef, float &_specularExponent) const
	{
		_diffuseCoef = diffuseCoef;
		_specularCoef = specularCoef;
		_specularExponent = specularExponent;
	}

	virtual Vector getNormal() const { return m_normal;}
	virtual void setNormal(const Vector& _normal) { m_normal = ~_normal;}

	_IMPLEMENT_CLONE(DefaultPhongShader);

};

//A phong shader that supports texturing
class TexturedPhongShader : public DefaultPhongShader
{
protected:
	float2 m_texCoord; 
public:
	SmartPtr<Texture> diffTexture;
	SmartPtr<Texture> amibientTexture;
	SmartPtr<Texture> specTexture;
	

	virtual void setTextureCoord(const float2& _texCoord) { m_texCoord = _texCoord;}

	virtual float4 getAmbientCoefficient() const 
	{ 
		float4 ret = DefaultPhongShader::getAmbientCoefficient();

// 		if(amibientTexture.data() != NULL)
// 			ret = amibientTexture->sample(m_texCoord);

		return ret;
	}

	virtual void getCoeff(float4 &_diffuseCoef, float4 &_specularCoef, float &_specularExponent) const
	{
		DefaultPhongShader::getCoeff(_diffuseCoef, _specularCoef, _specularExponent);
/*
		if(diffTexture.data() != NULL)
			_diffuseCoef = diffTexture->sample(m_texCoord);

		if(specTexture.data() != NULL)
			_specularCoef = specTexture->sample(m_texCoord);*/
	}

	
	_IMPLEMENT_CLONE(TexturedPhongShader);
};



class BumpTexturePhongShader : public TexturedPhongShader
{
public:
	SmartPtr<Texture> bumpTexture;
	Vector pu, pv;
	virtual Vector getNormal() const 
	{ 
		Vector ret = m_normal;
		if(bumpTexture.data() != NULL) 
		{
			// we modify the normal with respect to the bumpTexture
			
// 			std::cout << 1 << " " <<  m_texCoord.x << " " << m_texCoord.y << std::endl;
			Vector a = pu % m_normal;
			Vector b = pv % m_normal;
			
			float2 der = bumpTexture->derivatives(m_texCoord);
			return ~(m_normal + (der.x * a - der.y * b));
		}
		return ret;
	}
	virtual void setPuPv(Point x, Point y, Point z, float2 u, float2 v, float2 w)
	{ 
		Vector vec[3];
		vec[0] = Vector(u.x, u.y, 1.0f);
		vec[1] = Vector(v.x, v.y, 1.0f);
		vec[2] = Vector(w.x, w.y, 1.0f);
		Matrix m(vec);
		m = m.inverse();
		
		vec[0] = Vector(x[0], x[1], x[2]);
		vec[1] = Vector(y[0], y[1], y[2]);
		vec[2] = Vector(z[0], z[1], z[2]);
		Matrix wor(vec);
		
		Matrix pupv = wor * m;
		pu = pupv.v[0];
		pu = pupv.v[1];
	}		
	_IMPLEMENT_CLONE(BumpTexturePhongShader);
};

class RRPhongShader : public DefaultPhongShader
{
public:
	float n1, n2;
	Point m_position;

	
	// _out == -ray.d
	
	virtual float4 getIndirectRadiance(const Vector &_out, Integrator *_integrator) const
	{
		float4 color = float4::rep(0.0f);
		Vector normal = getNormal();
		// if we hit the surface from inside we swap the n1,n2 and invert normal
		bool front = true;
		float nn2 = n2;
		float nn1 = n1;
		float nn = nn1/nn2;
		if(normal * (-_out) > 0.0f) {
			normal = -normal;
			nn1 = n2;
			nn2 = n1;
			nn = nn1/nn2;
			front = false;
		}
		// compute refection coeficient
		float cosI =  normal * (_out);
		float sinT2 = nn * nn * (1.0f - cosI * cosI);
		float4 reflCoef = float4::rep(0.0);
		if(sinT2 > 1.0) 
			reflCoef = float4::rep(1.0);
		else {
			float cosT = sqrt(1 - sinT2);
			float NOrth = (nn1 * cosI - nn2 * cosT) / (nn1 * cosI + nn2 * cosT);
			float Rpar =  (nn2 * cosI - nn1 * cosT) / (nn2 * cosI + nn1 * cosT);
			float R = (NOrth * NOrth + Rpar * Rpar) / 2.0f;
			reflCoef = float4::rep(R);	
		}
		// first get reflected light
		Ray newray;
		newray.d = ~(- _out - 2 * cosI * normal);
		newray.o = m_position + newray.d;

		color = reflCoef * _integrator->getRadiance(newray);
		if(sinT2 <= 1.0) {
			float cosT = sqrt(1 - sinT2);
			newray.d  = nn*(-_out) + (((nn * cosI) - cosT) * normal);
			//we enter object 
// 			if(front) {
// 				_integrator->terminate = true;
// 				_integrator->count = 6;
// 			}
			color = color + ((float4::rep(1.0f) - reflCoef) * _integrator->getRadiance(newray));
		}
		return color;
	}
	

	
	virtual void setPosition(const Point& _point) { m_position = _point; }

	_IMPLEMENT_CLONE(RRPhongShader);
};

#endif //__INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016