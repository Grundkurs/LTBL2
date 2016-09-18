#pragma once

#include "Utils.hpp"
#include "LightPointEmission.hpp"
#include "LightDirectionEmission.hpp"
#include "LightShape.hpp"
#include "LightResources.hpp"

namespace ltbl
{

class LightSystem : sf::NonCopyable
{
	public:
		struct Penumbra
		{
			sf::Vector2f _source;
			sf::Vector2f _lightEdge;
			sf::Vector2f _darkEdge;
			float _lightBrightness;
			float _darkBrightness;
			float _distance;
		};

    public:
		// Ctor
		LightSystem();

		// Create quadtrees, resources and render textures
        void create(const sf::FloatRect& rootRegion, const sf::Vector2u& imageSize);

		// Render the lights
        void render(sf::RenderTarget& target);

		// Create a light shape
		LightShape* createLightShape();

		// Remove a light shape
		void removeShape(LightShape* shape);

		// Create a light point
		LightPointEmission* createLightPointEmission();

		// Remove a light point
		void removeLight(LightPointEmission* light);

		// Create a light direction
		LightDirectionEmission* createLightDirectionEmission();

		// Remove a light direction
		void removeLight(LightDirectionEmission* light);

		// Set ambient color
		void setAmbientColor(const sf::Color& color);

		// Get ambient color
		const sf::Color& getAmbientColor() const;

		// Update shader texture and the size of render texture
		// Note : Call it only if you change the penumbra texture or shaders, render texture size are automatically updated
		void update(const sf::Vector2u& size = sf::Vector2u());

		// Get the penumbra texture
		// Note : As penumbra texture if automatically loaded, use it only to change the texture
		// Note : You have to call update(), right after you changed it
		// Note : Don't forget to smooth the texture after changing it
		sf::Texture& getPenumbraTexture();

		// Get the unshadow shader
		// Note : As unshadow shader if automatically loaded, use it only to change the shader
		// Note : You have to call update(), right after you changed it
		sf::Shader& getUnshadowShader();

		// Get the light over shape shader
		// Note : As light over shape shader if automatically loaded, use it only to change the shader
		// Note : You have to call update(), right after you changed it
		sf::Shader& getLightOverShapeShader();

		// TODO : Check this function
		static void getPenumbrasPoint(std::vector<Penumbra>& penumbras, std::vector<int>& innerBoundaryIndices, std::vector<sf::Vector2f>& innerBoundaryVectors, std::vector<int>& outerBoundaryIndices, std::vector<sf::Vector2f>& outerBoundaryVectors, const LightShape& shape, const sf::Vector2f& sourceCenter, float sourceRadius);
		
		// TODO : Check this function
		static void getPenumbrasDirection(std::vector<Penumbra>& penumbras, std::vector<int>& innerBoundaryIndices, std::vector<sf::Vector2f>& innerBoundaryVectors, std::vector<int>& outerBoundaryIndices, std::vector<sf::Vector2f>& outerBoundaryVectors, const LightShape& shape, const sf::Vector2f& sourceDirection, float sourceRadius, float sourceDistance);

	private:
		sf::Texture mPenumbraTexture;
		sf::Shader mUnshadowShader;
		sf::Shader mLightOverShapeShader;

		priv::Quadtree mLightShapeQuadtree;
		priv::Quadtree mLightPointEmissionQuadtree;

		std::unordered_set<LightPointEmission*> mPointEmissionLights;
		std::unordered_set<LightDirectionEmission*> mDirectionEmissionLights;
		std::unordered_set<LightShape*> mLightShapes;

		sf::RenderTexture mLightTempTexture;
		sf::RenderTexture mEmissionTempTexture;
		sf::RenderTexture mAntumbraTempTexture;
		sf::RenderTexture mCompositionTexture;

		float mDirectionEmissionRange;
		float mDirectionEmissionRadiusMultiplier;
		sf::Color mAmbientColor;
};

} // namespace ltbl
