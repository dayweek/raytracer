#ifndef __INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016
#define __INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016
#ifdef _MSC_VER
	#pragma once
#endif

#include "../rt/shading_basics.h"
#include "../rt/texture.h"

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

		if(amibientTexture.data() != NULL)
			ret = amibientTexture->sample(m_texCoord);

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
		pu = Vector(1,0,0);
		pu = Vector(0,1,0);
	}
/*	virtual float4 getAmbientCoefficient() const 
	{ 
		float4 ret = DefaultPhongShader::getAmbientCoefficient();

		if(bumpTexture.data() != NULL) {
// 			std::cout << "ok" << std::endl;
			ret = bumpTexture->sample(m_texCoord);
		}

		return ret;
	}*/		
	_IMPLEMENT_CLONE(BumpTexturePhongShader);
};

#endif //__INCLUDE_GUARD_810F2AF5_7E81_4F1E_AA05_992B6D2C0016