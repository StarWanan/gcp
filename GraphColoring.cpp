#include "GraphColoring.h"

#include <random>
#include <iostream>
#include <set>
#include <map>
#include <cstring>
#include <time.h>
using namespace std;



#define P1 0
#define P2 1
#define E1 2
#define E2 3
#define BEST 4

#define SHOW
#define SHOW_TIME
// #define SHOW_left
// #define SHOW_seed
// #define SHOW_file
// #define SHOW_tabu
// #define BUG

namespace szx
{

	class Solver
	{
		// random number generator.
		mt19937 pseudoRandNumGen;
		void initRand(int seed) { pseudoRandNumGen = mt19937(seed); }
		int fastRand(int lb, int ub) { return (pseudoRandNumGen() % (ub - lb)) + lb; }
		int fastRand(int ub) { return pseudoRandNumGen() % ub; }
		int rand(int lb, int ub) { return uniform_int_distribution<int>(lb, ub - 1)(pseudoRandNumGen); }
		int rand(int ub) { return uniform_int_distribution<int>(0, ub - 1)(pseudoRandNumGen); }

		const int inf = 0x3f3f3f3f;
		int MAX_TABU_ITER = 10000;
		int ITER_CYCLE = 10;

		int N, M, K;
		
		int *h, *e, *ne;
		int idx = 1;

		int *p[5], *c[2];
		int f[5];

		// tabucol
		int **adj_table, **tabu_table;
		int best_f;
		
		// LS
		int delta;
		int iter;
		int equ_size;
		int equ_delt[2000][2]; 
		int sel_vertex, sel_color;
		 
		clock_t start, end;

	public:
		void add(int a, int b)
		{
			e[idx] = b, ne[idx] = h[a], h[a] = idx++;
		}
		void init_graph(GraphColoring &in)
		{
			N = in.nodeNum;
			M = in.edgeNum << 1;
			K = in.colorNum;

			h = new int [N];
			e = new int [M];
			ne = new int [M];

			for (auto edge : in.edges)
			{
				int a = edge[0], b = edge[1];
				add(a, b);
				add(b, a);
			}

			adj_table = new int *[N];
			tabu_table = new int *[N];
			for (int i = 0; i < N; i++)
			{
				adj_table[i] = new int[K];
				tabu_table[i] = new int[K];
			}

			for (int i = 0; i < 5; i++)
			{
				p[i] = new int[N];
			}
			c[0] = new int[N];
			c[1] = new int[N];
		}

		int count_f(int *p)
		{
			int f = 0;
			for (int i = 0; i < N; i ++)
			{
				for (int j = h[i]; j; j = ne[j]) {
					int k = e[j];
					if (p[k] == p[i]) f ++;
				}
			}
			f >>= 1;
			return f;
		}
		
		void init_p(int m)
		{
			for (int i = 0; i < N; i ++)
			{
				p[m][i] = rand(K);
			}
			f[m] = count_f(p[m]);
			tabucol(p[m], f[m]);
			if (f[m] < f[BEST]) {
				copy_p(BEST, m);
			}
		}

		void Init(GraphColoring &in)
		{
			init_graph(in);
			for (int i = 0; i < 5; i ++) {
				init_p(i);
			}
			// init_p(P1);
			// init_p(P2);
			// init_p(BEST);
		}

		void SaveOutput(NodeColors &output)
		{
			for (int i = 0; i < N; i++)
			{
				output[i] = p[BEST][i];
			}
		}

		void cal_adj(int *p)
		{
			for (int i = 0; i < N; i++)
				memset(adj_table[i], 0, K * sizeof(int));
			for (int i = 0; i < N; i ++) {
				for (int j = h[i]; j; j = ne[j]) {
					int k = e[j];
					adj_table[i][p[k]] ++;
				}
			}
		}
		
		void clear_tabu()
		{
			for (int i = 0; i < N ; i++) {
				memset(tabu_table[i], 0, K * sizeof(int));
			}
			
		}

		void findmove(int *p, int &f)
		{
			delta = inf;
			int tmp_delta;
			for (int i = 0; i < N; i ++) {
				if (adj_table[i][p[i]]) {
					for (int j = 0; j < K; j ++) {
						if (j != p[i]) {
							tmp_delta = adj_table[i][j] - adj_table[i][p[i]];
							if (tmp_delta <= delta && (iter >= tabu_table[i][j] || f + tmp_delta < best_f)) {
								if (tmp_delta < delta) // 当前解小于本次迭代最优解,则重新开始保存解
								{ 
									delta = tmp_delta;
									equ_size = 0;
								}
								equ_delt[equ_size][0] = i;
								equ_delt[equ_size][1] = j;
								equ_size++; // end of another way
							}
						}
					}
				}
			}
			int tmp = rand(equ_size); // 有多个解时，随机选择
			sel_vertex = equ_delt[tmp][0];
			sel_color = equ_delt[tmp][1];
		}	

		void makemove(int *p, int &f)
		{
			f = f + delta;
			int ori_color = p[sel_vertex];	// 原始颜色
			p[sel_vertex] = sel_color;		// 更新颜色
			tabu_table[sel_vertex][ori_color] = iter + 0.6 * f + rand(10);

			// 更新邻接颜色表, 只影响相邻节点
			for (int i = h[sel_vertex]; i; i = ne[i]) {
				int j = e[i];
				adj_table[j][ori_color] --;
				adj_table[j][sel_color] ++;
			}
		}

		void tabucol(int *p, int &f)
		{
			cal_adj(p);
			clear_tabu();
			best_f = f;
			for (iter = 0; iter < MAX_TABU_ITER && best_f; iter ++)
			{
				findmove(p, f);
				makemove(p, f);
				// if (f < best_f) best_f = f;
				if (f < best_f)
				{
#ifdef SHOW_tabu
					fprintf(stderr, "iter: %d \tf: %d -> %d\n", iter, best_f, f);
#endif
					best_f = f;
				}
			}
		}

		void GPX(int *p0, int *p1, int *c)
		{
			set<int>::iterator it;
			set<int> p[2][K];
			for (int i = 0; i < N; i++)
			{
				p[0][p0[i]].insert(i);
				p[1][p1[i]].insert(i);
			}

			int id = 0;
			for (int u = 0; u < K; u++)
			{
				int max_color_cnt = 0;
				vector<int> max_color_id;
				for (int i = 0; i < K; i++)
				{
					if (max_color_cnt <= p[id][i].size())
					{
						if (max_color_cnt < p[id][i].size())
						{
							max_color_cnt = p[id][i].size();
							max_color_id.clear();
						}
						max_color_id.push_back(i);
					}
				}

				int mc = max_color_id[rand(max_color_id.size())];
				for (auto v : p[id][mc])
				{
					c[v] = u;
					for (int j = 0; j < K; j++)
					{
						if (p[id ^ 1][j].count(v))
							p[id ^ 1][j].erase(v);
					}
				}
				p[id][mc].clear();
				id ^= 1;
			}

			for (int i = 0; i < K; i++)
			{
				for (auto v : p[0][i])
				{
					c[v] = rand(K);
				}
			}
		}

		void copy_p(int tar, int src)
		{
			memcpy(p[tar], p[src], N * sizeof(int));
			f[tar] = f[src];
		}

		void solve(NodeColors &output, GraphColoring &input, std::function<bool()> isTimeout, int seed)
		{
			initRand(seed);
			Init(input);

			if (N >= 500 && K >= 80) MAX_TABU_ITER = 40000;
			else MAX_TABU_ITER = 5000;

			start = clock();
			int generation;
			for (generation = 1; f[BEST] && !isTimeout(); generation ++)
			{
				GPX(p[P1], p[P2], c[0]);
				GPX(p[P2], p[P1], c[1]);

				int tmpf0, tmpf1;
				tmpf0 = count_f(c[0]);
				tmpf1 = count_f(c[1]);
				tabucol(c[0], tmpf0);
				tabucol(c[1], tmpf1);
				memcpy(p[0], c[0], N * sizeof(int));
				memcpy(p[1], c[1], N * sizeof(int));
				f[0] = tmpf0;
				f[1] = tmpf1;

				#ifdef BUG
				fprintf(stderr, "f[0]: %d, f[1]: %d\n", f[0], f[1]);
				#endif

				int best_p = 0, min_f = 10000;
				if (f[0] < f[1]){
					min_f = f[0];
					best_p = 0;
				}
				else{
					min_f = f[1];
					best_p = 1;
				}
				if (f[E1] > min_f) copy_p(E1, best_p);

				if (f[BEST] > f[E1]){
					copy_p(BEST, E1);
#ifdef SHOW
					fprintf(stderr, "generation=%d  best_f: %d\n", generation, f[BEST]);
#endif
				}

				if (generation % ITER_CYCLE == 0)
				{
					copy_p(P1, E2);
					copy_p(E2, E1);
					init_p(E1);
				}
			}
			end = clock();
			if (f[BEST] == 0)
			{
				#ifdef BUG
				puts("here");
				#endif
				SaveOutput(output);
			}

#ifdef SHOW
			printf("END. generation = %d\t Time = %lus\n", generation, (end - start) / CLOCKS_PER_SEC);
#endif
		}
	};

	

	// solver.
	void solveGraphColoring(NodeColors &output, GraphColoring &input, std::function<bool()> isTimeout, int seed)
	{
		Solver().solve(output, input, isTimeout, seed);
	}
}

/*
clang++ -fcolor-diagnostics -fansi-escape-codes -O3 -g -std=c++11 Main.cpp GraphColoring.cpp -o gcp

clang++ Main.cpp GraphColoring.cpp -o GCP -O3 -static
g++ Main.cpp GraphColoring.cpp -o GCP -O3 -static
x86_64-w64-mingw32-g++ Main.cpp GraphColoring.cpp -o GCP -O3 -static

/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes -O2 -g -std=c++11 /Users/star/workspace/Smart/GraphColoringProblem/Main.cpp /Users/star/workspace/Smart/GraphColoringProblem/GraphColoring.cpp -o /Users/star/workspace/Smart/GraphColoringProblem/mac
*/