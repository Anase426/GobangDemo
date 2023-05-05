#include "Game.h"
#include <iostream>
#include<windows.h>
#include <graphics.h>
#include <fstream>
#include <math.h>
#include <vector>
#pragma comment(lib,"Winmm.lib")
#include <iomanip> // ���������ʽ
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
int board[SIZE][SIZE];	//����
MOUSEMSG msg;

//��Ϸ��ʼҳ��
void initGame()
{
	initgraph(BOARDWIDTH, BOARDHEIGHT);
	loadimage(0, ksImg);
	
	loadimage(&BlackImg, blackImg, CHESSSIZE, CHESSSIZE, true);
	loadimage(&WhiteImg, whiteImg, CHESSSIZE, CHESSSIZE, true);
	
	while (true)
	{
		msg = GetMouseMsg();		//�����Ϣ

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

// ����PNGͼ��ȥ͸������
void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //xΪ����ͼƬ��X���꣬yΪY����
{

	// ������ʼ��
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()���������ڻ�ȡ��ͼ�豸���Դ�ָ�룬EASYX�Դ�
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //��ȡpicture���Դ�ָ��
	int picture_width = picture->getwidth(); //��ȡpicture�Ŀ�ȣ�EASYX�Դ�
	int picture_height = picture->getheight(); //��ȡpicture�ĸ߶ȣ�EASYX�Դ�
	int graphWidth = getwidth();       //��ȡ��ͼ���Ŀ�ȣ�EASYX�Դ�
	int graphHeight = getheight();     //��ȡ��ͼ���ĸ߶ȣ�EASYX�Դ�
	int dstX = 0;    //���Դ������صĽǱ�

	// ʵ��͸����ͼ ��ʽ�� Cp=��p*FP+(1-��p)*BP �� ��Ҷ˹���������е���ɫ�ĸ��ʼ���
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //���Դ������صĽǱ�
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA��͸����
			int sr = ((src[srcX] & 0xff0000) >> 16); //��ȡRGB���R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //���Դ������صĽǱ�
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //��ʽ�� Cp=��p*FP+(1-��p)*BP  �� ��p=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //��p=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //��p=sa/255 , FP=sb , BP=db
			}
		}
	}
}

//��ʼ������
void initBoard()
{
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			board[i][j] = Player::NONE;
}

//��ʼ��Ϸ
void startGame()
{
	cleardevice();
	loadimage(0, bjImg);
	initBoard();
	nowPlay = Player::BLACK_PLAYER;
	drawAlpha(&BlackImg, 580, 80);
	doGame();
}

//ʤ���ж�
bool judge(int newX, int newY)
{
	int score, x, y;	//������������x�У���y��

	// ͳ�ƺ���
	score = -1;			//��ʼ����������Ϊ-1
	x = newX;			//x���赱ǰnewX��

	//y�дӵ�ǰnewY�п�ʼ����y���ڵ���0�������̵�x�У�y--�м���������뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ�����м�������������
	for (y = newY; y >= 0 && board[x][y--] == board[newX][newY]; score++);
	//y�дӵ�ǰnewY�п�ʼ����yС�ڱ߽粢�����̵�x�У�y++�м������ұ��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ��ұ��м�������������
	for (y = newY; y < SIZE && board[x][y++] == board[newX][newY]; score++);
	if (score >= 5)		//���������ڵ���5���򷵻���
		return true;

	// ͳ������
	score = -1;			//��ʼ����������Ϊ-1
	y = newY;			//y���赱ǰnewY��
	//x�дӵ�ǰnewX�п�ʼ����x���ڵ���0�������̵�x--�У�y�м������ϱ��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ��ϱ��м�������������
	for (x = newX; x >= 0 && board[x--][y] == board[newX][newY]; score++);
	//x�дӵ�ǰnewX�п�ʼ����xС�ڱ߽粢�����̵�x++�У�y�м������±��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ��±��м�������������
	for (x = newX; x < SIZE && board[x++][y] == board[newX][newY]; score++);
	if (score >= 5)		//���������ڵ���5���򷵻���
		return true;
	
	// ͳ�����Խ�	/ ����
	score = -1;			//��ʼ����������Ϊ-1
	//x�дӵ�ǰnewX�п�ʼ��y�дӵ�ǰnewY�п�ʼ����x���ڵ���0����yС�ڱ߽磬�����̵�x--�У�y++�м��Խ����Ͻ��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ����Ͻ��м�������������
	for (x = newX, y = newY; x >= 0 && y < SIZE && board[x--][y++] == board[newX][newY]; score++);
	//x�дӵ�ǰnewX�п�ʼ��y�дӵ�ǰnewY�п�ʼ����xС�ڱ߽粢��y���ڵ���0�������̵�x++�У�y--�м��Խ����½��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ����½��м�������������
	for (x = newX, y = newY; x < SIZE && y >= 0 && board[x++][y--] == board[newX][newY]; score++);
	if (score >= 5)		//���������ڵ���5���򷵻���
		return true;
	
	// ͳ�Ƹ��Խ�	\ ����
	score = -1;			//��ʼ����������Ϊ-1
	//x�дӵ�ǰnewX�п�ʼ��y�дӵ�ǰnewY�п�ʼ����x���ڵ���0����y���ڵ���0�������̵�x--�У�y--�м��Խ����Ͻ��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ����Ͻ��м�������������
	for (x = newX, y = newY; x >= 0 && y >= 0 && board[x--][y--] == board[newX][newY]; score++);
	//x�дӵ�ǰnewX�п�ʼ��y�дӵ�ǰnewY�п�ʼ����xС�ڱ߽粢��yС�ڱ߽磬�����̵�x++�У�y++�м��Խ����½��뵱ǰͬɫ��ʱ��������++�����㵱ǰ����λ�õ����½��м�������������
	for (x = newX, y = newY; x < SIZE && y < SIZE && board[x++][y++] == board[newX][newY]; score++);
	if (score >= 5)		//���������ڵ���5���򷵻���
		return true;

	return false;		//������϶��ж�Ϊ���򷵻ؼ�
}

//�������
void doChess(int dx, int dy) {
	//��Ϊ���к����ص�dx��y�ķ������෴�ģ�����x���������У�y������С�
	//row�е���ȡ��ǰdx��ȥ�߽�23����35���õ�ǰλ��Ϊ�����������λ�ø�������һ��ʱ��Ӧȡ+1�����Ե�ǰdx��ȥ���̱߽�23���س�������ÿ���С35���ش��ڰ���Сʱ��ȡֵ+1
	int row = (dx - 23) % 35 > 35 / 2 ? (((dx - 23) / 35) + 1) : ((dx - 23) / 35);
	//col�е���ȡ��ǰdy��ȥ�߽�23����35���õ�ǰλ��Ϊ�����������λ�ø�������һ��ʱ��Ӧȡ+1�����Ե�ǰdy��ȥ���̱߽�23���س�������ÿ���С35���ش��ڰ���Сʱ��ȡֵ+1
	int col = (dy - 23) % 35 > 35 / 2 ? (((dy - 23) / 35) + 1) : ((dy - 23) / 35);
	//���ݼ�������г���ÿ���С35.428������������zY
	int zY = 23 + (int)(col * 35.428);
	//���ݼ�������г���ÿ���С35.428������������zX
	int zX = 23 + (int)(row * 35.428);
	//offSet����ȡ������Ӵ�С���������Ч�߽����
	int offSet = (int)(30 * 0.5);
	//�������������ĵ�zX��zY���͵��λ��dx��dy�����ù��ɶ��������λ�����������ĵľ���
	int len = (int)sqrt((dx - zX) * (dx - zX) +
					(dy - zY) * (dy - zY));
	//���������С����Ч�߽���룬��ʾ�����Ч
	if (len < offSet)
	{
		//������λ�ò�Ϊ��
		if (board[col][row] != Player::NONE)
		{
			//������ʾ��λ��������
			MessageBox(NULL, "��λ��������", "�������", MB_OK | MB_SYSTEMMODAL);
			return;
		}
		//��¼��Ч����
		board[col][row] = nowPlay;
		//����ǰ����Ϊ����
		if (nowPlay == Player::BLACK_PLAYER)
			drawAlpha(&BlackImg, zX - 15, zY - 15);//���ƺ���
		//����ǰ����Ϊ����
		if(nowPlay == Player::WHITE_PLAYER)
			drawAlpha(&WhiteImg, zX - 15, zY - 15);//���ư���
		//�ж�ʤ��
		if (judge(col, row))
			successGame();//��Ϸʤ������
		//�������֣�����ǰ����Ϊ���ӣ����Ϊ���ӣ������Ϊ���ӡ�
		nowPlay = nowPlay == Player::BLACK_PLAYER ? Player::WHITE_PLAYER : Player::BLACK_PLAYER;
		//���ĵ�ǰ����ͼ��
		if (nowPlay == Player::BLACK_PLAYER)
			drawAlpha(&BlackImg, 580, 80);//���ƺ���
		//���ĵ�ǰ����ͼ��
		else if (nowPlay == Player::WHITE_PLAYER)
			drawAlpha(&WhiteImg, 580, 80);//���ư���
	}
}

//��ȡ��Ϸ
void readGame()
{
	ifstream rfile;
	rfile.open(gameTxt, ios::in);
	if (!rfile.is_open())
	{
		MessageBox(NULL, "��ȡ��Ϸʧ��", "��ȡ��Ϸ", MB_OK | MB_SYSTEMMODAL);
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

//������Ϸ��
void doGame()
{
	while (true)
	{
		msg = GetMouseMsg();		//�����Ϣ
		//��갴��
		if (msg.uMsg == WM_LBUTTONDOWN)
		{	
			//����
			if (msg.x >= 0 && msg.y >= 0 && msg.x <= 540 && msg.y <= 540)
			{
				doChess(msg.x, msg.y);
			}
			//���¿�ʼ
			if (msg.x >= 547 && msg.y >= 335 && msg.x <= 678 && msg.y <= 385)
			{
				startGame();
			}
			//������Ϸ
			if (msg.x >= 547 && msg.y >= 398 && msg.x <= 678 && msg.y <= 450)
			{
				if (saveGame())
				{
					MessageBox(NULL, "����ɹ�", "������Ϸ", MB_OK | MB_SYSTEMMODAL);
				}
				else
				{
					MessageBox(NULL, "����ʧ��", "������Ϸ", MB_OK | MB_SYSTEMMODAL);
				}
			}
			//�˳���Ϸ
			if (msg.x >= 547 && msg.y >= 464 && msg.x <= 678 && msg.y <= 516)
				endGame();
		}
	}
}

//������Ϸ
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

//��Ϸʤ��
void successGame() {
	if(nowPlay == Player::BLACK_PLAYER)
		MessageBox(NULL, "����ʤ", "��Ϸ����", MB_OK | MB_SYSTEMMODAL);
	if (nowPlay == Player::WHITE_PLAYER)
		MessageBox(NULL, "����ʤ", "��Ϸ����", MB_OK | MB_SYSTEMMODAL);
	while (true)
	{
		msg = GetMouseMsg();		//�����Ϣ
		//��갴��
		if (msg.uMsg == WM_LBUTTONDOWN)
		{
			//���¿�ʼ
			if (msg.x >= 547 && msg.y >= 335 && msg.x <= 678 && msg.y <= 385)
			{
				startGame();
			}
			//������Ϸ
			if (msg.x >= 547 && msg.y >= 398 && msg.x <= 678 && msg.y <= 450)
			{
				MessageBox(NULL, "�Խ����޷�����", "������Ϸ", MB_OK | MB_SYSTEMMODAL);
			}
			//�˳���Ϸ
			if (msg.x >= 547 && msg.y >= 464 && msg.x <= 678 && msg.y <= 516)
				endGame();
		}
	}
}

//�˳���Ϸ
void endGame()
{
	closegraph();
	exit(0);
}
