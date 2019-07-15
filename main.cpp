#include "stdafx.h"
#include "classes/system/Shader.h"
#include "classes/system/Scene.h"
#include "classes/system/FPSController.h"
#include "classes/buffers/StaticBuffer.h"
#include "classes/level/Leaf.h"
#include "classes/level/PathGenerator.h"
#include "classes/delaunay/delaunay.h"
#include "classes/level/TileMap.h"
#include "classes/system/Random.h"

#include <queue>
#include <list>

bool Pause;
bool keys[1024] = {0};
int WindowWidth = 800, WindowHeight = 600;
bool EnableVsync = 1;
GLFWwindow* window;
stFPSController FPSController;

int TilesCount[2] = {60, 60};//{30, 30}
glm::vec2 Edge(2, 2);
glm::vec2 TileSize(10, 10);//(20, 20)
glm::vec2 MouseSceneCoord;

MShader Shader;
MScene Scene;

MStaticBuffer RoomsBuffer;
MStaticBuffer LeafsBuffer;

list<NVector2> GetRegion(char Type, int i, int j, char** inCellsMap, char** inFlagsMap) {
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
					if(inCellsMap[x][y] == Type && !inFlagsMap[x][y] && (x == Cell.x || y == Cell.y) ) {
						Queue.push(NVector2(x, y));
						inFlagsMap[x][y] = 1;
					}
				}
			}
        }
		Queue.pop();
	}
	
	return Region;
}

bool GenerateCave() {
	RoomsBuffer.Clear();
	
	int RandomPercent = 55;
	int SmoothCycles = 2;
	int WallsToSmooth = 4;
	int WallTreshold = 30;
	int RoomTreshold = 30;
	
	//allocate memory
	char** CellsMap;
	CellsMap = new char* [TilesCount[0]];
	for(int i=0; i<TilesCount[0]; i++) {
		CellsMap[i] = new char [TilesCount[1]];
		memset(CellsMap[i], 0, TilesCount[1]);
	}
	
	//fill map
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			CellsMap[i][j] = RandomChancePercent(RandomPercent);
		}
	}
	
	//smooth map
	int WallsCount;
	for(int k=0; k<SmoothCycles; k++) {
		for(int i=1; i<TilesCount[0] - 1; i++) {
			for(int j=1; j<TilesCount[1] - 1; j++) {
				//get surround walls count
				WallsCount = CellsMap[i-1][j-1] + CellsMap[i][j-1] + CellsMap[i+1][j-1] +
					CellsMap[i-1][j] + CellsMap[i+1][j] + 
					CellsMap[i-1][j+1] + CellsMap[i][j+1] + CellsMap[i+1][j+1];
				if(WallsCount > WallsToSmooth) CellsMap[i][j] = 1;
				else if(WallsCount < WallsToSmooth) CellsMap[i][j] = 0;
			}
		}	
	}
	
	//work with regions
	char** FlagsMap;
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
	//clear regions
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
	for(int i=0; i<TilesCount[0]; i++) {
		if(FlagsMap[i]) delete [] FlagsMap[i];
	}
	if(FlagsMap) delete [] FlagsMap;
	
	//fill map in buffer
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			if(CellsMap[i][j]) {
				RoomsBuffer.AddVertex(glm::vec2(i * TileSize.x, j * TileSize.y), RegionColor);
				RoomsBuffer.AddVertex(glm::vec2((i + 1) * TileSize.x, j * TileSize.y), RegionColor);
				RoomsBuffer.AddVertex(glm::vec2((i + 1) * TileSize.x, (j + 1) * TileSize.y), RegionColor);
				RoomsBuffer.AddVertex(glm::vec2(i * TileSize.x, (j + 1) * TileSize.y), RegionColor);
			}
		}
	}
	
	//clear memory
	for(int i=0; i<TilesCount[0]; i++) {
		if(CellsMap[i]) delete [] CellsMap[i];
	}
	if(CellsMap) delete [] CellsMap;
	
	if(!RoomsBuffer.Dispose()) return false;
	return true;
}

bool GenerateLevel() {
	RoomsBuffer.Clear();
	LeafsBuffer.Clear();
	
	list<TNode<stLeaf>* > Tree;
	int MinLeafSize = 6;
	int MaxLeafSize = 20;
	int MinRoomSize = 3;
	
	if(MinRoomSize < 3) {
		cout<<"Room size cannot be less than 3"<<endl;
		return false;
	}
	
	//create tree
	if(MinRoomSize >= MinLeafSize || MinLeafSize >= MaxLeafSize) {
		cout<<"Wrong settings"<<endl;
		return false;
	}
	if(!SplitTree(&Tree, TilesCount[0], TilesCount[1], MinLeafSize, MaxLeafSize)) return false;
	
	//create rooms and fill centers map
	glm::vec3 Color = glm::vec3(1, 1, 1);
	TNode<NRectangle2>* pRoomNode;
	map<glm::vec2, TNode<NRectangle2>*, stVec2Compare> NodesCenters;
	glm::vec2 Center;
	NRectangle2* pRoom;
	stLeaf* pLeaf;
	list<TNode<stLeaf>* >::iterator itl;
	int RoomsNumber = 0;
	for(itl = Tree.begin(); itl != Tree.end(); itl++) {
		pRoomNode = CreateRoomInLeaf(*itl, MinRoomSize);
		if(!pRoomNode) continue;
		pRoom = pRoomNode->GetValueP();
		if(!pRoom) continue;
		//add in map
		Center.x = (pRoom->Position.x + pRoom->Size.x * 0.5) * TileSize.x;
		Center.y = (pRoom->Position.y + pRoom->Size.y * 0.5) * TileSize.y;
		NodesCenters.insert(pair<glm::vec2, TNode<NRectangle2>* >(Center, pRoomNode));
		//add leaf in buffer
		pLeaf = (*itl)->GetValueP();
		LeafsBuffer.AddVertex(glm::vec2(pLeaf->Body.Position.x * TileSize.x, pLeaf->Body.Position.y * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2((pLeaf->Body.Position.x + pLeaf->Body.Size.x) * TileSize.x, pLeaf->Body.Position.y * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2(pLeaf->Body.Position.x * TileSize.x, (pLeaf->Body.Position.y + pLeaf->Body.Size.y) * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2((pLeaf->Body.Position.x + pLeaf->Body.Size.x) * TileSize.x, (pLeaf->Body.Position.y + pLeaf->Body.Size.y) * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2(pLeaf->Body.Position.x * TileSize.x, pLeaf->Body.Position.y * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2(pLeaf->Body.Position.x * TileSize.x, (pLeaf->Body.Position.y + pLeaf->Body.Size.y) * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2((pLeaf->Body.Position.x + pLeaf->Body.Size.x) * TileSize.x, pLeaf->Body.Position.y * TileSize.y), glm::vec3(1,1,1));
		LeafsBuffer.AddVertex(glm::vec2((pLeaf->Body.Position.x + pLeaf->Body.Size.x) * TileSize.x, (pLeaf->Body.Position.y + pLeaf->Body.Size.y) * TileSize.y), glm::vec3(1,1,1));
		//add room in buffer
		RoomsBuffer.AddVertex(glm::vec2(pRoom->Position.x * TileSize.x, pRoom->Position.y * TileSize.y), glm::vec3(1, 1, 1));
		RoomsBuffer.AddVertex(glm::vec2((pRoom->Position.x + pRoom->Size.x) * TileSize.x, pRoom->Position.y * TileSize.y), glm::vec3(1, 1, 1));
		RoomsBuffer.AddVertex(glm::vec2((pRoom->Position.x + pRoom->Size.x) * TileSize.x, (pRoom->Position.y + pRoom->Size.y) * TileSize.y), glm::vec3(1, 1, 1));
		RoomsBuffer.AddVertex(glm::vec2(pRoom->Position.x * TileSize.x, (pRoom->Position.y + pRoom->Size.y) * TileSize.y), glm::vec3(1, 1, 1));
		RoomsNumber ++;
	}
	if(RoomsNumber < 2) {
		cout<<"Too few rooms: "<<RoomsNumber<<endl;
		return false;
	}
	
	//copy centers for triangulation
	map<glm::vec2, TNode<NRectangle2>*, stVec2Compare>::iterator itm;
	vector<glm::vec2> CentersPoints;
	for(itm = NodesCenters.begin(); itm != NodesCenters.end(); itm++) {
		CentersPoints.push_back(itm->first);
	}
	
	//triangulate by delaunay and get mst
	MDelaunay Triangulation;
	vector<MTriangle> Triangles = Triangulation.Triangulate(CentersPoints);
	vector<MEdge> Edges = Triangulation.GetEdges();
	vector<MEdge> MST = Triangulation.CreateMSTEdges();
	
	//create halls
	vector<NRectangle2> Halls;
	TNode<NRectangle2>* pNode0;
	TNode<NRectangle2>* pNode1;
	glm::vec3 HallColor = glm::vec3(1, 1, 1);
	for(int i=0; i<MST.size(); i++) {
		pNode0 = NodesCenters[MST[i].p1];
		pNode1 = NodesCenters[MST[i].p2];
		
		Halls.clear();
		Halls = CreateHalls3(pNode0->GetValueP(), pNode1->GetValueP());
		for(int j=0; j<Halls.size(); j++) {
			RoomsBuffer.AddVertex(glm::vec2(Halls[j].Position.x * TileSize.x, Halls[j].Position.y * TileSize.y), HallColor);
			RoomsBuffer.AddVertex(glm::vec2((Halls[j].Position.x + Halls[j].Size.x) * TileSize.x, Halls[j].Position.y * TileSize.y), HallColor);
			RoomsBuffer.AddVertex(glm::vec2((Halls[j].Position.x + Halls[j].Size.x) * TileSize.x, (Halls[j].Position.y + Halls[j].Size.y) * TileSize.y), HallColor);
			RoomsBuffer.AddVertex(glm::vec2(Halls[j].Position.x * TileSize.x, (Halls[j].Position.y + Halls[j].Size.y) * TileSize.y), HallColor);	
			cout<<Halls[j].Position.x<<" "<<Halls[j].Position.y<<" "<<Halls[j].Size.x<<" "<<Halls[j].Size.y<<endl;
		}
		
		Center.x = (pNode0->GetValueP()->Position.x + pNode0->GetValueP()->Size.x * 0.5) * TileSize.x;
		Center.y = (pNode0->GetValueP()->Position.y + pNode0->GetValueP()->Size.y * 0.5) * TileSize.y;
		LeafsBuffer.AddVertex(Center, glm::vec3(0, 0, 1));
		Center.x = (pNode1->GetValueP()->Position.x + pNode1->GetValueP()->Size.x * 0.5) * TileSize.x;
		Center.y = (pNode1->GetValueP()->Position.y + pNode1->GetValueP()->Size.y * 0.5) * TileSize.y;
		LeafsBuffer.AddVertex(Center, glm::vec3(0, 0, 1));
	}
	Halls.clear();
	cout<<"===================="<<endl;
	
	MST.clear();
	Triangulation.Clear();
	Triangles.clear();
	Edges.clear();
	CentersPoints.clear();

	NodesCenters.clear();
	
	ClearTree(&Tree);
	
	if(!LeafsBuffer.Dispose()) return false;
	if(!RoomsBuffer.Dispose()) return false;
	return true;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void mousepos_callback(GLFWwindow* window, double x, double y) {
	MouseSceneCoord = Scene.WindowPosToWorldPos(x, y);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	if(key == 'R' && action == GLFW_PRESS) {
		GenerateLevel();
		//GenerateCave();
	}
}

bool InitApp() {
	LogFile<<"Starting application"<<endl;    
    glfwSetErrorCallback(error_callback);
    
    if(!glfwInit()) return false;
    window = glfwCreateWindow(WindowWidth, WindowHeight, "TestApp", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return false;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);
    if(glfwExtensionSupported("WGL_EXT_swap_control")) {
    	LogFile<<"Window: V-Sync supported. V-Sync: "<<EnableVsync<<endl;
		glfwSwapInterval(EnableVsync);//0 - disable, 1 - enable
	}
	else LogFile<<"Window: V-Sync not supported"<<endl;
    LogFile<<"Window created: width: "<<WindowWidth<<" height: "<<WindowHeight<<endl;

	//glew
	GLenum Error = glewInit();
	if(GLEW_OK != Error) {
		LogFile<<"Window: GLEW Loader error: "<<glewGetErrorString(Error)<<endl;
		return false;
	}
	LogFile<<"GLEW initialized"<<endl;
	
	if(!CheckOpenglSupport()) return false;

	//shaders
	if(!Shader.CreateShaderProgram("shaders/main.vertexshader.glsl", "shaders/main.fragmentshader.glsl")) return false;
	if(!Shader.AddUnifrom("MVP", "MVP")) return false;
	LogFile<<"Shaders loaded"<<endl;

	//scene
	if(!Scene.Initialize(&WindowWidth, &WindowHeight)) return false;
	LogFile<<"Scene initialized"<<endl;

	//randomize
    srand(time(NULL));
    LogFile<<"Randomized"<<endl;
    
    //other initializations
    //init buffers
    if(!LeafsBuffer.Initialize()) return false;
    LeafsBuffer.SetPrimitiveType(GL_LINES);
    if(!RoomsBuffer.Initialize()) return false;
    RoomsBuffer.SetPrimitiveType(GL_QUADS);
    //generate level
	if(!GenerateLevel()) return false;
	//if(!GenerateCave()) return false;
	
	//turn off pause
	Pause = false;
    
    return true;
}

void RenderStep() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(Shader.ProgramId);
	glUniformMatrix4fv(Shader.Uniforms["MVP"], 1, GL_FALSE, Scene.GetDynamicMVP());
	
	//draw functions
	RoomsBuffer.Begin();
		//glEnable(GL_DEPTH_TEST);
		RoomsBuffer.Draw();
		//glDisable(GL_DEPTH_TEST);
		LeafsBuffer.Draw();
	RoomsBuffer.End();
}

void ClearApp() {
	//clear funstions
	LeafsBuffer.Close();
	RoomsBuffer.Close();
	
	memset(keys, 0, 1024);
	Shader.Close();
	LogFile<<"Application: closed"<<endl;
}

int main(int argc, char** argv) {
	LogFile<<"Application: started"<<endl;
	if(!InitApp()) {
		ClearApp();
		glfwTerminate();
		LogFile.close();
		return 0;
	}
	FPSController.Initialize(glfwGetTime());
	while(!glfwWindowShouldClose(window)) {
		FPSController.FrameStep(glfwGetTime());
    	FPSController.FrameCheck();
		RenderStep();
        glfwSwapBuffers(window);
        glfwPollEvents();
	}
	ClearApp();
    glfwTerminate();
    LogFile.close();
}
