#include "Quadtree.hpp"

#include <algorithm>
#include <assert.h>

namespace ltbl
{

QuadtreeOccupant::QuadtreeOccupant() 
	: mQuadtreeNode(nullptr)
	, mQuadtree(nullptr)
	, mAwake(true)
	, mTurnedOn(true)
{
}

void QuadtreeOccupant::quadtreeUpdate()
{
	if (mQuadtreeNode != nullptr)
	{
		mQuadtreeNode->update(this);
	}
	else
	{
		mQuadtree->mOutsideRoot.erase(this);
		mQuadtree->add(this);
	}
}

void QuadtreeOccupant::quadtreeRemove()
{
	if (mQuadtreeNode != nullptr)
	{
		mQuadtreeNode->remove(this);
	}
	else
	{
		mQuadtree->mOutsideRoot.erase(this);
	}
}

void QuadtreeOccupant::setAwake(bool awake)
{
	mAwake = awake;
}

bool QuadtreeOccupant::isAwake() const
{
	return mAwake;
}

void QuadtreeOccupant::toggleAwake()
{
	mAwake = !mAwake;
}

void QuadtreeOccupant::setTurnedOn(bool turnedOn)
{
	mTurnedOn = turnedOn;
}

bool QuadtreeOccupant::isTurnedOn() const
{
	return mTurnedOn;
}

void QuadtreeOccupant::toggleTurnedOn()
{
	mTurnedOn = !mTurnedOn;
}

QuadtreeNode::QuadtreeNode()
	: mHasChildren(false)
	, mNumOccupantsBelow(0)
	, mParent(nullptr)
	, mQuadtree(nullptr)
	, mRegion(sf::FloatRect())
	, mLevel(0)
{
}

QuadtreeNode::QuadtreeNode(const sf::FloatRect &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree)
	: mHasChildren(false)
	, mNumOccupantsBelow(0)
	, mParent(pParent)
	, mQuadtree(pQuadtree)
	, mRegion(region)
	, mLevel(level)
{
}

void QuadtreeNode::create(const sf::FloatRect &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree)
{
	mHasChildren = false;
	mRegion = region;
	mLevel = level;
	mParent = pParent;
	mQuadtree = pQuadtree;
}

Quadtree* QuadtreeNode::getTree() const
{
	return mQuadtree;
}

void QuadtreeNode::add(QuadtreeOccupant* oc)
{
	assert(oc != nullptr);

	mNumOccupantsBelow++;

	// See if the occupant fits into any children (if there are any)
	if (hasChildren())
	{
		if (addToChildren(oc))
		{
			return;
		}
	}
	else
	{
		// Check if we need a new partition
		if (mOccupants.size() >= mQuadtree->getMaxNumNodeOccupants() && static_cast<size_t>(mLevel) < mQuadtree->getMaxLevels())
		{
			partition();
			if (addToChildren(oc))
			{
				return;
			}
		}
	}

	// Did not fit in anywhere, add to this level, even if it goes over the maximum size
	addToThisLevel(oc);
}

const sf::FloatRect& QuadtreeNode::getRegion() const
{
	return mRegion;
}

void QuadtreeNode::getAllOccupantsBelow(std::vector<QuadtreeOccupant*>& occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;
	open.push_back(this);

	while (!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++)
		{
			if ((*it) != nullptr)
			{
				// Add to this node
				occupants.push_back(*it);
			}
		}

		// If the node has children, add them to the open list
		if (pCurrent->hasChildren())
		{
			for (int i = 0; i < 4; i++)
			{
				open.push_back(pCurrent->mChildren[i].get());
			}
		}
	}
}

void QuadtreeNode::getAllOccupantsBelow(std::unordered_set<QuadtreeOccupant*>& occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;
	open.push_back(this);

	while (!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++)
		{
			if ((*it) == nullptr)
			{
				// Add to this node
				occupants.insert(*it);
			}
		}

		// If the node has children, add them to the open list
		if (pCurrent->hasChildren())
		{
			for (int i = 0; i < 4; i++)
			{
				open.push_back(pCurrent->mChildren[i].get());
			}
		}
	}
}

int QuadtreeNode::getNumOccupantsBelow() const
{
	return mNumOccupantsBelow;
}

bool QuadtreeNode::hasChildren() const
{
	return mHasChildren;
}

void QuadtreeNode::getPossibleOccupantPosition(QuadtreeOccupant* oc, sf::Vector2i& point)
{
	// Compare the center of the AABB of the occupant to that of this node to determine
	// which child it may (possibly, not certainly) fit in
	const sf::Vector2f &occupantCenter = rectCenter(oc->getAABB());
	const sf::Vector2f &nodeRegionCenter = rectCenter(mRegion);

	point.x = occupantCenter.x > nodeRegionCenter.x ? 1 : 0;
	point.y = occupantCenter.y > nodeRegionCenter.y ? 1 : 0;
}

void QuadtreeNode::addToThisLevel(QuadtreeOccupant* oc)
{
	if (mOccupants.find(oc) != mOccupants.end())
	{
		return;
	}
	mOccupants.insert(oc);
	oc->mQuadtreeNode = this;
}

bool QuadtreeNode::addToChildren(QuadtreeOccupant* oc)
{
	assert(hasChildren());

	sf::Vector2i position;

	getPossibleOccupantPosition(oc, position);

	QuadtreeNode* pChild = mChildren[position.x + position.y * 2].get();

	// See if the occupant fits in the child at the selected position
	if (rectContains(pChild->mRegion, oc->getAABB()))
	{
		// Fits, so can add to the child and finish
		pChild->add(oc);
		return true;
	}
	return false;
}

void QuadtreeNode::destroyChildren()
{
	for (int i = 0; i < 4; i++)
	{
		mChildren[i].reset();
	}
	mHasChildren = false;
}

void QuadtreeNode::getOccupants(std::unordered_set<QuadtreeOccupant*>& occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;
	open.push_back(this);

	while (!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++)
		{
			if ((*it) != nullptr)
			{
				// Assign new node
				(*it)->mQuadtreeNode = this;

				// Add to this node
				occupants.insert(*it);
			}
		}

		// If the node has children, add them to the open list
		if (pCurrent->hasChildren())
		{
			for (int i = 0; i < 4; i++)
			{
				open.push_back(pCurrent->mChildren[i].get());
			}
		}
	}
}

void QuadtreeNode::partition()
{
	assert(!hasChildren());

	sf::Vector2f halfRegionDims = rectHalfDims(mRegion);
	sf::Vector2f regionLowerBound = rectLowerBound(mRegion);
	sf::Vector2f regionCenter = rectCenter(mRegion);

	int nextLowerLevel = mLevel - 1;

	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			sf::Vector2f offset(x * halfRegionDims.x, y * halfRegionDims.y);

			sf::FloatRect childAABB = rectFromBounds(regionLowerBound + offset, regionCenter + offset);

			// Scale up AABB by the oversize multiplier
			sf::Vector2f newHalfDims = rectHalfDims(childAABB);
			sf::Vector2f center = rectCenter(childAABB);
			childAABB = rectFromBounds(center - newHalfDims, center + newHalfDims);

			mChildren[x + y * 2].reset(new QuadtreeNode(childAABB, nextLowerLevel, this, mQuadtree));
		}
	}

	mHasChildren = true;
}

void QuadtreeNode::merge()
{
	if (hasChildren())
	{
		// Place all occupants at lower levels into this node
		getOccupants(mOccupants);
		destroyChildren();
	}
}

void QuadtreeNode::update(QuadtreeOccupant* oc)
{
	if (oc == nullptr)
	{
		return;
	}

	if (!mOccupants.empty())
	{
		// Remove, may be re-added to this node later
		mOccupants.erase(oc);
	}

	// Propogate upwards, looking for a node that has room (the current one may still have room)
	QuadtreeNode* pNode = this;

	while (pNode != nullptr)
	{
		pNode->mNumOccupantsBelow--;

		// If has room for 1 more, found a spot
		if (rectContains(pNode->mRegion, oc->getAABB()))
		{
			break;
		}

		pNode = pNode->mParent;
	}

	// If no node that could contain the occupant was found, add to outside root set
	if (pNode == nullptr)
	{
		assert(mQuadtree != nullptr);

		if (mQuadtree->mOutsideRoot.find(oc) != mQuadtree->mOutsideRoot.end())
		{
			return;
		}

		mQuadtree->mOutsideRoot.insert(oc);

		oc->mQuadtreeNode = nullptr;
	}
	else // Add to the selected node
	{
		pNode->add(oc);
	}
}

void QuadtreeNode::remove(QuadtreeOccupant* oc)
{
	assert(!mOccupants.empty());

	// Remove from node
	mOccupants.erase(oc);

	if (oc == nullptr)
	{
		return;
	}

	// Propogate upwards, merging if there are enough occupants in the node
	QuadtreeNode* pNode = this;

	while (pNode != nullptr)
	{
		pNode->mNumOccupantsBelow--;

		if (pNode->mNumOccupantsBelow > 0 && static_cast<size_t>(pNode->mNumOccupantsBelow) >= mQuadtree->getMinNumNodeOccupants())
		{
			pNode->merge();
			break;
		}

		pNode = pNode->mParent;
	}
}

void QuadtreeNode::removeForDeletion(std::unordered_set<QuadtreeOccupant*>& occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;
	open.push_back(this);

	while (!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++)
		{
			if ((*it) != nullptr)
			{
				// Since will be deleted, remove the reference
				(*it)->mQuadtreeNode = nullptr;

				// Add to this node
				occupants.insert(*it);
			}
		}

		// If the node has children, add them to the open list
		if (pCurrent->hasChildren())
		{
			for (int i = 0; i < 4; i++)
			{
				open.push_back(pCurrent->mChildren[i].get());
			}
		}
	}
}

Quadtree::Quadtree()
    : mMinNumNodeOccupants(3)
    , mMaxNumNodeOccupants(6)
    , mMaxLevels(40)
    , mOversizeMultiplier(1.0f)
{
}

Quadtree::Quadtree(const Quadtree& other)
{
	*this = other;
}

void Quadtree::operator=(const Quadtree &other) 
{
    mMinNumNodeOccupants = other.mMinNumNodeOccupants;
    mMaxNumNodeOccupants = other.mMaxNumNodeOccupants;
    mMaxLevels = other.mMaxLevels;
    mOversizeMultiplier = other.mOversizeMultiplier;
    mOutsideRoot = other.mOutsideRoot;
    if (other.mRootNode != nullptr) 
	{
        mRootNode.reset(new QuadtreeNode());
        recursiveCopy(mRootNode.get(), other.mRootNode.get(), nullptr);
    }
}

void Quadtree::queryRegion(std::vector<QuadtreeOccupant*> &result, const sf::FloatRect &region) 
{
    // Query outside root elements
    for (auto oc : mOutsideRoot) 
	{
        // Intersects, add to list
		if (oc != nullptr && region.intersects(oc->getAABB()))
		{
			result.push_back(oc);
		}
    }

    std::list<QuadtreeNode*> open;
    open.push_back(mRootNode.get());

    while (!open.empty()) 
	{
        // Depth-first (results in less memory usage), remove objects from open list
        QuadtreeNode* pCurrent = open.back();
        open.pop_back();

        if (region.intersects(pCurrent->mRegion)) 
		{
            for (auto oc : pCurrent->mOccupants) 
			{
                // Visible, add to list
				if (oc != nullptr && region.intersects(oc->getAABB()) && oc->isAwake())
				{
					result.push_back(oc);
				}
            }

            // Add children to open list if they intersect the region
			if (pCurrent->hasChildren())
			{
				for (int i = 0; i < 4; i++)
				{
					if (pCurrent->mChildren[i]->getNumOccupantsBelow() != 0)
					{
						open.push_back(pCurrent->mChildren[i].get());
					}
				}
			}
        }
    }
}

void Quadtree::queryPoint(std::vector<QuadtreeOccupant*> &result, const sf::Vector2f &p) 
{
    // Query outside root elements
    for (std::unordered_set<QuadtreeOccupant*>::iterator it = mOutsideRoot.begin(); it != mOutsideRoot.end(); it++) 
	{
        QuadtreeOccupant* oc = *it;
		if (oc != nullptr && oc->getAABB().contains(p))
		{
			// Intersects, add to list
			result.push_back(oc);
		}
    }

    std::list<QuadtreeNode*> open;
    open.push_back(mRootNode.get());

    while (!open.empty()) 
	{
        // Depth-first (results in less memory usage), remove objects from open list
        QuadtreeNode* pCurrent = open.back();
        open.pop_back();

        if (pCurrent->mRegion.contains(p)) 
		{
            for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++) 
			{
                QuadtreeOccupant* oc = *it;
				if (oc != nullptr && oc->getAABB().contains(p) && oc->isAwake())
				{
					// Visible, add to list
					result.push_back(oc);
				}
            }

            // Add children to open list if they intersect the region
			if (pCurrent->hasChildren())
			{
				for (int i = 0; i < 4; i++)
				{
					if (pCurrent->mChildren[i]->getNumOccupantsBelow() != 0)
					{
						open.push_back(pCurrent->mChildren[i].get());
					}
				}
			}
        }
    }
}

void Quadtree::queryShape(std::vector<QuadtreeOccupant*> &result, const sf::ConvexShape &shape) 
{
    // Query outside root elements
    for (std::unordered_set<QuadtreeOccupant*>::iterator it = mOutsideRoot.begin(); it != mOutsideRoot.end(); it++) 
	{
        QuadtreeOccupant* oc = *it;
		if (oc != nullptr && shapeIntersection(shapeFromRect(oc->getAABB()), shape))
		{
			// Intersects, add to list
			result.push_back(oc);
		}
    }

    std::list<QuadtreeNode*> open;
    open.push_back(mRootNode.get());

    while (!open.empty()) 
	{
        // Depth-first (results in less memory usage), remove objects from open list
        QuadtreeNode* pCurrent = open.back();
        open.pop_back();

        if (shapeIntersection(shapeFromRect(pCurrent->mRegion), shape)) 
		{
            for (std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->mOccupants.begin(); it != pCurrent->mOccupants.end(); it++) 
			{
                QuadtreeOccupant* oc = *it;
                sf::ConvexShape r = shapeFromRect(oc->getAABB());
				if (oc != nullptr && shapeIntersection(shapeFromRect(oc->getAABB()), shape) && oc->isAwake())
				{
					// Visible, add to list
					result.push_back(oc);
				}
            }

            // Add children to open list if they intersect the region
			if (pCurrent->hasChildren())
			{
				for (int i = 0; i < 4; i++)
				{
					if (pCurrent->mChildren[i]->getNumOccupantsBelow() != 0)
					{
						open.push_back(pCurrent->mChildren[i].get());
					}
				}
			}
        }
    }
}

size_t Quadtree::getMinNumNodeOccupants() const
{
	return mMinNumNodeOccupants;
}

size_t Quadtree::getMaxNumNodeOccupants() const
{
	return mMaxNumNodeOccupants;
}

size_t Quadtree::getMaxLevels() const
{
	return mMaxLevels;
}

float Quadtree::getOversizeMultiplier() const
{
	return mOversizeMultiplier;
}

void Quadtree::clear()
{
	mRootNode.reset();
}

void Quadtree::create(sf::FloatRect const& rootRegion)
{
	mRootNode = std::unique_ptr<QuadtreeNode>(new QuadtreeNode(rootRegion, 0, nullptr, this));
}

bool Quadtree::created() const
{
	return mRootNode != nullptr;
}

const sf::FloatRect Quadtree::getRootRegion() const
{
	if (mRootNode != nullptr)
	{
		return mRootNode->getRegion();
	}
	return sf::FloatRect();
}

void Quadtree::onRemoval()
{
}

void Quadtree::setQuadtree(QuadtreeOccupant* oc)
{
	oc->mQuadtree = this;
}

void Quadtree::recursiveCopy(QuadtreeNode* thisNode, QuadtreeNode* otherNode, QuadtreeNode* thisParent)
{
	thisNode->mHasChildren = otherNode->mHasChildren;
	thisNode->mLevel = otherNode->mLevel;
	thisNode->mNumOccupantsBelow = otherNode->mNumOccupantsBelow;
	thisNode->mOccupants = otherNode->mOccupants;
	thisNode->mRegion = otherNode->mRegion;
	thisNode->mParent = thisParent;
	thisNode->mQuadtree = this;
	if (thisNode->hasChildren())
	{
		for (int i = 0; i < 4; i++)
		{
			thisNode->mChildren[i].reset(new QuadtreeNode());
			recursiveCopy(thisNode->mChildren[i].get(), otherNode->mChildren[i].get(), thisNode);
		}
	}
}

StaticQuadtree::StaticQuadtree()
{
}

StaticQuadtree::StaticQuadtree(sf::FloatRect const& rootRegion)
{
	create(rootRegion);
}

void StaticQuadtree::add(QuadtreeOccupant* oc)
{
	assert(created());

	setQuadtree(oc);

	// If the occupant fits in the root node
	if (rectContains(mRootNode->getRegion(), oc->getAABB()))
	{
		mRootNode->add(oc);
	}
	else
	{
		mOutsideRoot.insert(oc);
	}
}

DynamicQuadtree::DynamicQuadtree()
	: mMinOutsideRoot(1)
	, mMaxOutsideRoot(8)
{
}

DynamicQuadtree::DynamicQuadtree(sf::FloatRect const& rootRegion)
	: mMinOutsideRoot(1)
	, mMaxOutsideRoot(8)
{
	create(rootRegion);
}

void DynamicQuadtree::add(QuadtreeOccupant* oc)
{
	assert(created());

	// If the occupant fits in the root node
	if (rectContains(mRootNode->getRegion(), oc->getAABB()))
	{
		mRootNode->add(oc);
	}
	else
	{
		mOutsideRoot.insert(oc);
	}

	setQuadtree(oc);
}

void DynamicQuadtree::trim()
{
	if (mRootNode.get() == nullptr)
	{
		return;
	}

	if (mOutsideRoot.size() > mMaxOutsideRoot)
	{
		expand();
	}
	else if (mOutsideRoot.size() < mMinOutsideRoot && mRootNode->hasChildren())
	{
		contract();
	}
}


void DynamicQuadtree::expand()
{
	// Find direction with most occupants
	sf::Vector2f averageDir(0.0f, 0.0f);

	for (const auto& occupant : mOutsideRoot)
	{
		averageDir += vectorNormalize(rectCenter(occupant->getAABB()) - rectCenter(mRootNode->getRegion()));
	}

	sf::Vector2f centerOffsetDist(rectHalfDims(mRootNode->getRegion()) / mOversizeMultiplier);
	sf::Vector2f centerOffset((averageDir.x > 0.0f ? 1.0f : -1.0f) * centerOffsetDist.x, (averageDir.y > 0.0f ? 1.0f : -1.0f) * centerOffsetDist.y);

	// Child node position of current root node
	int rX = centerOffset.x > 0.0f ? 0 : 1;
	int rY = centerOffset.y > 0.0f ? 0 : 1;

	sf::FloatRect newRootAABB = rectFromBounds(sf::Vector2f(0.0f, 0.0f), centerOffsetDist * 4.0f);
	newRootAABB = rectRecenter(newRootAABB, centerOffset + rectCenter(mRootNode->getRegion()));

	QuadtreeNode* pNewRoot = new QuadtreeNode(newRootAABB, mRootNode->mLevel + 1, nullptr, this);

	// ----------------------- Manual Children Creation for New Root -------------------------

	sf::Vector2f halfRegionDims = rectHalfDims(pNewRoot->mRegion);
	sf::Vector2f regionLowerBound = rectLowerBound(pNewRoot->mRegion);
	sf::Vector2f regionCenter = rectCenter(pNewRoot->mRegion);

	// Create the children nodes
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			if (x == rX && y == rY)
			{
				pNewRoot->mChildren[x + y * 2].reset(mRootNode.release());
			}
			else
			{
				sf::Vector2f offset(x * halfRegionDims.x, y * halfRegionDims.y);
				sf::FloatRect childAABB = rectFromBounds(regionLowerBound + offset, regionCenter + offset);

				// Scale up AABB by the oversize multiplier
				sf::Vector2f center = rectCenter(childAABB);

				childAABB.width *= mOversizeMultiplier;
				childAABB.height *= mOversizeMultiplier;

				childAABB = rectRecenter(childAABB, center);

				pNewRoot->mChildren[x + y * 2].reset(new QuadtreeNode(childAABB, mRootNode->mLevel, pNewRoot, this));
			}
		}
	}

	pNewRoot->mHasChildren = true;
	pNewRoot->mNumOccupantsBelow = mRootNode->mNumOccupantsBelow;
	mRootNode->mParent = pNewRoot;

	// Transfer ownership
	mRootNode.release();
	mRootNode.reset(pNewRoot);

	// ----------------------- Try to Add Previously Outside Root -------------------------

	// Make copy so don't try to re-add ones just added
	std::unordered_set<QuadtreeOccupant*> outsideRootCopy(mOutsideRoot);
	mOutsideRoot.clear();
	for (auto& occupant : outsideRootCopy)
	{
		add(occupant);
	}
}

void DynamicQuadtree::contract()
{
	assert(mRootNode->hasChildren());

	// Find child with the most occupants and shrink to that
	int maxIndex = 0;

	for (int i = 1; i < 4; i++)
	{
		if (mRootNode->mChildren[i]->getNumOccupantsBelow() > mRootNode->mChildren[maxIndex]->getNumOccupantsBelow())
		{
			maxIndex = i;
		}
	}

	// Reorganize
	for (int i = 0; i < 4; i++)
	{
		if (i == maxIndex)
		{
			continue;
		}

		mRootNode->mChildren[i]->removeForDeletion(mOutsideRoot);
	}

	QuadtreeNode* pNewRoot = mRootNode->mChildren[maxIndex].release();
	mRootNode->destroyChildren();
	mRootNode->removeForDeletion(mOutsideRoot);
	mRootNode.reset(pNewRoot);
	mRootNode->mParent = nullptr;
}

} // namespace lum