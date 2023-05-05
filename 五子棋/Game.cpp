#include "Game.h"
#include <iostream>
#include<windows.h>
#include <graphics.h>
#include <fstream>
#include <math.h>
#include <vector>
#pragma comment(lib,"Winmm.lib")
#include <iomanip> // 设置输出格式
using namespace std;

#define BOARDWIDTH 690
#define BOARDHEIGHT 540
#define CHESSSIZE 36
#define SIZE 15

typedef enum {
	BLACK_PLAYER = 1,
	WHITE_PLAYER = -1,
	NONE = 0
}Player;

char blackImg[20] = "./res/black.png";
char whiteImg[20] = "./res/white.png";
char bjImg[20] = "./res/board.jpg";
char ksImg[20] = "./res/ks.png";
char gameTxt[20] = "./res/game.txt";

Player nowPlay;
IMAGE BlackImg;
IMAGE WhiteImg;
int board[SIZE][SIZE];	//棋盘
MOUSEMSG msg;

//游戏开始页面
void initGame()
{
	initgraph(BOARDWIDTH, BOARDHEIGHT);
	loadimage(0, ksImg);
	
	loadimage(&BlackImg, blackImg, CHESSSIZE, CHESSSIZE, true);
	loadimage(&WhiteImg, whiteImg, CHESSSIZE, CHESSSIZE, true);
	
	while (true)
	{
		msg = GetMouseMsg();		//鼠标信息

		if (msg.uMsg == WM_LBUTTONDOWN)
		{
			if (msg.x >= 295 && msg.y >= 303 && msg.x <= 407 && msg.y <= 347)
				startGame();
			if (msg.x >= 295 && msg.y >= 359 && msg.x <= 407 && msg.y <= 400)
				readGame();
			if (msg.x >= 295 && msg.y >= 414 && msg.x <= 407 && msg.y <= 457)
				endGame();
		}
	}
}

// 载入PNG图并去透明部分
void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //x为载入图片的X坐标，y为Y坐标
{

	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}

//初始化棋盘
void initBoard()
{
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			board[i][j] = Player::NONE;
}

//开始游戏
void startGame()
{
	cleardevice();
	loadimage(0, bjImg);
	initBoard();
	nowPlay = Player::BLACK_PLAYER;
	drawAlpha(&BlackImg, 580, 80);
	doGame();
}

//胜负判断
bool judge(int newX, int newY)
{
	int score, x, y;	//连珠数量，第x行，第y列

	// 统计横向
	score = -1;			//初始化连珠数量为-1
	x = newX;			//x赋予当前newX行

	//y列从当前newY列开始，但y大于等于0并且棋盘的x行，y--列即横向左边与当前同色，时连珠数量++。计算当前落子位置的左边有几个连珠数量。
	for (y = newY; y >= 0 && board[x][y--] == board[newX][newY]; score++);
	//y列从当前newY列开始，但y小于边界并且棋盘的x行，y++列即横向右边与当前同色，时连珠数量++。计算当前落子位置的右边有几个连珠数量。
	for (y = newY; y < SIZE && board[x][y++] == board[newX][newY]; score++);
	if (score >= 5)		//如果连珠大于等于5，则返回真
		return true;

	// 统计纵向
	score = -1;			//初始化连珠数量为-1
	y = newY;			//y赋予当前newY行
	//x行从当前newX行开始，但x大于等于0并且棋盘的x--行，y列即横向上边与当前同色，时连珠数量++。计算当前落子位置的上边有几个连珠数量。
	for (x = newX; x >= 0 && board[x--][y] == board[newX][newY]; score++);
	//x行从当前newX行开始，但x小于边界并且棋盘的x++行，y列即横向下边与当前同色，时连珠数量++。计算当前落子位置的下边有几个连珠数量。
	for (x = newX; x < SIZE && board[x++][y] == board[newX][newY]; score++);
	if (score >= 5)		//如果连珠大于等于5，则返回真
		return true;
	
	// 统计主对角	/ 方向
	score = -1;			//初始化连珠数量为-1
	//x行从当前newX行开始，y列从当前newY列开始，但x大于等于0并且y小于边界，且棋盘的x--行，y++列即对角右上角与当前同色，时连珠数量++。计算当前落子位置的右上角有几个连珠数量。
	for (x = newX, y = newY; x >= 0 && y < SIZE && board[x--][y++] == board[newX][newY]; score++);
	//x行从当前newX行开始，y列从当前newY列开始，但x小于边界并且y大于等于0，且棋盘的x++行，y--列即对角左下角与当前同色，时连珠数量++。计算当前落子位置的左下角有几个连珠数量。
	for (x = newX, y = newY; x < SIZE && y >= 0 && board[x++][y--] == board[newX][newY]; score++);
	if (score >= 5)		//如果连珠大于等于5，则返回真
		return true;
	
	// 统计副对角	\ 方向
	score = -1;			//初始化连珠数量为-1
	//x行从当前newX行开始，y列从当前newY列开始，但x大于等于0并且y大于等于0，且棋盘的x--行，y--列即对角左上角与当前同色，时连珠数量++。计算当前落子位置的左上角有几个连珠数量。
	for (x = newX, y = newY; x >= 0 && y >= 0 && board[x--][y--] == board[newX][newY]; score++);
	//x行从当前newX行开始，y列从当前newY列开始，但x小于边界并且y小于边界，且棋盘的x++行，y++列即对角右下角与当前同色，时连珠数量++。计算当前落子位置的右下角有几个连珠数量。
	for (x = newX, y = newY; x < SIZE && y < SIZE && board[x++][y++] == board[newX][newY]; score++);
	if (score >= 5)		//如果连珠大于等于5，则返回真
		return true;

	return false;		//如果以上都判断为否，则返回假
}

//点击棋盘
void doChess(int dx, int dy) {
	//因为行列和像素的dx和y的方向是相反的，所以x方向计算得列，y方向得行。
	//row列等于取当前dx减去边界23除以35，得当前位置为列数。但点击位置更靠近下一格时，应取+1格，所以当前dx减去棋盘边界23像素除以棋盘每格大小35像素大于半格大小时，取值+1
	int row = (dx - 23) % 35 > 35 / 2 ? (((dx - 23) / 35) + 1) : ((dx - 23) / 35);
	//col行等于取当前dy减去边界23除以35，得当前位置为行数。但点击位置更靠近下一格时，应取+1格，所以当前dy减去棋盘边界23像素除以棋盘每格大小35像素大于半格大小时，取值+1
	int col = (dy - 23) % 35 > 35 / 2 ? (((dy - 23) / 35) + 1) : ((dy - 23) / 35);
	//根据计算出的行乘以每格大小35.428，计算落棋点的zY
	int zY = 23 + (int)(col * 35.428);
	//根据计算出的列乘以每格大小35.428，计算落棋点的zX
	int zX = 23 + (int)(row * 35.428);
	//offSet用以取半个棋子大小算落棋的有效边界距离
	int offSet = (int)(30 * 0.5);
	//根据落棋点的中心的zX与zY，和点击位置dx与dy。利用勾股定理计算点击位置与落子中心的距离
	int len = (int)sqrt((dx - zX) * (dx - zX) +
					(dy - zY) * (dy - zY));
	//但点击距离小于有效边界距离，表示点击有效
	if (len < offSet)
	{
		//但落子位置不为空
		if (board[col][row] != Player::NONE)
		{
			//弹窗提示该位置有子了
			MessageBox(NULL, "该位置已有子", "下棋错误", MB_OK | MB_SYSTEMMODAL);
			return;
		}
		//记录有效落子
		board[col][row] = nowPlay;
		//当当前棋手为黑子
		if (nowPlay == Player::BLACK_PLAYER)
			drawAlpha(&BlackImg, zX - 15, zY - 15);//绘制黑子
		//当当前棋手为白子
		if(nowPlay == Player::WHITE_PLAYER)
			drawAlpha(&WhiteImg, zX - 15, zY - 15);//绘制白子
		//判断胜负
		if (judge(col, row))
			successGame();//游戏胜利方法
		//交换棋手，当当前棋手为黑子，则改为白子，否则改为黑子。
		nowPlay = nowPlay == Player::BLACK_PLAYER ? Player::WHITE_PLAYER : Player::BLACK_PLAYER;
		//更改当前棋手图标
		if (nowPlay == Player::BLACK_PLAYER)
			drawAlpha(&BlackImg, 580, 80);//绘制黑子
		//更改当前棋手图标
		else if (nowPlay == Player::WHITE_PLAYER)
			drawAlpha(&WhiteImg, 580, 80);//绘制白子
	}
}

//读取游戏
void readGame()
{
	ifstream rfile;
	rfile.open(gameTxt, ios::in);
	if (!rfile.is_open())
	{
		MessageBox(NULL, "读取游戏失败", "读取游戏", MB_OK | MB_SYSTEMMODAL);
		endGame();
	}
	int i, j;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			rfile >> board[i][j];
			cout << board[i][j] << " ";
		}
		cout << endl;
	}
	int n;
	rfile >> n;
	nowPlay = n == Player::BLACK_PLAYER ? Player::BLACK_PLAYER : Player::WHITE_PLAYER;
	rfile.close();

	cleardevice();
	loadimage(0, bjImg);
	if(nowPlay == Player::BLACK_PLAYER)
		drawAlpha(&BlackImg, 580, 80);
	else
		drawAlpha(&WhiteImg, 580, 80);
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			int y = (23 + (int)(i * 35.428)) - 15;
			int x = (23 + (int)(j * 35.428)) - 15;
			if (board[i][j] == Player::BLACK_PLAYER) {
				drawAlpha(&BlackImg, x, y);
			}
			if (board[i][j] == Player::WHITE_PLAYER) {
				drawAlpha(&WhiteImg, x, y);
			}
		}
	}
	doGame();
}

//进行游戏中
void doGame()
{
	while (true)
	{
		msg = GetMouseMsg();		//鼠标信息
		//鼠标按下
		if (msg.uMsg == WM_LBUTTONDOWN)
		{	
			//下棋
			if (msg.x >= 0 && msg.y >= 0 && msg.x <= 540 && msg.y <= 540)
			{
				doChess(msg.x, msg.y);
			}
			//重新开始
			if (msg.x >= 547 && msg.y >= 335 && msg.x <= 678 && msg.y <= 385)
			{
				startGame();
			}
			//保存游戏
			if (msg.x >= 547 && msg.y >= 398 && msg.x <= 678 && msg.y <= 450)
			{
				if (saveGame())
				{
					MessageBox(NULL, "保存成功", "保存游戏", MB_OK | MB_SYSTEMMODAL);
				}
				else
				{
					MessageBox(NULL, "保存失败", "保存游戏", MB_OK | MB_SYSTEMMODAL);
				}
			}
			//退出游戏
			if (msg.x >= 547 && msg.y >= 464 && msg.x <= 678 && msg.y <= 516)
				endGame();
		}
	}
}

//保存游戏
bool saveGame()
{
	ofstream outfile;
	outfile.open(gameTxt, ios::out | ios::trunc);
	if (!outfile.is_open())
	{
		return false;
	}
	int i, j;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			outfile << board[i][j] << " ";
		}
		outfile << endl;
	}
	outfile << nowPlay << endl;
	outfile.close();
	return true;
}

//游戏胜利
void successGame() {
	if(nowPlay == Player::BLACK_PLAYER)
		MessageBox(NULL, "黑子胜", "游戏结束", MB_OK | MB_SYSTEMMODAL);
	if (nowPlay == Player::WHITE_PLAYER)
		MessageBox(NULL, "白子胜", "游戏结束", MB_OK | MB_SYSTEMMODAL);
	while (true)
	{
		msg = GetMouseMsg();		//鼠标信息
		//鼠标按下
		if (msg.uMsg == WM_LBUTTONDOWN)
		{
			//重新开始
			if (msg.x >= 547 && msg.y >= 335 && msg.x <= 678 && msg.y <= 385)
			{
				startGame();
			}
			//保存游戏
			if (msg.x >= 547 && msg.y >= 398 && msg.x <= 678 && msg.y <= 450)
			{
				MessageBox(NULL, "以结束无法保存", "保存游戏", MB_OK | MB_SYSTEMMODAL);
			}
			//退出游戏
			if (msg.x >= 547 && msg.y >= 464 && msg.x <= 678 && msg.y <= 516)
				endGame();
		}
	}
}

//退出游戏
void endGame()
{
	closegraph();
	exit(0);
}
