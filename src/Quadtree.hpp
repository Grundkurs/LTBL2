#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "Math.hpp"

#include <array>
#include <list>
#include <memory>
#include <unordered_set>

namespace lum
{

class QuadtreeOccupant
{
	public:
		QuadtreeOccupant();

		void quadtreeUpdate();
		void quadtreeRemove();

		virtual sf::FloatRect getAABB() const = 0;

		void setAwake(bool awake);
		bool isAwake() const;
		void toggleAwake();

		void setTurnedOn(bool turnedOn);
		bool isTurnedOn() const;
		void toggleTurnedOn();

	private:
		class QuadtreeNode* mQuadtreeNode;
		class Quadtree* mQuadtree;

		bool mAwake;
		bool mTurnedOn;

	private:
		friend class Quadtree;
		friend class QuadtreeNode;
		friend class DynamicQuadtree;
		friend class StaticQuadtree;
};

class QuadtreeNode : public sf::NonCopyable
{
	public:
		QuadtreeNode();
		QuadtreeNode(const sf::FloatRect &region, int level, QuadtreeNode* pParent, class Quadtree* pQuadtree);

		void create(const sf::FloatRect &region, int level, QuadtreeNode* pParent, class Quadtree* pQuadtree);

		class Quadtree* getTree() const;

		void add(QuadtreeOccupant* oc);

		const sf::FloatRect& getRegion() const;

		void getAllOccupantsBelow(std::vector<QuadtreeOccupant*>& occupants);
		void getAllOccupantsBelow(std::unordered_set<QuadtreeOccupant*>& occupants);

		int getNumOccupantsBelow() const;

		bool hasChildren() const;

	private:
		void getPossibleOccupantPosition(QuadtreeOccupant* oc, sf::Vector2i& point);

		void addToThisLevel(QuadtreeOccupant* oc);

		// Returns true if occupant was added to children
		bool addToChildren(QuadtreeOccupant* oc);

		void destroyChildren();

		void getOccupants(std::unordered_set<QuadtreeOccupant*> &occupants);

		void partition();

		void merge();

		void update(QuadtreeOccupant* oc);
		void remove(QuadtreeOccupant* oc);

		void removeForDeletion(std::unordered_set<QuadtreeOccupant*> &occupants);

	private:
		QuadtreeNode* mParent;
		class Quadtree* mQuadtree;

		bool mHasChildren;

		std::array<std::unique_ptr<QuadtreeNode>, 4> mChildren;
		std::unordered_set<QuadtreeOccupant*> mOccupants;

		sf::FloatRect mRegion;

		int mLevel;

		int mNumOccupantsBelow = 0;

	private:
		friend class QuadtreeOccupant;
		friend class Quadtree;
		friend class DynamicQuadtree;
};

// Base class for dynamic and static Quadtree types
class Quadtree 
{
	public:
		Quadtree();
		Quadtree(Quadtree const& other);

		virtual ~Quadtree() {}

		void operator=(Quadtree const& other);

		void queryRegion(std::vector<QuadtreeOccupant*>& result, sf::FloatRect const& region);
		void queryPoint(std::vector<QuadtreeOccupant*>& result, sf::Vector2f const& p);
		void queryShape(std::vector<QuadtreeOccupant*>& result, sf::ConvexShape const& shape);

		size_t getMinNumNodeOccupants() const;
		size_t getMaxNumNodeOccupants() const;
		size_t getMaxLevels() const;
		float getOversizeMultiplier() const;

		void clear();
		void create(sf::FloatRect const& rootRegion);
		bool created() const;
		virtual void add(QuadtreeOccupant* oc) = 0;
		const sf::FloatRect getRootRegion() const;

	protected:
		// Called whenever something is removed, an action can be defined by derived classes, defaults to doing nothing
		virtual void onRemoval();

		void setQuadtree(QuadtreeOccupant* oc);

		void recursiveCopy(QuadtreeNode* thisNode, QuadtreeNode* otherNode, QuadtreeNode* thisParent);

	protected:
		std::unordered_set<QuadtreeOccupant*> mOutsideRoot;

		std::unique_ptr<QuadtreeNode> mRootNode;
		size_t mMinNumNodeOccupants;
		size_t mMaxNumNodeOccupants;
		size_t mMaxLevels;
		float mOversizeMultiplier;

	private:
		friend class QuadtreeOccupant;
		friend class QuadtreeNode;
};

class StaticQuadtree : public Quadtree
{
	public:
		StaticQuadtree();
		StaticQuadtree(sf::FloatRect const& rootRegion);

		void add(QuadtreeOccupant* oc);
};

class DynamicQuadtree : public Quadtree
{
	public:
		DynamicQuadtree();
		DynamicQuadtree(sf::FloatRect const& rootRegion);

		void add(QuadtreeOccupant* oc);

		void trim();

	private:
		void expand();
		void contract();

	private:
		size_t mMinOutsideRoot;
		size_t mMaxOutsideRoot;
};

} // namespace lum