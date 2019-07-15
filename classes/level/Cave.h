#ifndef caveH
#define caveH

#include "Level.h"
#include <list>
#include <queue>

class MCave: public MLevel {
private:
	int RandomPercent = 55;
	int SmoothCycles = 2;
	int WallsToSmooth = 4;
	int WallTreshold = 30;
	int RoomTreshold = 30;
	char** FlagsMap;
	
	void SmoothMap();
	bool AllocateFlagsMap();
	list<NVector2> GetRegion(int i, int j, char Value);
public:
	MCave();
	MCave(int TilesCountX, int TilesCountY, int inRandomPercent, int inSmoothCycles, int inWallsToSmooth, int inWallTreshold, int inRoomTreshold);
	bool Generate();
	void Clear();
	void Close();
	int GetType();
}

#endif
