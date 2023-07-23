#include<iostream>
#include<fstream>
#include<graphics.h>
#include<ctime>
#include<mmsystem.h>
#include<cmath>
#include "tools.h"
#include "vector2.h"
#pragma comment(lib,"winmm.lib")
using namespace std;

#define WIDTH 900	//the width of the window
#define HEIGHT 600	//the height of the window
#define ZM_MAX 10  //the count of zombies you need to kill

enum { PEA, SUNFLOWER, plantCount };//the types of the plant
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCE };//the status of sunshine
enum { GOING, WIN, FAIL }; //the status of the game
int cost[3] = { 0,100,50 };//the cost of sunshine for a certain plant
IMAGE imgBg;//background image
IMAGE imgBar;//bar image
IMAGE imgCards[plantCount];//plant cards image
IMAGE* imgPlant[plantCount][20];//plant image
int killCount;
int zmCount;
int gameStatus;
int curx, cury, curPlant;
int sunshine = 50;

class plant {
public:
	int x, y;
	int type;
	int frameIndex;
	bool caught;
	int deadTimer;
	int timer;
};
class sunshineBall {
public:
	int x, y;
	int frameIndex;
	int destY;
	bool used;
	int timer;
	float xoff, yoff;
	float t;
	vector2 p1, p2, p3, p4;
	vector2 pCur;
	float speed;
	int status;
};
sunshineBall balls[10];
IMAGE imgballs[29];
plant plantMap[3][9];

class zombie {
public:
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int health;
	bool dead;
	bool eating;
};
zombie zms[10];
IMAGE imgZombie[30];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];
IMAGE imgZMStand[11];

class bullet {
public:
	int x, y;
	int speed;
	bool used;
	int row;
	bool blast;
	int frameIndex;
};
bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBallBlast[4];

class gameFunc {
public:
	bool fileExist(char*);
	void gameInit();
	void startUI();
	void viewScence();
	void barsDown();
	void drawZombie();
	void drawSunshine();
	void drawCards();
	void drawPlant();
	void drawBullets();
	void updateWin();
	void updateSunshine();
	void updateZombie();
	void updateBullets();
	void updatePlant();
	void updateGame();
	void createSunshine();
	void createZombie();
	void collectSunshine(ExMessage*);
	void userClick();
	void shoot();
	void collisionCheck();
	bool checkGame();
};
gameFunc pvz;

int main()
{
	pvz.gameInit();
	pvz.startUI();
	pvz.viewScence();
	pvz.barsDown();
	int timer = 0;
	while (1) {
		pvz.userClick();
		timer += getDelay();
		if (timer > 10) {
			pvz.updateWin();
			pvz.updateGame();
			if (pvz.checkGame()) {
				break;
			}
			timer = 0;
		}


	}
	system("pause");
	return 0;
}
bool gameFunc::fileExist(char* name) {
	ifstream fp(name, ios::in);
	if (fp) return true;
	else return false;
}
void gameFunc::gameInit() {
	//load background image and the bar
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");
	//initialize the datas
	memset(imgPlant, 0, sizeof(imgPlant));
	memset(zms, 0, sizeof(zms));
	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;
	char name[64];
	for (int i = 0; i < plantCount; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			if (fileExist(name)) {
				imgPlant[i][j] = new IMAGE;
				loadimage(imgPlant[i][j], name);
			}
			else break;
		}
	}
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZombie[i], name);
	}
	curPlant = 0;
	sunshine = 50;
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgballs[i], name);
	}
	srand(time(NULL));
	initgraph(WIDTH, HEIGHT);
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy_s(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));
	//initialize the image of bullets and zombies
	loadimage(&imgBallBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = 0.2 * (i + 1);
		loadimage(&imgBallBlast[i], "res/bullets/bullet_blast.png", imgBallBlast[3].getwidth() * k, imgBallBlast->getheight() * k, true);

	}
	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i], name);
	}
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZMStand[i], name);
	}
}
void gameFunc::drawZombie() {
	for (int i = 0; i < sizeof(zms) / sizeof(zms[0]); i++) {
		if (zms[i].used) {
			IMAGE* img = NULL;
			if (zms[i].dead)	img = imgZMDead;
			else if (zms[i].eating)	img = imgZMEat;
			else img = imgZombie;
			img += zms[i].frameIndex;
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}
void gameFunc::drawSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			IMAGE* img = &imgballs[balls[i].frameIndex];
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);//show the amount of the sunshine
}
void gameFunc::drawCards() {
	for (int i = 0; i < plantCount; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}
void gameFunc::drawPlant() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (plantMap[i][j].type > 0) {
				int plantType = plantMap[i][j].type - 1;
				int index = plantMap[i][j].frameIndex;
				putimagePNG(plantMap[i][j].x, plantMap[i][j].y, imgPlant[plantType][index]);
			}
		}
	}
	if (curPlant) {
		IMAGE* img = imgPlant[curPlant - 1][0];
		putimagePNG(curx - img->getwidth() / 2, cury - img->getheight() / 2, img);
	}
}
void gameFunc::drawBullets() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBallBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
		if (bullets[i].used) {
			putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
		}
	}
}
void gameFunc::updateWin() {
	BeginBatchDraw();
	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);
	pvz.drawCards();
	pvz.drawPlant();
	pvz.drawSunshine();
	pvz.drawZombie();
	pvz.drawBullets();
	EndBatchDraw();
}
void gameFunc::collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgballs[0].getwidth();
	int h = imgballs[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h) {
				balls[i].status = SUNSHINE_COLLECT;
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);
				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8;
				balls[i].t = 0;
				balls[i].speed = 1.0 / (distance / off);
				break;

			}
		}
	}
}
void gameFunc::userClick() {
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * plantCount && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				status = 1;
				curPlant = index + 1;
			}
			else {
				pvz.collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			curx = msg.x;
			cury = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP && status == 1) {
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				int remain = sunshine - cost[curPlant];
				if (plantMap[row][col].type == 0 && remain >= 0) {
					plantMap[row][col].timer = 0;
					plantMap[row][col].type = curPlant;
					plantMap[row][col].frameIndex = 0;
					plantMap[row][col].x = 256 + col * 81;
					plantMap[row][col].y = 179 + row * 102 + 14;
					sunshine = remain;
				}
			}
			curPlant = 0;
			status = 0;
		}
	}
}
void gameFunc::createSunshine() {
	static int count = 0;
	static int fre = 400;
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 200;
		count = 0;
		int i = 0;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)	return;
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}
	//sunflowers produce the sunshine
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (plantMap[i][j].type == SUNFLOWER + 1) {
				plantMap[i][j].timer++;
				if (plantMap[i][j].timer > 200) {
					plantMap[i][j].timer = 0;
					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;
					balls[k].used = true;
					balls[k].p1 = vector2(plantMap[i][j].x, plantMap[i][j].y);
					int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);
					float p4y = plantMap[i][j].y + imgPlant[SUNFLOWER][0]->getheight() - imgballs[0].getheight();
					balls[k].p4 = vector2(plantMap[i][j].x + w, p4y);
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCE;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}
}
void gameFunc::updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex++;
			balls[i].frameIndex %= 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
					balls[i].timer = 0;

				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCE) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
		}
	}
}
void gameFunc::createZombie() {
	if (zmCount >= ZM_MAX) {
		return;
	}

	static int zmFre = 200;
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 200 + 300;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax) {
			memset(&zms[i], 0, sizeof(zms[i]));
			zms[i].used = true;
			zms[i].dead = false;
			zms[i].row = rand() % 3;
			zms[i].x = WIDTH;
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].health = 300;
			zmCount++;
		}
	}

}
void gameFunc::updateZombie() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int cnt = 0;
	cnt++;
	if (cnt > 4) {
		cnt = 0;
		//update the location of zombies
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 10) {
					//GAME OVER
					gameStatus = FAIL;
				}
			}
		}
	}
	static int cnt2;
	cnt2++;
	if (cnt2 > 8) {
		cnt2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {
					zms[i].frameIndex++;
					if (zms[i].frameIndex > 19) {
						zms[i].used = false;
						killCount++;
						if (killCount == ZM_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}

}
void gameFunc::shoot() {
	int lines[3] = { 0 };
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int zmCnt = sizeof(zms) / sizeof(zms[0]);
	int dangerX = WIDTH - imgZombie[0].getwidth() + 50;
	for (int i = 0; i < zmCnt; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (plantMap[i][j].type == PEA + 1 && lines[i]) {
				plantMap[i][j].timer++;
				if (plantMap[i][j].timer > 100) {
					plantMap[i][j].timer = 0;
					int k = 0;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 6;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;
						int zwX = 256 + j * 81;
						int zwY = 179 + i * 102 + 14;
						bullets[k].x = zwX + imgPlant[plantMap[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}
void gameFunc::updateBullets() {
	int cnt = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < cnt; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIDTH) {
				bullets[i].used = false;
			}
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex > 3) {
					bullets[i].used = false;
				}
			}
		}
	}
}
void gameFunc::collisionCheck() {
	//check the collision between bullets and zombies
	int cntB = sizeof(bullets) / sizeof(bullets[0]);
	int cntZ = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < cntB; i++) {
		if (bullets[i].used == false || bullets[i].blast) continue;
		for (int k = 0; k < cntZ; k++) {
			if (zms[k].used == false) continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && bullets[i].x > x1 && bullets[i].x < x2) {
				zms[k].health -= 20;
				bullets[i].blast = true;
				bullets[i].speed = 0;
				if (zms[k].health <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}
		}
	}
	//check the collision between plant and zombies
	for (int i = 0; i < cntZ; i++) {
		if (zms[i].dead || zms[i].used == false)	continue;
		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			if (plantMap[row][k].type == 0)	continue;
			int x = 256 + k * 81;
			int x1 = x + 10;
			int x2 = x + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (plantMap[row][k].caught) {
					plantMap[row][k].deadTimer++;
					if (plantMap[row][k].deadTimer > 100) {
						plantMap[row][k].deadTimer = 0;
						plantMap[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else {
					plantMap[row][k].caught = true;
					plantMap[row][k].deadTimer = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}
	}
}
void gameFunc::updatePlant() {
	static int cnt = 0;
	if (cnt++ < 3)	return;
	cnt = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (plantMap[i][j].type > 0) {
				plantMap[i][j].frameIndex++;
				if (imgPlant[plantMap[i][j].type - 1][plantMap[i][j].frameIndex] == NULL) {
					plantMap[i][j].frameIndex = 0;
				}
			}
		}
	}
}
void gameFunc::updateGame() {
	//update the actions of plant, zombies, sunshine, and bullets
	updatePlant();
	createSunshine();
	updateSunshine();
	createZombie();
	updateZombie();
	shoot();//shoot the bullets
	updateBullets();
	collisionCheck();
}
void gameFunc::startUI() {
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;
	while (1) {
		BeginBatchDraw();
		putimagePNG(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);
		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag) {
				EndBatchDraw();
				break;
			}
		}
		EndBatchDraw();
	}
	// play backgound music
	mciSendString("play res/bg.mp3", 0, 0, 0);
}
void gameFunc::viewScence() {
	int xMin = WIDTH - imgBg.getwidth();
	int x;
	vector2 points[9] = { {550,80},{530,160},{630,170},{530,200},{515,270}
	,{565,370},{605,345},{705,280},{690,340} };
	int index[9] = { 0 };
	for (int i = 0; i < 9; i++)	index[i] = rand() % 11;
	int count = 0;
	for (x = 0; x >= xMin; x -= 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZMStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count > 9)	count = 0;
		EndBatchDraw();
		Sleep(5);
	}
	for (int i = 0; i < 100; i++) {
		BeginBatchDraw();
		putimage(xMin, 0, &imgBg);
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x, points[k].y, &imgZMStand[index[k]]);
			index[k] = (index[k] + 1) % 11;
		}
		EndBatchDraw();
		Sleep(20);
	}
	for (x = xMin; x <= 0; x += 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZMStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count > 9)	count = 0;
		EndBatchDraw();
		Sleep(5);
	}
}
void gameFunc::barsDown() {
	int height = imgBar.getheight();
	int y;
	for (y = -height; y <= 0; y++) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(250, y, &imgBar);
		for (int i = 0; i < plantCount; i++) {
			int x1 = 338 + i * 65;
			int y1 = 6 + y;
			putimage(x1, y1, &imgCards[i]);
		}
		EndBatchDraw();
	}
}
bool gameFunc::checkGame() {
	if (gameStatus == WIN) {
		Sleep(1000);
		MessageBox(NULL, "YOU HAVE WON THE GAME", "GAME OVER!", MB_SETFOREGROUND);
		return true;
	}
	else if (gameStatus == FAIL) {
		Sleep(2000);
		MessageBox(NULL, "YOU HAVE LOST THE GAME", "GAME OVER!", MB_SETFOREGROUND);
		return true;
	}
	else return false;
}