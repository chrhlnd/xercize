#include <iostream>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <time.h>

using namespace std;

vector<char> board = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const char* disp = "_*O";

const char* place = "0123456789";

void printBoard() {
	cout << "-------------------" << endl;
	int i = 0;
	for (auto &v : board) {
		if ( i > 0 && (i % 3) == 0 ) {
			cout << ":" << endl;
		}

		auto show = v == 0 ? place[i] : disp[v];

		cout << ":" << show;
		i++;
	}

	cout << ":" << endl;
}

vector<int> getPickables() {
	vector<int> picks;
	for (auto it = board.begin(); it != board.end(); ++it) {
		if (*it == 0) {
			picks.push_back(it - board.begin());
		}
	}

	return picks;
}

int countFree() {
	int free = 0;
	for (auto &v : board) {
		free += v == 0 ? 1 : 0;
	}
	return free;
}

int aiPick(int forPlayer, vector<int>& picks);

int getInput(int player, int aiPlayer) {
	int ret = 0;

	while (true) {
		cout << "*Player";
		cout << (player == 0 ? "1" : "2") << "* : Make your pick!"  << endl;

		cout << "(";

		auto picks = getPickables();

		for (auto it = picks.begin(); it != picks.end(); ++it) {
			cout << *it;
			if (it + 1 != picks.end()) {
				cout << ",";
			}
		}
		cout << ") : ";

		if (player != aiPlayer) {
			cin >> ret;
		} else {
			ret = aiPick(player, picks);
			cout << ret << endl;
		}

		if (find(picks.begin(),picks.end(),ret) != picks.end()) {
			break;
		}

		cout << " Error, invalid pick " << endl;
		if (aiPlayer == player) {
			exit(1);
			break;
		}
	}
	return ret;
}


bool hasWinner(int one, int two, int three, int &player) {
	if (board[one] != 0 && board[one] == board[two] && board[one] == board[three]) {
		player = board[one] - 1;
		return true;
	}
	return false;
}

bool hasWinner(int &winner) {
	if (hasWinner(0,1,2,winner)) return true;
	if (hasWinner(3,4,5,winner)) return true;
	if (hasWinner(6,7,8,winner)) return true;
	if (hasWinner(0,3,6,winner)) return true;
	if (hasWinner(1,4,7,winner)) return true;
	if (hasWinner(2,5,8,winner)) return true;
	if (hasWinner(0,4,8,winner)) return true;
	if (hasWinner(2,4,6,winner)) return true;
	return false;
}

void showWinner(int winner, int round) {
	cout << "!!!!!!!!!!!!!" << endl;
	printBoard();
	cout << "*Player" << winner+1 << " won, on round " << round+1 << "." << endl;
}

void showDraw(int round) {
	cout << "~~~~~~~~~~~~" << endl;
	printBoard();
	cout << "Draw on round " << round+1 << "." << endl;
}


bool canWin(int one, int two, int three, int player, int& pick) {	
	int t = board[one] + board[two] + board[three];
	int picks = board[one] > 0;
	picks += board[two] > 0;
	picks += board[three] > 0;
	if (picks > 1 && picks < 3 && t == (player+1) * 2) {
		pick = board[one] == 0 ? one
			: board[two] == 0 ? two
			: three;

		//cout << "Picking " << pick << endl;
		//cout << "  picks " << picks << "  t " << t << endl;
		return true;
	}
	return false;
}

int weightAround(int idx) {
	if (idx == 0) return (board[1] > 0 + board[4] > 0 + board[3] > 0) - 4;
	if (idx == 1) return (board[0] > 0 + board[2] > 0 + board[4] > 0) - 3;
	if (idx == 2) return (board[1] > 0 + board[4] > 0 + board[5] > 0) - 4;
	if (idx == 3) return (board[0] > 0 + board[4] > 0 + board[6] > 0) - 3;
	if (idx == 4) return (board[1] > 0 + board[3] > 0 + board[5] > 0 + board[7] > 0) - 5;
	if (idx == 5) return (board[2] > 0 + board[4] > 0 + board[8] > 0) - 3;
	if (idx == 6) return (board[3] > 0 + board[4] > 0 + board[7] > 0) - 4;
	if (idx == 7) return (board[4] > 0 + board[6] > 0 + board[8] > 0) - 3;
	if (idx == 8) return (board[4] > 0 + board[5] > 0 + board[7] > 0) - 4;
	return 99;
}

int aiPick(int forPlayer, vector<int>& picks) {
	int pick = 0;

	if (canWin(0,1,2,forPlayer,pick)) return pick;
	if (canWin(3,4,5,forPlayer,pick)) return pick;
	if (canWin(6,7,8,forPlayer,pick)) return pick;

	if (canWin(0,3,6,forPlayer,pick)) return pick;
	if (canWin(1,4,7,forPlayer,pick)) return pick;
	if (canWin(2,5,8,forPlayer,pick)) return pick;

	if (canWin(0,4,8,forPlayer,pick)) return pick;
	if (canWin(2,4,6,forPlayer,pick)) return pick;

	int opp = (forPlayer + 1) % 2;

	if (canWin(0,1,2,opp,pick)) return pick;
	if (canWin(3,4,5,opp,pick)) return pick;
	if (canWin(6,7,8,opp,pick)) return pick;

	if (canWin(0,3,6,opp,pick)) return pick;
	if (canWin(1,4,7,opp,pick)) return pick;
	if (canWin(2,5,8,opp,pick)) return pick;

	if (canWin(0,4,8,opp,pick)) return pick;
	if (canWin(2,4,6,opp,pick)) return pick;

	int low = 99;
	pick = picks[0];
	for (auto &p : picks) {
		auto val = weightAround(p);
		if (val < low) {
			low = val;
			pick = p;
		} else {
			if (val == low) {
				if (rand() > (RAND_MAX/2)) {
					pick = p;
				}
			}
		}
	}

	return pick;
}

int pickPosition() {
	cout << "Would you like to go first? ([y]/n) " << endl;
	char input;
	cin >> input;
	return (input == 'n' || input == 'N') ? 0 : 1;
}

int main() {
	srand(time(NULL));

	bool done = false;
	int round = 0;
	int winner = -1;

	int aiPlayer = -1;

	while (!done) {
		while (aiPlayer < 0) { aiPlayer = pickPosition(); cout << "Ai is " << aiPlayer; }

		cout << endl << "Round: " << round+1 << endl;
		printBoard();

		auto player = round % 2;
		auto pick   = getInput(player, aiPlayer);

		board[pick] = player + 1;

		done = hasWinner(winner);
		if (done) showWinner(winner, round);
		if (countFree() == 0) {
			done = true;
			showDraw(round);
		}
		round++;
	}
}
