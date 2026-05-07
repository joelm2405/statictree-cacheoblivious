# Cache-Oblivious Static Search Tree

### Integrantes:
- Joel David Miguel Fernandez - 202310186

- Ary Werner Aaron Rojas Durand - 202310366

## Descripción general

En este proyecto implementé un **cache-oblivious static search tree** y lo comparé contra un **BST tradicional implementado con punteros**.

La idea principal del static tree es construir un árbol de búsqueda balanceado sobre un conjunto fijo de claves, pero almacenarlo en memoria usando un orden especial llamado **van Emde Boas layout**. Este layout busca que los nodos que suelen visitarse juntos durante una búsqueda queden cerca en memoria, mejorando la localidad de caché.

El árbol es **static** porque se construye una sola vez y luego solo se realizan consultas. Es decir, no implementa inserciones ni eliminaciones.

---

## Objetivo del proyecto

El objetivo fue implementar el árbol estático cache-oblivious visto en clase y medir su eficiencia frente a un BST normal con punteros.

Para la comparación:

- ambos árboles se construyen con los mismos elementos;
- se generan consultas aleatorias;
- las mismas consultas se ejecutan en ambos árboles;
- se mide el tiempo de construcción y búsqueda;
- se repite el experimento varias veces;
- se reportan los promedios finales.

---

## Idea del static tree

Primero se construye un BST balanceado a partir de los elementos ordenados. Luego, en lugar de guardar los nodos en cualquier orden, se reorganizan en un arreglo usando **van Emde Boas layout**.

La búsqueda sigue siendo una búsqueda normal de BST:

```cpp
if (x < key) voy a la izquierda;
else voy a la derecha;
```

La diferencia está en cómo están ubicados físicamente los nodos en memoria. En el static tree, los nodos se guardan en un `vector`, usando índices para representar los hijos. Esto evita saltos de punteros dispersos en memoria y mejora la localidad.

---

## van Emde Boas layout

El **van Emde Boas layout** es una forma recursiva de ordenar los nodos del árbol en memoria.

La idea es:

1. Tomar un subárbol.
2. Cortarlo aproximadamente a la mitad de su altura.
3. Guardar primero la parte superior.
4. Luego guardar recursivamente los subárboles inferiores.
5. Repetir el proceso hasta llegar a nodos individuales.

Por ejemplo, para los elementos del 1 al 15, el BST lógico balanceado es:

```text
        8
      /   \
     4     12
    / \    /  \
   2   6  10  14
  / \ / \ / \ / \
 1  3 5 7 9 11 13 15
```

En memoria, usando vEB layout, puede quedar como:

```text
8 4 12 2 1 3 6 5 7 10 9 11 14 13 15
```

Este orden no es ascendente ni es exactamente por niveles. Es un orden pensado para mejorar la localidad de memoria durante las búsquedas.

---

# Archivos del proyecto

La implementación está dividida en tres archivos principales:

```text
StaticTree.h      -> implementación del static tree cache-oblivious
PointerBST.h      -> implementación del BST normal con punteros
main.cpp          -> ejecución de experimentos y medición de tiempos
```

---

# StaticTree.h

Este archivo contiene la implementación del árbol estático cache-oblivious.

## `struct StaticNode`

```cpp
struct StaticNode {
    int key;
    int left;
    int right;
};
```

Cada nodo guarda:

- `key`: el valor del nodo;
- `left`: índice del hijo izquierdo dentro del vector;
- `right`: índice del hijo derecho dentro del vector.

A diferencia del BST con punteros, aquí no se usan `Node*`. Los hijos se representan con índices. Si un nodo no tiene hijo izquierdo o derecho, se usa `-1`.

Esto permite guardar todo el árbol en un arreglo continuo de memoria.

---

## Variables principales de la clase `StaticTree`

```cpp
std::vector<StaticNode> normal;
std::vector<StaticNode> tree;
std::vector<int> h;
int normalRoot;
int root;
```

### `normal`

Es un BST balanceado auxiliar. Lo uso solo durante la construcción para representar la estructura lógica del árbol.

### `tree`

Es el árbol final, ya reorganizado en memoria con van Emde Boas layout. Las búsquedas se realizan sobre este vector.

### `h`

Guarda la altura de cada nodo del árbol auxiliar `normal`. Se necesita porque el layout vEB divide el árbol según su altura.

### `normalRoot`

Es el índice de la raíz en el árbol auxiliar `normal`.

### `root`

Es el índice de la raíz en el árbol final `tree`.

---

## `buildBST`

```cpp
int buildBST(const std::vector<int>& a, int l, int r)
```

Esta función construye un BST balanceado a partir de un arreglo ordenado.

La idea es tomar siempre el elemento del medio como raíz:

```cpp
int mid = l + (r - l) / 2;
```

Luego:

- los elementos de la izquierda forman el subárbol izquierdo;
- los elementos de la derecha forman el subárbol derecho.

Por ejemplo, si los elementos son del 1 al 15, el primer elemento elegido como raíz es 8, porque está al medio del arreglo ordenado.

Esta función devuelve el índice del nodo raíz que acaba de construir.

---

## `calcHeight`

```cpp
int calcHeight(int u)
```

Calcula la altura de cada nodo del árbol auxiliar `normal`.

La altura se calcula como:

```text
1 + max(altura del hijo izquierdo, altura del hijo derecho)
```

Si el nodo no existe, retorna 0.

Esta información se guarda en el vector `h` y luego se usa para decidir dónde cortar el árbol al construir el layout vEB.

---

## `getRootsAtDepth`

```cpp
void getRootsAtDepth(int u, int depth, std::vector<int>& roots) const
```

Esta función obtiene todos los nodos que están a una profundidad específica desde un nodo dado.

Por ejemplo, si estoy parado en la raíz 8:

```text
        8
      /   \
     4     12
    / \    / \
   2   6  10 14
```

- a profundidad 1 están `4` y `12`;
- a profundidad 2 están `2`, `6`, `10` y `14`.

Esta función es importante porque, cuando se corta el árbol en la mitad de su altura, necesito encontrar las raíces de los subárboles inferiores.

---

## `makeVEBOrder`

```cpp
void makeVEBOrder(int u, int heightLimit, std::vector<int>& order) const
```

Esta es la función principal del static tree.

Su objetivo es calcular el orden van Emde Boas de los nodos.

La función trabaja así:

1. Si el nodo no existe, termina.
2. Si la altura considerada es 1, agrega ese nodo al orden.
3. Divide la altura del subárbol en dos partes:

```cpp
int topHeight = (heightLimit + 1) / 2;
int bottomHeight = heightLimit - topHeight;
```

4. Primero procesa la parte superior.
5. Luego encuentra las raíces de los subárboles inferiores.
6. Finalmente procesa cada subárbol inferior de forma recursiva.

En otras palabras, esta función implementa la idea:

```text
guardar arriba primero, luego guardar recursivamente los subárboles de abajo
```

Ese es el corazón del layout cache-oblivious.

---

## `buildVEBLayout`

```cpp
void buildVEBLayout()
```

Esta función construye el árbol final en memoria.

Primero llama a:

```cpp
makeVEBOrder(normalRoot, h[normalRoot], order);
```

Con eso obtiene el orden en que los nodos deben aparecer en el vector final `tree`.

Luego crea un vector `pos`, que sirve para traducir posiciones antiguas a posiciones nuevas:

```text
posición antigua en normal -> posición nueva en tree
```

Esto es necesario porque, al reordenar los nodos, los hijos también cambian de índice.

Después copia cada nodo al nuevo vector `tree` y actualiza sus hijos `left` y `right` usando las nuevas posiciones.

Al final, `tree` contiene el mismo BST lógico, pero guardado físicamente en orden van Emde Boas.

---

## Constructor `StaticTree`

```cpp
StaticTree(std::vector<int> a)
```

El constructor recibe los elementos del árbol.

Hace lo siguiente:

1. Ordena los elementos.
2. Elimina duplicados.
3. Construye un BST balanceado auxiliar.
4. Calcula las alturas.
5. Construye el layout van Emde Boas.
6. Libera las estructuras auxiliares `normal` y `h`.

La liberación de `normal` y `h` es importante porque el árbol auxiliar solo se necesita durante la construcción. Después de construir `tree`, las búsquedas ya no usan `normal`.

---

## `find`

```cpp
bool find(int x) const
```

Busca si un valor `x` está en el árbol.

La búsqueda es igual a la de un BST normal:

- si `x` es igual a la clave actual, retorna `true`;
- si `x` es menor, baja al hijo izquierdo;
- si `x` es mayor, baja al hijo derecho;
- si llega a `-1`, significa que no encontró el valor.

La diferencia con un BST tradicional está en la memoria: aquí los nodos están en un vector con layout vEB.

---

## `predecessor`

```cpp
bool predecessor(int x, int& ans) const
```

Busca el mayor valor menor que `x`.

Si encuentra un predecesor, lo guarda en `ans` y retorna `true`. Si no existe, retorna `false`.

Durante la búsqueda:

- si el nodo actual es menor que `x`, puede ser respuesta, entonces lo guardo y trato de buscar uno más grande yendo a la derecha;
- si el nodo actual es mayor o igual que `x`, voy a la izquierda.

---

## `successor`

```cpp
bool successor(int x, int& ans) const
```

Busca el menor valor mayor que `x`.

Si encuentra un sucesor, lo guarda en `ans` y retorna `true`. Si no existe, retorna `false`.

Durante la búsqueda:

- si el nodo actual es mayor que `x`, puede ser respuesta, entonces lo guardo y trato de buscar uno más pequeño yendo a la izquierda;
- si el nodo actual es menor o igual que `x`, voy a la derecha.

---

## `printMemoryLayout`

```cpp
void printMemoryLayout(std::ostream& out) const
```

Imprime el orden físico de los nodos dentro del vector `tree`.

Sirve para verificar cómo quedó el layout en memoria. No imprime los elementos ordenados ascendentemente, sino el orden vEB.

---

## `countBlocksInSearch`

```cpp
int countBlocksInSearch(int x, int B) const
```

Esta función simula cuántos bloques de memoria se tocarían durante una búsqueda, asumiendo que cada bloque contiene `B` nodos.

Se usa solo como apoyo para entender el comportamiento por bloques. No se usa en la medición principal de tiempos, porque utiliza un `set` y eso agregaría costo extra a la búsqueda.

---

## `size`

```cpp
int size() const
```

Devuelve la cantidad de nodos almacenados en el árbol final `tree`.

---

# main.cpp

Este archivo ejecuta los experimentos, la comparacion respecto a el Static Tree, con el BST con punteros (implementacion en archivo PointerBST.h)

---

## `generateKeys`

```cpp
vector<int> generateKeys(int N)
```

Genera los elementos que se insertan en ambos árboles.

Para `N = 1000000`, genera:

```text
0, 1, 2, ..., 999999
```

Estos son enteros únicos de 32 bits.

---

## `generateQueries`

```cpp
vector<int> generateQueries(int Q, int maxValue, int seed)
```

Genera consultas aleatorias.

En los experimentos se usa:

```cpp
generateQueries(Q, 2 * N, 12345 + t)
```

Esto genera valores entre `0` y `2N`.

Como los elementos insertados están entre `0` y `N - 1`, aproximadamente la mitad de consultas existen en los árboles y la otra mitad no.

---

## `elapsedMs`

```cpp
double elapsedMs(steady_clock::time_point start, steady_clock::time_point end)
```

Calcula el tiempo transcurrido entre dos instantes y lo devuelve en milisegundos.

Se usa para medir construcción y búsqueda.

---

## `testStaticTree`

```cpp
double testStaticTree(const StaticTree& tree, const vector<int>& queries, int& foundCount)
```

Ejecuta todas las consultas sobre el static tree.

Por cada consulta llama a:

```cpp
tree.find(x)
```

También cuenta cuántas consultas fueron encontradas.

Devuelve el tiempo total de búsqueda en milisegundos.

---

## `testPointerBST`

```cpp
double testPointerBST(const PointerBST& tree, const vector<int>& queries, int& foundCount)
```

Hace lo mismo que `testStaticTree`, pero usando el BST con punteros.

---

## Parámetros principales

```cpp
const int N = 1000000;
const int Q = 1000000;
const int T = 5;
const int B = 64;
```

- `N`: número de elementos en los árboles.
- `Q`: número de consultas aleatorias.
- `T`: número de experimentos.
- `B`: block size target usado como referencia.

El árbol cache-oblivious no usa `B` para construirse.

---

## Flujo del experimento

El experimento sigue estos pasos:

1. Genero `N` elementos.
2. Repito el experimento `T` veces.
3. En cada repetición genero `Q` consultas aleatorias.
4. Construyo el `StaticTree`.
5. Construyo el `PointerBST`.
6. Ejecuto las mismas consultas en ambos árboles.
7. Mido tiempos de construcción y búsqueda.
8. Verifico que ambos árboles encuentren la misma cantidad de elementos.
9. Acumulo tiempos para calcular promedios.

---

# Resultados experimentales

Los experimentos se ejecutaron en modo **Release**.

## Parámetros usados

```text
N = 1000000 elementos
Q = 1000000 consultas
T = 5 experimentos
B = 64 block size target
Tipo de dato: int (32 bits)
```

---

## Resultados por experimento

| Experimento | Build StaticTree (ms) | Build PointerBST (ms) | Search StaticTree (ms) | Search PointerBST (ms) | Encontrados StaticTree | Encontrados PointerBST |
|------------:|----------------------:|----------------------:|------------------------:|------------------------:|-----------------------:|-----------------------:|
| 1 | 115.975 | 75.241 | 123.358 | 278.791 | 500043 | 500043 |
| 2 | 108.346 | 76.194 | 102.515 | 255.239 | 499132 | 499132 |
| 3 | 102.739 | 74.773 | 106.518 | 239.721 | 500010 | 500010 |
| 4 | 113.452 | 74.700 | 122.130 | 248.581 | 499270 | 499270 |
| 5 | 100.588 | 77.119 | 121.329 | 254.412 | 499529 | 499529 |

---

## Promedios finales

| Métrica | StaticTree | PointerBST |
|--------|-----------:|-----------:|
| Build promedio | 108.220 ms | 75.605 ms |
| Search promedio | 115.170 ms | 255.349 ms |

---

# Análisis de resultados

El `PointerBST` construye más rápido que el `StaticTree`:

```text
Build StaticTree promedio: 108.220 ms
Build PointerBST promedio: 75.605 ms
```

Esto es esperable, porque el `StaticTree` realiza más pasos durante la construcción:

1. construye un BST balanceado auxiliar;
2. calcula alturas;
3. calcula el orden van Emde Boas;
4. reconstruye el árbol final en un vector;
5. libera estructuras auxiliares.

En cambio, el `PointerBST` solo construye nodos enlazados con punteros.

Sin embargo, en búsquedas el `StaticTree` fue claramente más rápido:

```text
Search StaticTree promedio: 115.170 ms
Search PointerBST promedio: 255.349 ms
```

La mejora aproximada en búsqueda fue:

```text
255.349 / 115.170 ≈ 2.22
```

Es decir, en estos experimentos, el `StaticTree` fue aproximadamente **2.22 veces más rápido** que el BST con punteros en la fase de búsqueda.

También se puede interpretar como una reducción de tiempo de aproximadamente:

```text
1 - (115.170 / 255.349) ≈ 54.9%
```

Por lo tanto, el static tree redujo el tiempo de búsqueda en alrededor de **54.9%** frente al BST con punteros.

---

# Validación de correctitud

En todos los experimentos, ambos árboles encontraron exactamente la misma cantidad de elementos.

Por ejemplo, en el experimento 1:

```text
Encontrados StaticTree: 500043
Encontrados PointerBST: 500043
```

Esto indica que ambas estructuras están respondiendo correctamente a las mismas consultas.

La cantidad de encontrados está cerca de la mitad de las consultas porque las consultas se generan entre `0` y `2N`, mientras que los elementos del árbol están entre `0` y `N - 1`.

---

# Complejidad

## StaticTree

| Operación | Complejidad |
|----------|-------------|
| Construcción del BST auxiliar | O(N) después de ordenar |
| Ordenamiento inicial | O(N log N) |
| Construcción del layout vEB | O(N) |
| Búsqueda | O(log N) comparaciones |
| Transferencias de memoria esperadas | O(log_B N) |

## PointerBST

| Operación | Complejidad |
|----------|-------------|
| Construcción balanceada | O(N) después de ordenar |
| Ordenamiento inicial | O(N log N) |
| Búsqueda | O(log N) comparaciones |

La diferencia principal no está en el número de comparaciones, sino en el acceso a memoria. El `StaticTree` guarda los nodos en un vector continuo con layout vEB, mientras que el `PointerBST` usa punteros, lo que puede producir saltos a zonas más dispersas de memoria.

---

# Hardware usado

Completar con los datos de la computadora donde se ejecutaron los experimentos:

```text
Procesador: AMD RYZEN 5 5000 SERIES
Memoria RAM: 16GB RAM 3200 MHZ
Sistema operativo: Windows 64 BITS
Compilador: C++ 23
Modo de compilación: Release
```

---

# Conclusión

En este proyecto implementé un cache-oblivious static search tree usando van Emde Boas layout y lo comparé contra un BST balanceado con punteros.

Los resultados muestran que el `StaticTree` tiene mayor costo de construcción, pero obtiene una mejora clara en el tiempo de búsqueda. Esto tiene sentido porque el layout vEB intenta mejorar la localidad de memoria, haciendo que durante una búsqueda se acceda a nodos más cercanos físicamente en memoria.

Como el árbol es estático, este costo adicional de construcción puede valer la pena cuando se realizan muchas consultas sobre el mismo conjunto de datos.
