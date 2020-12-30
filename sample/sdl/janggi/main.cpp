#include "SDL.h"
#include <stdlib.h>
#include "SDLSingleton.h"
#include "CGameCore.h"
#include <assert.h>

int main(int argc, char** argv)
{
	if (SDLSingleton::GetInstance()->InitSystem() == false)
		assert(0);

	CGameCore::GetInstance()->Initialize();
	bool running = true;

	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				CGameCore::GetInstance()->ProcessInput(event.key.keysym.scancode);
			}

			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_FINGERUP)
			{

				float fingerX = event.tfinger.x;
				float fingerY = event.tfinger.y;

				CGameCore::GetInstance()->ProcessInputWithTouch(fingerX, fingerY);
			}
		}

		CGameCore::GetInstance()->ProcessGame();

		SDL_RenderClear(SDLSingleton::GetInstance()->GetRenderer());
		CGameCore::GetInstance()->Render();
		SDL_RenderPresent(SDLSingleton::GetInstance()->GetRenderer());
	}



	return 0;
}


/*std::string getCoordString(MV mv)
{
	int sqSrc = SRC(mv);
	int sqDst = DST(mv);

	int from_x = GET_X(sqSrc);
	int from_y = GET_Y(sqSrc);
	int to_x = GET_X(sqDst);
	int to_y = GET_Y(sqDst);

	char szFromY[256];
	itoa(from_y, 10, szFromY);
	char szToY[256];
	itoa(to_y, 10, szToY);

	std::string pos;
	pos = coordx[from_x] + szFromY;
	pos += coordx[to_x] + szToY;

	return pos;
}

void printAllLegalMoves(Search* search, SIDE side) {
	MV mvs[50] = { '0' };
	int ret;

	cout << "All possible moves:" << "\n";
	ret = search->generateMoves(side, mvs);
	if (ret < 0) {
		cout << "generateMoves ERROR ret:" << ret;
		return;
	}

	int i = 0;

	do {
	string pos = getCoordString(mvs[i]);
		cout << pos.c_str() << " ";
		i++;
	} while (mvs[i] != 0);

	cout << "\n";
}

void printLegalMoves(Search* search, std::string command) {
	MV mvs[50] = { '0' };
	int ret;

	int from_x = command[0] - 'a';
	int from_y = command[1] - '0';

	cout << "Possible moves:" << "\n";
	ret = search->getNextPieceMoves(from_x, from_y, mvs);
	if (ret < 0) {
		cout << "getNextPieceMoves ERROR ret:" << ret;
		return;
	}

	int i = 0;

	do {
	string pos = getCoordString(mvs[i]);
		cout << pos.c_str() << " ";
		i++;
	} while (mvs[i] != 0);

	cout << "\n";
}

int makeHumanMove(Search* search, SIDE side, std::string command) {
	int from_x = command[0] - 'a';
	int from_y = command[1] - '0';
	int to_x = command[2] - 'a';
	int to_y = command[3] - '0';

	if (side != search->GetSideAt(from_x, from_y)) {
		cout << "Can`t move that piece." << "\n";
		return -1;
	}

	int ret = search->makeMove(from_x, from_y, to_x, to_y);
	if (ret < 0) {
		cout << "makeMove ERROR ret:" << ret << "\n";
		return -1;
	}

	return 1;
}

void makeMove(Search* search, MV mv) {
	int sqSrc = SRC(mv);
	int sqDst = DST(mv);

	int from_x = GET_X(sqSrc);
	int from_y = GET_Y(sqSrc);
	int to_x = GET_X(sqDst);
	int to_y = GET_Y(sqDst);

	int ret = search->makeMove(from_x, from_y, to_x, to_y);
	if (ret < 0) {
		cout << "makeMove ERROR ret:" << ret;
	}
}

void moveBackLastMove(Search* search) {
	if (!search->moveBack())
		cout << "moveBack  ERROR" << "\n";
	if (!search->moveBack())
		cout << "moveBack  ERROR" << "\n";
}


int main2(int argc, char* argv[]) {


	playerSide = SD_HAN;
	computerSide = SD_CHO;

	while (1) {
		cout << "\n";
		search.print();
		cout << "\n";
		cout << "[" << search.zobristKey << "] [" << search.zobristLock << "]" << "\n";

		if (currentTurn == playerSide) {
			cout << sdstring[currentTurn] << " move? ";
			cin >> raw;
			cout << "\n";

		string command(raw);

			if (command == "l") {
				printAllLegalMoves(&search, currentTurn);
				continue;
			}
			else if (command == "back") {
				moveBackLastMove(&search);
				continue;
			}
			else if (command == "exit")
				return 0;


			if (command.length() == 2) {
				printLegalMoves(&search, command);
				continue;
			}


			int ret = makeHumanMove(&search, playerSide, command);
			if (ret < 0) continue;

			currentTurn = (SD_HAN + 1) - currentTurn;
		}
		else {
			cout << "Computer thinking......" << "\n";

			int commv = search.getBestMove(computerSide, 0);
			cout << getCoordString(commv).c_str() << "\n";
			makeMove(&search, commv);
			currentTurn = (SD_HAN + 1) - currentTurn;
		}
	}

	return 0;
}*/
