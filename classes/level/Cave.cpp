#include "Cave.h"

MCave::MCave():MLevel {
	FlagsMap = NULL;
	RandomPercent = 0;
	SmoothCycles = 0;
	WallsToSmooth = 0;
	WallTreshold = 0;
	RoomTreshold = 0;
}

MCave::MCave(int TilesCountX, int TilesCountY, int inRandomPercent, int inSmoothCycles, int inWallsToSmooth, int inWallTreshold, int inRoomTreshold):MLevel(TilesCountX, TilesCountY) {
	FlagsMap = NULL;
	RandomPercent = 0;
	SmoothCycles = 0;
	WallsToSmooth = 0;
	WallTreshold = 0;
	RoomTreshold = 0;
	if(inRandomPercent > 0) RandomPercent = inRandomPercent;
	if(inSmoothCycles > 0) SmoothCycles = inSmoothCycles;
	if(inWallsToSmooth > 0) WallsToSmooth = inWallsToSmooth;
	if(inWallTreshold > 0) WallTreshold = inWallTreshold;
	if(inRoomTreshold > 0) RoomTreshold = inRoomTreshold;
}

bool MLevel::AllocateFlagsMap() {
	if(TilesCount[0] <= 0 || TilesCount[1] <= 0) return false;
	if(FlagsMap) return false;
	
	FlagsMap = new char* [TilesCount[0]];
	for(int i=0; i<TilesCount[0; i++) {
		FlagsMap[i] = new char[TilesCount[1]];
		memset(FlagsMap[i], 0, TilesCount[1]);
	}
	
	return true;
}

list<NVector2> MCave::GetRegion(int i, int j, char Value) {
	list<NVector2> Region;
	queue<NVector2> Queue;
	NVector2 Cell;
	
	Queue.push(NVector2(i, j));
	inFlagsMap[i][j] = 1;
	
	while(Queue.size()) {
		Cell = Queue.front();
		Region.push_back(Cell);
		for (int x = Cell.x - 1; x <= Cell.x + 1; x++) {
			for (int y = Cell.y - 1; y <= Cell.y + 1; y++) {
				if(x < TilesCount[0] && y < TilesCount[1] && x > 0 && y > 0) {
					if(Map[x][y] == Value && !FlagsMap[x][y] && (x == Cell.x || y == Cell.y) ) {
						Queue.push(NVector2(x, y));
						FlagsMap[x][y] = 1;
					}
				}
			}
        }
		Queue.pop();
	}
	
	return Region;
}

bool MCave::Generate() {
	if(!Map) return false;
	Clear();
	
	//fill map
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			CellsMap[i][j] = RandomChancePercent(RandomPercent);
		}
	}
	
	SmoothMap();
	
	return true;
}

void MCave::Clear() {
	if(FlagsMap) {
		for(int i=0; i<TilesCount[0]; i++) {
			memset(FlagsMap[i], 0, TilesCount[1]);
		}
	}
	
	MLevel::Clear();
}

void MCave::Close() {
	if(FlagsMap) {
		for(int i=0; i<TilesCount[0]; i++) {
			if(FlagsMap[i]) delete [] FlagsMap[i];
		}
		delete FlagsMap;
	}
	FlagsMap = NULL;
	
	MLevel::Close();
}

int MCave::GetType() {
	return LGT_CAVE;
}

void MCave::SmoothMap() {
	//smooth map
	int WallsCount;
	for(int k=0; k<SmoothCycles; k++) {
		for(int i=1; i<TilesCount[0] - 1; i++) {
			for(int j=1; j<TilesCount[1] - 1; j++) {
				//get surround walls count
				WallsCount = Map[i-1][j-1] + Map[i][j-1] + Map[i+1][j-1] +
					Map[i-1][j] + Map[i+1][j] + 
					Map[i-1][j+1] + Map[i][j+1] + Map[i+1][j+1];
				if(WallsCount > WallsToSmooth) Map[i][j] = 1;
				else if(WallsCount < WallsToSmooth) Map[i][j] = 0;
			}
		}	
	}
	
	//work with regions
	glm::vec3 RegionColor = glm::vec3(1, 1, 1);
	list<list<NVector2> > RoomsRegionsList;
	list<list<NVector2> > WallsRegionsList;
	FlagsMap = new char* [TilesCount[0]];
	for(int i=0; i<TilesCount[0]; i++) {
		FlagsMap[i] = new char [TilesCount[1]];
		memset(FlagsMap[i], 0, TilesCount[1]);
	}
	
	//get regions
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			if(FlagsMap[i][j] == 0 && CellsMap[i][j] == 1) {
				RoomsRegionsList.push_back(GetRegion(1, i, j, CellsMap, FlagsMap));
			}
			if(FlagsMap[i][j] == 0 && CellsMap[i][j] == 0) {
				WallsRegionsList.push_back(GetRegion(0, i, j, CellsMap, FlagsMap));
			}
		}
	}
	
	//remove small walls and rooms regions, clear regions
	for(list<list<NVector2> >::iterator it = RoomsRegionsList.begin(); it != RoomsRegionsList.end(); it++) {
		if(it->size() < RoomTreshold) {
			for(list<NVector2>::iterator ita = it->begin(); ita != it->end(); ita++) {
				CellsMap[ita->x][ita->y] = 0;
			}	
		}
		it->clear();
	}
	for(list<list<NVector2> >::iterator it = WallsRegionsList.begin(); it != WallsRegionsList.end(); it++) {
		if(it->size() < WallTreshold) {
			for(list<NVector2>::iterator ita = it->begin(); ita != it->end(); ita++) {
				if(ita->x > 1 && ita->y > 1 && ita->x < TilesCount[0] && ita->y < TilesCount[1]) CellsMap[ita->x][ita->y] = 1;
			}	
		}
		it->clear();
	}
	RoomsRegionsList.clear();
	WallsRegionsList.clear();
	
	//remove flags map
	for(int i=0; i<TilesCount[0]; i++) {
		if(FlagsMap[i]) delete [] FlagsMap[i];
	}
	if(FlagsMap) delete [] FlagsMap;
	
	return true;
}