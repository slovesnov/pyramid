#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>
#include <algorithm>
#include <map>
#include "Permutations.h"

using uchar= unsigned char;
using VInt = std::vector<int>;

const char *FILEN = "table.tbl";
const int NPOS = 81 * 16 * 720; // 6^4*6! = 933'120
const char *A[] = { "wbr", "yrb", "ybw", "ywr" };
const int B[][3] = { { 11, 21, 15 }, { 3, 17, 19 }, { 5, 23, 9 }, { 1, 7, 13 } };
const char *C[] = { "rb", "wb", "wr", "yr", "yw", "yb" };
const int D[][2] = { { 16, 18 }, { 10, 22 }, { 8, 12 }, { 0, 14 }, { 2, 6 }, {
		4, 20 } };
const int M[][3] = { { 0, 2, 1 }, { 3, 0, 5 }, { 1, 4, 5 }, { 4, 2, 3 }, { 2, 0,
		1 }, { 0, 3, 5 }, { 4, 1, 5 }, { 2, 4, 3 } };

int perm[6][6][6][6][6];
char gm[NPOS];
//map<period, map<layer,count>>
std::map<int, std::map<int, int>> periodMap;

//same time with or without LASTMOVE
//#define LASTMOVE

struct Position {
	/*sa, sb, sc, sd - states of the centers
	 p[i] - position of the twos in the i-th position
	 o[i] - orientation of the twos in the i-th position*/
	uchar sa, sb, sc, sd, p[6], o[6];
#ifdef LASTMOVE
	uchar l;
#endif
	int code() {
		int c = (27 * sa + 9 * sb + 3 * sc + sd
				+ 81 * perm[p[0]][p[1]][p[2]][p[3]][p[4]]) << 5;
		for (int i = 0; i < 5; ++i) {
			if (o[i])
				c |= 1 << i;
		}
		return c;
	}

	void operator=(Position &p) {
		std::memcpy(this, &p, sizeof(Position));
	}

	void move(int i) {
		const int *a = M[i];
		uchar *c[] = { &sa, &sb, &sc, &sd };
		uchar t = p[a[0]];
		p[a[0]] = p[a[1]];
		p[a[1]] = p[a[2]];
		p[a[2]] = t;

		t = o[a[0]];
		o[a[0]] = o[a[1]];
		o[a[1]] = !o[a[2]];
		o[a[2]] = !t;

		uchar *q = c[i & 3];
		*q = (*q + (i < 4 ? 1 : 2)) % 3;
#ifdef LASTMOVE
		l = i & 3;
#endif
	}

	std::string set(std::string const &s) {
		int i, j, k;
		char c1, c2;

		if (s.length() != 24) {
			return "invalid string length";
		}
		for (i = 0; i < 24; i++) {
			if (!strchr("brwy", s[i])) {
				return "invalid char found at position " + std::to_string(i);
			}
		}

		uchar *c[] = { &sa, &sb, &sc, &sd };
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 3; j++) {
				k = (i + j) % 3;
				if (s[B[i][0]] == A[i][k]) {
					if (s[B[i][1]] != A[i][(k + 1) % 3]
							|| s[B[i][2]] != A[i][(k + 2) % 3]) {
						return "impossible position" + std::to_string(k);
					}
					*c[i] = k;
					break;
				}
			}
		}

		for (i = 0; i < 6; i++) {
			c1 = s[D[i][0]];
			c2 = s[D[i][1]];
			for (j = 0; j < 6; j++) {
				auto q = C[j];
				if ((c1 == q[0] && c2 == q[1]) || (c1 == q[1] && c2 == q[0])) {
					p[i] = j;
					o[i] = c1 == q[1];
					break;
				}
			}
		}

		for (i = j = 0; i < 6; ++i) {
			if (o[i])
				j = !j;
		}
		if (j) {
			return "impossible position sum(o[]) is odd.";
		}
		for (i = 0; i < 6; ++i) {
			j = std::find(p + i + 1, p + 6, p[i]) - p;
			if (j != 6) {
				return "impossible position p[" + std::to_string(i) + "]=p["
						+ std::to_string(j) + "].";
			}
		}

		return "";
	}

	void setStart() {
		memset(this, 0, sizeof(Position));
		for (int i = 1; i < 6; ++i) {
			p[i] = i;
		}
#ifdef LASTMOVE
		l = -1;
#endif

	}

	bool isStart() {
		if (sa || sb || sc || sd)
			return false;

		for (int i = 0; i < 6; ++i) {
			if (p[i] != i || o[i])
				return false;
		}
		return true;
	}

	bool isSpecial() {
		/* First the yellow edge is entered, we assume that the pyramid is standing on the blue edge
		 the white edge is entered, we assume that the pyramid is standing on the blue edge
		 the red edge is entered, we assume that the pyramid is standing on the blue edge
		 the blue edge is entered, we assume that the pyramid is standing on the white edge
		 */
		const int type = 1;
		size_t i;
		std::string s = toString();
		if (type) {
			int j;
			const char c[] = "ywrb";
			for (i = 0; i < strlen(c); i++)
				for (j = 0; j < 6; j += 2)
					if (s[6 * i + j] != c[i])
						return false;
			return true;
		} else {
			const char c[] = "ywr";
			for (i = 0; i < strlen(c); i++)
				for (auto e : { 1, 3, 4, 5 })
					if (s[e + i * 6] != c[i])
						return false;

			for (i = 18; i < 24; i++)
				if (s[i] != 'b')
					return false;

			return true;
		}
	}

	std::string toString() {
		char s[25];
		s[24] = 0;
		int i, j;

		uchar c[] = { sa, sb, sc, sd };
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 3; j++) {
				s[B[i][j]] = A[i][(j + c[i]) % 3];
			}
		}

		for (i = 0; i < 6; i++) {
			s[D[i][0]] = C[p[i]][o[i]];
			s[D[i][1]] = C[p[i]][!o[i]];
		}

		return s;
	}
};

VInt solven(std::string const &s) {
	VInt v;
	std::string r;
	Position t;
	r = t.set(s);
	if (!r.empty()) {
		printf("%s\n", r.c_str());
		return v;
	}

	int i, n = gm[t.code()];
	while (n > 0) {
		for (i = 0; i < 8; ++i) {
			t.move(i);
			if (gm[t.code()] == n - 1) {
				n--;
				v.push_back(i);
				break;
			}
			t.move((i + 4) & 7);
		}
	}
	return v;
}

/* First the yellow edge is entered, we assume that the pyramid is standing on the blue edge
 the white edge is entered, we assume that the pyramid is standing on the blue edge
 the red edge is entered, we assume that the pyramid is standing on the blue edge
 the blue edge is entered, we assume that the pyramid is standing on the white edge
 */
std::string solve(std::string const &s, int option = 0) {
	std::string r;
	std::string ss[][4] = { { "yellow", "white", "red", "blue" }, { "y", "w",
			"r", "b" } };
	std::string *q = ss[option];
	VInt v = solven(s);
	int n = v.size();
	if (option == 0)
		r = std::to_string(n) + " move" + (n == 1 ? "" : "s");

	for (int i : v) {
		if (option == 0)
			r += std::string(" ");
		r += q[i & 3] + (i > 3 ? "'" : "");

	}
	return r;
}

int period(std::string const &s) {
	VInt v = solven(s); //same period of rotation string and inverted rotation string
	int n = 0; //period for empty=1
	Position t;
	t.setStart();
	do {
		for (auto e : v) {
			t.move(e);
		}
		n++;
	} while (!t.isStart());
	return n;
}

void showPeriodMap() {
	printf("periods");
	for (auto &e : periodMap) {
		printf(" %d", e.first);
		bool f = true;
		for (auto &e1 : e.second) {
			printf("%c%d:%d", f ? '{' : ' ', e1.first, e1.second);
			f = false;
		}
		printf("}");
	}
	fflush(stdout);
}

void tree() {
	const int option = 0; /*
	 * 0 - simple
	 * 1 - show last layer
	 * 2 - find special positions
	 * 3 - calculate periods
	 */
	const bool quotes = true; //for option 1|2
	const char NMAX = 20;
	const int MAXL = 480'467;
	int i, j, k, n = 1, code, c1 = 1, c2;
	clock_t tv1 = clock(), tv2;
	Position t;
	auto p1 = new Position[MAXL];
	auto p2 = new Position[MAXL];
	char m[NPOS] = { 0 };
	std::fill(m + 1, m + NPOS, NMAX);
	//set layer 1
	p1->setStart();
	printf("layer 0");
	std::vector<std::string> v;
	std::string r;
	int p;
	if (option == 3)
		periodMap.insert( { 1, { { 0, 1 } } });
	for (i = 1; c1; ++i) { // i - layer
		tv2 = clock();
		printf(" n=%6d [%6d] total time=%.2lf", n, c1,
				double(tv2 - tv1) / CLOCKS_PER_SEC);
//		if (option == 3)
//			showPeriodMap();
		printf("\nlayer%2d", i);
		fflush(stdout);

		c2 = 0;
		for (k = 0; k < c1; ++k) {

			//make moves
			for (j = 0; j < 8; j++) {
#ifdef LASTMOVE
				if (p1[k].l == (j & 3)) {
					continue;
				}
#endif
				//load position
				t = p1[k];
				t.move(j);
				code = t.code();

				if (m[code] == NMAX) { //this position has not existed yet
					m[code] = i;
					n++;
					//write position
					p2[c2++] = t;
					if (option == 3) {
						r = t.toString();
						p = period(r);
						auto it = periodMap.find(p);
						if (it == periodMap.end()) {
							periodMap.insert( { p, { { i, 1 } } });
						} else {
							auto &m = it->second;
							auto it1 = m.find(i);
							if (it1 == m.end()) {
								m.insert( { i, 1 });
							} else {
								it1->second++;
							}
						}
					} else if ((option == 1 && i == 11)
							|| (option == 2 && t.isSpecial())) {
						v.push_back(t.toString());
					}
				}
			} //for(j)
		} //for(k)

		//swap layers
		std::swap(p1, p2);
		c1 = c2;
	} //for(i)
	printf(" n=%6d [%6d] no next layer\n", n, 0);
	if (option == 3)
		showPeriodMap();

	if (option == 1 || option == 2) {
		size_t i = 0;
		for (auto &e : v) {
			i++;
			r = solve(e, 1);
			printf("%s%s %s%s\n", quotes ? "\"" : "", e.c_str(), r.c_str(),
					quotes ? (i == v.size() ? "\"" : "\",") : "");
		}
		printf("total %llu\n", v.size());
	}

	delete[] p1;
	delete[] p2;

	FILE *f = fopen(FILEN, "wb+");
	fwrite(m, 1, NPOS, f);
	fclose(f);
}

int main() {
	const int option = 0;
	/* option = 0 store tree to FILEN
	 * option = 1 solve test positions
	 * option = 2 solve the position entered from the keyboard
	 * option = 3 test
	 */
	std::string ss[] = { "yyyyyywwwwwwrrrrrrbbbbbb", "yyyyyywwrwrrbrrbbrwbbwwb",
			"yyyywywwrwwwbrrryrbbrbbb", "bybybyrwrwrwwrwrwrybybyb" };

	int c, i;
	std::string r;
	std::vector<int> m;
	bool even;

	FILE *f = fopen(FILEN, "rb");
	if (f) {
		fread(gm, 1, NPOS, f);
		fclose(f);
	}
	//initialize permutations
	c = 0;
	Permutations pe(6, 6, Permutations::PERMUTATIONS_WITHOUT_REPLACEMENTS);
	for (auto &p : pe) { //p - std::vector<int>
		//test whether even
		even = true;
		m = p;
		for (i = 0; i < 6; ++i) {
			if (m[i] != i) {
				auto a = std::find(m.begin(), m.end(), i);
				std::swap(*a, m[i]);
				even = !even;
			}
		}
		if (even)
			perm[p[0]][p[1]][p[2]][p[3]][p[4]] = c++;
	}

	if (option == 0) {
		tree();
	} else if (option == 1) {
		for (auto &s : ss) {
			r = solve(s, 0);
			printf("%s\n", r.c_str());
		}
		/* 0 moves
		 * 1 move yellow
		 * 5 moves yellow' red' yellow' red yellow'
		 * 9 moves yellow red yellow white' blue red' yellow white blue'
		 *
		 * solve(s,1)
		 *
		 * y
		 * y'r'y'ry'
		 * yryw'br'ywb'
		 */
	} else if (option == 2) {
		char c[25];		//24 symbols+null terminator
		printf("input position (24 chars) ");
		fflush(stdout);
		fgets(c, 25, stdin);
		solve(c);
	} else if (option == 3) {
		VInt v;
		for (auto &s : ss) {
			r = solve(s, 0);
			printf("%s %d", r.c_str(), period(s));
			printf("\n");
		}
	} else {
		printf("unknown option");
	}
}
