#pragma once

#include "Quadtree.hpp"
#include "LightPointEmission.hpp"
#include "LightDirectionEmission.hpp"
#include "LightShape.hpp"

namespace lum
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
		LightSystem();

        void create(const sf::FloatRect& rootRegion, const sf::Vector2u& imageSize, const sf::Texture& penumbraTexture, sf::Shader& unshadowShader, sf::Shader& lightOverShapeShader);

        void render(sf::RenderTarget& target);

		LightShape::Ptr createLightShape();
		void removeShape(LightShape::Ptr shape);

		LightPointEmission::Ptr createLightPointEmission();
		void removeLight(LightPointEmission::Ptr light);

		LightDirectionEmission::Ptr createLightPointDirection();
        void removeLight(LightDirectionEmission::Ptr light);

		void trimLightPointEmissionQuadtree();
		void trimShapeQuadtree();

		void updateTextureSize(sf::Vector2u const& size);

	private:
		static void getPenumbrasPoint(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const sf::ConvexShape &shape, const sf::Vector2f &sourceCenter, float sourceRadius);
		static void getPenumbrasDirection(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const sf::ConvexShape &shape, const sf::Vector2f &sourceDirection, float sourceRadius, float sourceDistance);

	private:
		sf::Shader* mUnshadowShader;
		sf::Shader* mLightOverShapeShader;

		DynamicQuadtree mShapeQuadtree;
		DynamicQuadtree mLightPointEmissionQuadtree;

		std::unordered_set<LightPointEmission::Ptr> mPointEmissionLights;
		std::unordered_set<LightDirectionEmission::Ptr> mDirectionEmissionLights;
		std::unordered_set<LightShape::Ptr> mLightShapes;

		sf::RenderTexture mLightTempTexture;
		sf::RenderTexture mEmissionTempTexture;
		sf::RenderTexture mAntumbraTempTexture;
		sf::RenderTexture mCompositionTexture;

		float mDirectionEmissionRange;
		float mDirectionEmissionRadiusMultiplier;
		sf::Color mAmbientColor;

	private:
		friend class LightPointEmission;
		friend class LightDirectionEmission;
		friend class LightShape;
};

} // namespace lum
