#pragma once
#ifndef __Grid__
#define __Grid__

#include <vector>
#include <glm/fwd.hpp>


class Grid
{
public:
	Grid();
	virtual ~Grid();
	void setSize(int nrows, int ncols);
	void reset();
	void moveCP(const glm::vec2 &p);
	void findClosest(const glm::vec2 &p);
	void draw() const;
	
	int indexAt(int row, int col) const;
	int getRows() const { return nrows; };
	int getCols() const { return ncols; };
	std::vector<glm::vec2> getTileCPs(int index) const;
	const std::vector<glm::vec2> & getAllCPs() const;
	void save(const char *filename) const;
	void load(const char *filename);
	
private:
	
	std::vector<glm::vec2> cps;
	int nrows;
	int ncols;
	int closest;
};

#endif
