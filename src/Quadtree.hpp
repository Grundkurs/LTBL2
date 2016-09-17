#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <cassert>
#include <unordered_set>

namespace ltbl
{

class QuadtreeOccupant
{
	public: 
		QuadtreeOccupant();

		void setAwake(bool awake);
		bool isAwake() const;
		void toggleAwake();

		virtual sf::FloatRect getAABB() const = 0;
		void quadtreeAABBChanged();

	private:
		bool mAwake;
		bool mAABBChanged;

	private:
		friend class Quadtree;
};

class Quadtree : sf::NonCopyable, public sf::Drawable
{
	public:
		Quadtree(const sf::FloatRect& region, unsigned int maxOccupants = 5, unsigned int maxLevels = 5, Quadtree* parent = nullptr, unsigned int level = 0, unsigned int type = 0);

		void create(const sf::FloatRect& region, unsigned int maxOccupants = 5, unsigned int maxLevels = 5, Quadtree* parent = nullptr, unsigned int level = 0, unsigned int type = 0);

		void addOccupant(QuadtreeOccupant* oc);
		bool removeOccupant(QuadtreeOccupant* oc);

		bool update();

		void clear();

		void query(const sf::FloatRect& area, std::vector<QuadtreeOccupant*>& occupants);
		void query(const sf::Vector2f& point, std::vector<QuadtreeOccupant*>& occupants);
		void query(const sf::ConvexShape& shape, std::vector<QuadtreeOccupant*>& occupants);

	private:
		void split();
		void unsplit();

		unsigned int getNumOccupantsBelow() const;

		bool hasChildren() const;

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		static sf::ConvexShape shapeFromRect(const sf::FloatRect& rect);
		static bool shapeIntersection(const sf::ConvexShape& shapeA, const sf::ConvexShape& shapeB);

	private:
		sf::FloatRect mRegion;
		unsigned int mMaxOccupants;
		unsigned int mMaxLevels;
		Quadtree* mParent;
		unsigned int mLevel;
		unsigned int mType;

		std::unordered_set<QuadtreeOccupant*> mOccupants;
		std::unordered_set<QuadtreeOccupant*> mOutsideOccupants;

		std::vector<Quadtree*> mChildren;
};

} // namespace ltbl