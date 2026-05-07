#include "StaticTree.h"
#include "PointerBST.h"

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstdint>

using namespace std;
using namespace chrono;

vector<int> generateKeys(int N) {
    vector<int> keys;
    keys.reserve(N);

    for (int i = 0; i < N; i++) {
        keys.push_back(i);
    }

    return keys;
}

vector<int> generateQueries(int Q, int maxValue, int seed) {
    vector<int> queries;
    queries.reserve(Q);

    mt19937 rng(seed);
    uniform_int_distribution<int> dist(0, maxValue);

    for (int i = 0; i < Q; i++) {
        queries.push_back(dist(rng));
    }

    return queries;
}

double elapsedMs(steady_clock::time_point start, steady_clock::time_point end) {
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

double testStaticTree(const StaticTree& tree, const vector<int>& queries, int& foundCount) {
    foundCount = 0;

    auto start = steady_clock::now();

    for (int x : queries) {
        if (tree.find(x)) {
            foundCount++;
        }
    }

    auto end = steady_clock::now();

    return elapsedMs(start, end);
}

double testPointerBST(const PointerBST& tree, const vector<int>& queries, int& foundCount) {
    foundCount = 0;

    auto start = steady_clock::now();

    for (int x : queries) {
        if (tree.find(x)) {
            foundCount++;
        }
    }

    auto end = steady_clock::now();

    return elapsedMs(start, end);
}

int main() {
    static_assert(sizeof(int) == 4, "El tipo int debe ser de 32 bits.");

    const int N = 1000000;   // Numero de elementos
    const int Q = 1000000;   // Numero de consultas
    const int T = 5;         // Numero de experimentos

    // Este parametro no se usa para construir el arbol.
    // El static tree es cache-oblivious, por eso no depende de B.
    const int B = 64;

    cout << fixed << setprecision(3);

    cout << "Parametros del experimento\n";
    cout << "N = " << N << " elementos\n";
    cout << "Q = " << Q << " consultas\n";
    cout << "T = " << T << " experimentos\n";
    cout << "B = " << B << " block size target\n";
    cout << "Tipo de dato: int (" << sizeof(int) * 8 << " bits)\n";
    cout << "----------------------------------\n";

    vector<int> keys = generateKeys(N);

    double totalStaticBuild = 0.0;
    double totalBSTBuild = 0.0;
    double totalStaticSearch = 0.0;
    double totalBSTSearch = 0.0;

    for (int t = 1; t <= T; t++) {
        cout << "Experimento " << t << " de " << T << "\n";

        vector<int> queries = generateQueries(Q, 2 * N, 12345 + t);

        auto startBuildStatic = steady_clock::now();
        StaticTree staticTree(keys);
        auto endBuildStatic = steady_clock::now();

        double staticBuildMs = elapsedMs(startBuildStatic, endBuildStatic);

        auto startBuildBST = steady_clock::now();
        PointerBST pointerBST(keys);
        auto endBuildBST = steady_clock::now();

        double bstBuildMs = elapsedMs(startBuildBST, endBuildBST);

        int foundStatic = 0;
        int foundBST = 0;

        double staticSearchMs = testStaticTree(staticTree, queries, foundStatic);
        double bstSearchMs = testPointerBST(pointerBST, queries, foundBST);

        totalStaticBuild += staticBuildMs;
        totalBSTBuild += bstBuildMs;
        totalStaticSearch += staticSearchMs;
        totalBSTSearch += bstSearchMs;

        cout << "Build StaticTree: " << staticBuildMs << " ms\n";
        cout << "Build PointerBST: " << bstBuildMs << " ms\n";
        cout << "Search StaticTree: " << staticSearchMs << " ms\n";
        cout << "Search PointerBST: " << bstSearchMs << " ms\n";
        cout << "Encontrados StaticTree: " << foundStatic << "\n";
        cout << "Encontrados PointerBST: " << foundBST << "\n";

        if (foundStatic != foundBST) {
            cout << "advertencia: los resultados no coinciden.\n";
        }

        cout << "----------------------------------\n";
    }

    cout << "Promedios finales\n";
    cout << "Promedio build StaticTree: " << totalStaticBuild / T << " ms\n";
    cout << "Promedio build PointerBST: " << totalBSTBuild / T << " ms\n";
    cout << "Promedio search StaticTree: " << totalStaticSearch / T << " ms\n";
    cout << "Promedio search PointerBST: " << totalBSTSearch / T << " ms\n";

    return 0;
}