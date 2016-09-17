#include "Quadtree.hpp"

namespace ltbl
{

QuadtreeOccupant::QuadtreeOccupant()
	: mAwake(true)
{
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

void QuadtreeOccupant::quadtreeAABBChanged()
{
	mAABBChanged = true;
}

Quadtree::Quadtree(const sf::FloatRect& region, unsigned int maxOccupants, unsigned int maxLevels, Quadtree* parent, unsigned int level, unsigned int type)
	: mRegion(region)
	, mMaxOccupants(maxOccupants)
	, mMaxLevels(maxLevels)
	, mParent(parent)
	, mLevel(level)
	, mType(type)
{
}

void Quadtree::create(const sf::FloatRect& region, unsigned int maxOccupants, unsigned int maxLevels, Quadtree* parent, unsigned int level, unsigned int type)
{
	clear();

	mRegion = region;
	mMaxOccupants = maxOccupants;
	mMaxLevels = maxLevels;
	mParent = parent;
	mLevel = level;
	mType = type;
}

void Quadtree::addOccupant(QuadtreeOccupant* oc)
{
	if (oc != nullptr)
	{
		if (mRegion.intersects(oc->getAABB()))
		{
			if (hasChildren())
			{
				bool handled = false;
				for (std::size_t i = 0; i < mChildren.size(); i++)
				{
					if (!handled && mChildren[i]->mRegion.intersects(oc->getAABB()))
					{
						mChildren[i]->addOccupant(oc);
						handled = true;
					}
				}
				if (handled)
				{
					return;
				}
			}
			else
			{
				mOccupants.insert(oc);

				if (mOccupants.size() >= mMaxOccupants && mLevel < mMaxLevels)
				{
					split();
				}

				return;
			}
		}

		if (mParent != nullptr)
		{
			mParent->addOccupant(oc);
		}
		else
		{
			mOutsideOccupants.insert(oc);
		}
	}
}

bool Quadtree::removeOccupant(QuadtreeOccupant* oc)
{
	if (hasChildren())
	{
		bool removed = false;
		for (std::size_t i = 0; i < mChildren.size(); i++)
		{
			if (mChildren[i]->mRegion.intersects(oc->getAABB()))
			{
				if (mChildren[i]->removeOccupant(oc))
				{
					removed = true;
				}
			}
		}
		if (removed)
		{
			if (getNumOccupantsBelow() < mMaxOccupants)
			{
				unsplit();
			}
			return true;
		}
	}
	else
	{
		auto itr = mOccupants.find(oc);
		if (itr != mOccupants.end())
		{
			mOccupants.erase(itr);
			return true;
		}
	}

	if (mOutsideOccupants.size() > 0)
	{
		auto itr = mOutsideOccupants.find(oc);
		if (itr != mOutsideOccupants.end())
		{
			mOutsideOccupants.erase(itr);
			return true;
		}
	}

	return false;
}

bool Quadtree::update()
{
	bool moved = false;
	if (hasChildren())
	{
		for (std::size_t i = 0; i < mChildren.size(); i++)
		{
			if (mChildren[i]->update())
			{
				moved = true;
			}
		}

		if (mOutsideOccupants.size() > 0)
		{
			for (auto itr = mOutsideOccupants.begin(); itr != mOutsideOccupants.end(); )
			{
				bool erase = false;
				if ((*itr) != nullptr)
				{
					if ((*itr)->mAABBChanged && (*itr)->isAwake())
					{
						(*itr)->mAABBChanged = false;
						if (mRegion.intersects((*itr)->getAABB()))
						{
							addOccupant(*itr);
							erase = true;
						}
					}
				}
				else
				{
					erase = true;
				}
				if (erase)
				{
					itr = mOutsideOccupants.erase(itr);
				}
				else
				{
					++itr;
				}
			}
		}

		if (moved)
		{
			if (getNumOccupantsBelow() < mMaxOccupants)
			{
				unsplit();
			}
		}
	}
	else
	{
		for (auto itr = mOccupants.begin(); itr != mOccupants.end(); )
		{
			bool erase = false;
			if ((*itr) != nullptr)
			{
				if ((*itr)->mAABBChanged && (*itr)->isAwake())
				{
					(*itr)->mAABBChanged = false;
					if (!mRegion.intersects((*itr)->getAABB()))
					{
						if (mParent != nullptr)
						{
							mParent->addOccupant(*itr);
						}
						else
						{
							mOutsideOccupants.insert(*itr);
						}
						erase = true;
					}
				}
			}
			else
			{
				erase = true;
			}
			if (erase)
			{
				itr = mOccupants.erase(itr);
				moved = true;
			}
			else
			{
				++itr;
			}
		}
	}
	return moved;
}

void Quadtree::clear()
{
	if (hasChildren())
	{
		for (std::size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->clear();
			delete mChildren[i];
			mChildren[i] = nullptr;
		}
		mChildren.clear();
	}

	mOccupants.clear();
	mOutsideOccupants.clear();

	mParent = nullptr;
}

void Quadtree::query(const sf::FloatRect& area, std::vector<QuadtreeOccupant*>& occupants)
{
	for (auto itr = mOutsideOccupants.begin(); itr != mOutsideOccupants.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->isAwake() && area.intersects((*itr)->getAABB()))
		{
			occupants.push_back(*itr);
		}
	}

	std::list<Quadtree*> open;
	open.push_back(this);
	while (!open.empty())
	{
		Quadtree* current = open.back();
		open.pop_back();
		if (area.intersects(current->mRegion))
		{
			if (current->hasChildren())
			{
				for (std::size_t i = 0; i < mChildren.size(); i++)
				{
					if (current->mChildren[i]->getNumOccupantsBelow() > 0)
					{
						open.push_back(current->mChildren[i]);
					}
				}
			}
			else
			{
				for (auto itr = current->mOccupants.begin(); itr != current->mOccupants.end(); itr++)
				{
					if ((*itr) != nullptr && (*itr)->isAwake() && area.intersects((*itr)->getAABB()))
					{
						occupants.push_back(*itr);
					}
				}
			}
		}
	}
}

void Quadtree::query(const sf::Vector2f& point, std::vector<QuadtreeOccupant*>& occupants)
{
	for (auto itr = mOutsideOccupants.begin(); itr != mOutsideOccupants.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->isAwake() && (*itr)->getAABB().contains(point))
		{
			occupants.push_back(*itr);
		}
	}

	std::list<Quadtree*> open;
	open.push_back(this);
	while (!open.empty())
	{
		Quadtree* current = open.back();
		open.pop_back();
		if (current->mRegion.contains(point))
		{
			if (current->hasChildren())
			{
				for (std::size_t i = 0; i < mChildren.size(); i++)
				{
					if (current->mChildren[i]->getNumOccupantsBelow() > 0)
					{
						open.push_back(current->mChildren[i]);
					}
				}
			}
			else
			{
				for (auto itr = current->mOccupants.begin(); itr != current->mOccupants.end(); itr++)
				{
					if ((*itr) != nullptr && (*itr)->isAwake() && (*itr)->getAABB().contains(point))
					{
						occupants.push_back(*itr);
					}
				}
			}
		}
	}
}

void Quadtree::query(const sf::ConvexShape& shape, std::vector<QuadtreeOccupant*>& occupants)
{
	for (auto itr = mOutsideOccupants.begin(); itr != mOutsideOccupants.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->isAwake() && shapeIntersection(shape, shapeFromRect((*itr)->getAABB())))
		{
			occupants.push_back(*itr);
		}
	}

	std::list<Quadtree*> open;
	open.push_back(this);
	while (!open.empty())
	{
		Quadtree* current = open.back();
		open.pop_back();
		if (shapeIntersection(shape, shapeFromRect(current->mRegion)))
		{
			if (current->hasChildren())
			{
				for (std::size_t i = 0; i < mChildren.size(); i++)
				{
					if (current->mChildren[i]->getNumOccupantsBelow() > 0)
					{
						open.push_back(current->mChildren[i]);
					}
				}
			}
			else
			{
				for (auto itr = current->mOccupants.begin(); itr != current->mOccupants.end(); itr++)
				{
					if ((*itr) != nullptr && (*itr)->isAwake() && shapeIntersection(shape, shapeFromRect((*itr)->getAABB())))
					{
						occupants.push_back(*itr);
					}
				}
			}
		}
	}
}

void Quadtree::split()
{
	mChildren.clear();

	sf::Vector2f lower = { mRegion.left, mRegion.top };
	sf::Vector2f size = { mRegion.width * 0.5f, mRegion.height * 0.5f };

	for (std::size_t i = 0; i < 4; i++)
	{
		sf::FloatRect rect(lower.x, lower.y, size.x, size.y);
		switch (i)
		{
			case 1: rect.left += size.x; break;
			case 3: rect.left += size.x;
			case 2: rect.top += size.y; break;
			default: break;
		}
		mChildren.push_back(new Quadtree(rect, mMaxOccupants, mMaxLevels, this, mLevel + 1, i + 1));
	}

	for (auto itr = mOccupants.begin(); itr != mOccupants.end(); itr++)
	{
		if ((*itr) != nullptr)
		{
			addOccupant(*itr);
		}
	}

	mOccupants.clear();
}

void Quadtree::unsplit()
{
	for (std::size_t i = 0; i < mChildren.size(); i++)
	{
		if (mChildren[i] != nullptr)
		{
			assert(!mChildren[i]->hasChildren());

			for (auto itr = mChildren[i]->mOccupants.begin(); itr != mChildren[i]->mOccupants.end(); itr++)
			{
				if ((*itr) != nullptr)
				{
					mOccupants.insert(*itr);
				}
			}
			mChildren[i]->clear();

			delete mChildren[i];
			mChildren[i] = nullptr;
		}
	}
	mChildren.clear();
}

unsigned int Quadtree::getNumOccupantsBelow() const
{
	if (hasChildren())
	{
		unsigned int sum = 0;
		for (std::size_t i = 0; i < mChildren.size(); i++)
		{
			sum += mChildren[i]->getNumOccupantsBelow();
		}
		return sum;
	}
	else
	{
		return mOccupants.size();
	}
}

bool Quadtree::hasChildren() const
{
	return mChildren.size() > 0;
}

void Quadtree::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (hasChildren())
	{
		for (std::size_t i = 0; i < mChildren.size(); i++)
		{
			target.draw(*mChildren[i], states);
		}
	}
	else
	{
		sf::Color color;
		switch (mType)
		{
			case 1: color = sf::Color::Red; break;
			case 2: color = sf::Color::Green; break;
			case 3: color = sf::Color::Blue; break;
			case 4: color = sf::Color::Yellow; break;
			default: color = sf::Color::Magenta; break;
		}
		for (auto itr = mOccupants.begin(); itr != mOccupants.end(); itr++)
		{
			if ((*itr) != nullptr)
			{
				sf::FloatRect box = (*itr)->getAABB();
				sf::RectangleShape oc({ box.width, box.height });
				oc.setPosition({ box.left, box.top });
				oc.setFillColor(color);
				target.draw(oc, states);
			}
		}

		sf::RectangleShape shape({ mRegion.width, mRegion.height });
		shape.setPosition({ mRegion.left, mRegion.top });
		shape.setFillColor(sf::Color::Transparent);
		shape.setOutlineColor(sf::Color::Black);
		shape.setOutlineThickness(1.f);
		target.draw(shape, states);
	}

	for (auto itr = mOutsideOccupants.begin(); itr != mOutsideOccupants.end(); itr++)
	{
		if ((*itr) != nullptr)
		{
			sf::FloatRect box = (*itr)->getAABB();
			sf::RectangleShape oc({ box.width, box.height });
			oc.setPosition({ box.left, box.top });
			oc.setFillColor(sf::Color::Cyan);
			target.draw(oc, states);
		}
	}
}

sf::ConvexShape Quadtree::shapeFromRect(const sf::FloatRect & rect)
{
	sf::ConvexShape shape(4);
	shape.setPoint(0, { 0.f , 0.f });
	shape.setPoint(1, { rect.width, 0.f });
	shape.setPoint(2, { rect.width, rect.height});
	shape.setPoint(3, { 0.f, rect.height });
	shape.setPosition(rect.left, rect.top);
	return shape;
}

bool Quadtree::shapeIntersection(const sf::ConvexShape& left, const sf::ConvexShape& right)
{
	std::vector<sf::Vector2f> transformedLeft(left.getPointCount());
	for (unsigned i = 0; i < left.getPointCount(); i++)
	{
		transformedLeft[i] = left.getTransform().transformPoint(left.getPoint(i));
	}

	std::vector<sf::Vector2f> transformedRight(right.getPointCount());
	for (unsigned i = 0; i < right.getPointCount(); i++)
	{
		transformedRight[i] = right.getTransform().transformPoint(right.getPoint(i));
	}

	for (unsigned i = 0; i < left.getPointCount(); i++) 
	{
		sf::Vector2f point = transformedLeft[i];
		sf::Vector2f nextPoint = (i == left.getPointCount() - 1u) ? transformedLeft[0] : transformedLeft[i + 1];
		sf::Vector2f edge = nextPoint - point;
		sf::Vector2f edgePerpendicular = sf::Vector2f(edge.y, -edge.x);
		float magnitude = edgePerpendicular.x * edgePerpendicular.x + edgePerpendicular.y * edgePerpendicular.y;
		float pointProj = (point.x * edgePerpendicular.x + point.y * edgePerpendicular.y) / magnitude;
		float minRightProj = (transformedRight[0].x * edgePerpendicular.x + transformedRight[0].y * edgePerpendicular.y) / magnitude;

		for (unsigned j = 1; j < right.getPointCount(); j++)
		{
			float proj = (transformedRight[j].x * edgePerpendicular.x + transformedRight[j].y * edgePerpendicular.y) / magnitude;
			minRightProj = std::min(minRightProj, proj);
		}

		if (minRightProj > pointProj)
		{
			return false;
		}
	}

	for (unsigned i = 0; i < right.getPointCount(); i++)
	{
		sf::Vector2f point = transformedRight[i];
		sf::Vector2f nextPoint = (i == right.getPointCount() - 1u) ? transformedRight[0] : transformedRight[i + 1];
		sf::Vector2f edge = nextPoint - point;
		sf::Vector2f edgePerpendicular = sf::Vector2f(edge.y, -edge.x);
		float magnitude = edgePerpendicular.x * edgePerpendicular.x + edgePerpendicular.y * edgePerpendicular.y;
		float pointProj = (point.x * edgePerpendicular.x + point.y * edgePerpendicular.y) / magnitude;
		float minRightProj = (transformedLeft[0].x * edgePerpendicular.x + transformedLeft[0].y * edgePerpendicular.y) / magnitude;

		for (unsigned j = 1; j < left.getPointCount(); j++)
		{
			float proj = (transformedLeft[j].x * edgePerpendicular.x + transformedLeft[j].y * edgePerpendicular.y) / magnitude;
			minRightProj = std::min(minRightProj, proj);
		}

		if (minRightProj > pointProj)
		{
			return false;
		}
	}

	return true;
}

} // namespace ltbl