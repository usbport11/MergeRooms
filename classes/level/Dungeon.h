#ifndef dungeonH
#define dungeonH

#include "Leaf.h"
#include "../delaunay/delaunay.h"
#include "Level.h"

class MDungeon: public MLevel {
private:
	int MinLeafSize;
	int MaxLeafSize;
	int MinRoomSize;
	list<TNode<stLeaf>* > Tree;
	map<glm::vec2, TNode<NRectangle2>*, stVec2Compare> NodesCenters;
	vector<MEdge> MST;
	
	bool CreateRooms();
	bool Triangulate();
	bool CreateHalls();
public:
	MDungeon();
	MDungeon(int TilesCountX, int TilesCountY, int inMinLeafSize, int inMaxLeafSize, int inMinRoomSize);
	bool Generate();
	bool Clear();
	bool Close();
	int GetType();
};

#endif
