// ============================================================
// Práctica N.° 09 — Árboles Binarios de Búsqueda (BST)
// Actividades 8 a 13 — Implementación en C++17
// Curso: Algoritmos y Estructuras de Datos — SIS210
// Universidad Nacional del Altiplano — UNAP Puno, 2025
//
// Compilación:
//   g++ -std=c++17 -O2 -Wall -o bst_academico bst_cpp.cpp && ./bst_academico
// ============================================================

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>

namespace una_puno {

// ── ACTIVIDAD 8: Estructuras de datos ───────────────────────

enum class EstadoAcademico { ACTIVO, EGRESADO, RETIRADO, SUSPENDIDO };

inline std::string estadoStr(EstadoAcademico e) {
    switch(e) {
        case EstadoAcademico::ACTIVO:     return "ACTIVO";
        case EstadoAcademico::EGRESADO:   return "EGRESADO";
        case EstadoAcademico::RETIRADO:   return "RETIRADO";
        case EstadoAcademico::SUSPENDIDO: return "SUSPENDIDO";
        default:                          return "DESCONOCIDO";
    }
}

struct Estudiante {
    int         codigo;
    std::string nombre;
    std::string escuela;
    float       ppa;
    int         creditos;
    EstadoAcademico estado;
    std::string semestre_ingreso;

    Estudiante(int cod, std::string nom, std::string esc,
               float pp, int cred, EstadoAcademico est, std::string sem)
        : codigo(cod), nombre(std::move(nom)), escuela(std::move(esc)),
          ppa(pp), creditos(cred), estado(est),
          semestre_ingreso(std::move(sem))
    {
        if (cod < 10000000 || cod > 29999999)
            throw std::invalid_argument("Codigo invalido: " + std::to_string(cod));
        if (pp < 0.0f || pp > 20.0f)
            throw std::invalid_argument("PPA fuera de rango [0,20]");
    }

    void print() const {
        std::cout << std::left
                  << std::setw(12) << codigo
                  << std::setw(35) << nombre
                  << std::setw(18) << escuela
                  << "PPA:" << std::fixed << std::setprecision(1) << ppa
                  << "  " << estadoStr(estado) << '\n';
    }
};

struct NodoBST {
    Estudiante                dato;
    std::unique_ptr<NodoBST>  izquierdo;
    std::unique_ptr<NodoBST>  derecho;

    explicit NodoBST(Estudiante e)
        : dato(std::move(e)), izquierdo(nullptr), derecho(nullptr) {}
};

// ── ACTIVIDADES 9-11: Clase ArbolAcademico ──────────────────

class ArbolAcademico {
public:
    ArbolAcademico() = default;

    // ── INSERTAR (Actividad 9) ───────────────────────────────
    void insertar(Estudiante e) {
        insertar_(raiz_, std::move(e));
    }

    // ── IN-ORDER (Actividad 9) ───────────────────────────────
    std::vector<Estudiante> inOrder() const {
        std::vector<Estudiante> r;
        inOrder_(raiz_.get(), r);
        return r;
    }

    // ── PRE-ORDER (Actividad 9) ──────────────────────────────
    std::vector<Estudiante> preOrder() const {
        std::vector<Estudiante> r;
        preOrder_(raiz_.get(), r);
        return r;
    }

    // ── POST-ORDER (Actividad 9) ─────────────────────────────
    std::vector<Estudiante> postOrder() const {
        std::vector<Estudiante> r;
        postOrder_(raiz_.get(), r);
        return r;
    }

    // ── BFS (Actividad 9) ────────────────────────────────────
    std::vector<Estudiante> bfs() const {
        std::vector<Estudiante> resultado;
        if (!raiz_) return resultado;
        std::queue<const NodoBST*> cola;
        cola.push(raiz_.get());
        while (!cola.empty()) {
            const NodoBST* curr = cola.front(); cola.pop();
            resultado.push_back(curr->dato);
            if (curr->izquierdo) cola.push(curr->izquierdo.get());
            if (curr->derecho)   cola.push(curr->derecho.get());
        }
        return resultado;
    }

    int  altura()    const { return altura_(raiz_.get()); }
    bool estaVacio() const { return raiz_ == nullptr; }

    // ── BUSCAR (Actividad 10) ────────────────────────────────
    std::optional<Estudiante> buscar(int codigo) const {
        const NodoBST* n = buscar_(raiz_.get(), codigo);
        if (n) return n->dato;
        return std::nullopt;
    }

    // ── ELIMINAR (Actividad 10) ──────────────────────────────
    void eliminar(int codigo) {
        if (!buscar(codigo))
            throw std::runtime_error("Codigo no encontrado: " + std::to_string(codigo));
        raiz_ = eliminar_(std::move(raiz_), codigo);
    }

    // ── MAXIMO ───────────────────────────────────────────────
    std::optional<Estudiante> maximo() const {
        if (!raiz_) return std::nullopt;
        NodoBST* n = raiz_.get();
        while (n->derecho) n = n->derecho.get();
        return n->dato;
    }

    // ── CONSULTA POR RANGO DE PPA (Actividad 11) ─────────────
    std::vector<Estudiante> porRangoPPA(float ppa_min, float ppa_max = 20.0f) const {
        auto todos = inOrder();
        std::vector<Estudiante> resultado;
        std::copy_if(todos.begin(), todos.end(), std::back_inserter(resultado),
                     [ppa_min, ppa_max](const Estudiante& e){
                         return e.ppa >= ppa_min && e.ppa <= ppa_max;
                     });
        return resultado;
    }

    // ── ESTADÍSTICAS (Actividad 11) ──────────────────────────
    void estadisticas() const {
        auto todos = inOrder();
        if (todos.empty()) { std::cout << "Arbol vacio\n"; return; }
        float suma = 0.0f, mn = 20.0f, mx = 0.0f;
        int activos = 0;
        for (const auto& e : todos) {
            suma += e.ppa;
            if (e.ppa < mn) mn = e.ppa;
            if (e.ppa > mx) mx = e.ppa;
            if (e.estado == EstadoAcademico::ACTIVO) activos++;
        }
        std::cout << std::fixed << std::setprecision(2)
                  << "  Total nodos  : " << todos.size() << '\n'
                  << "  Altura       : " << altura() << '\n'
                  << "  PPA promedio : " << suma / todos.size() << '\n'
                  << "  PPA minimo   : " << mn << '\n'
                  << "  PPA maximo   : " << mx << '\n'
                  << "  Activos      : " << activos << '\n';
    }

    // ── IMPRESIÓN ASCII (Actividad 11) ───────────────────────
    void imprimirArbol() const {
        std::cout << "\n-- Estructura del BST --\n";
        imprimir_(raiz_.get(), "", false);
    }

private:
    std::unique_ptr<NodoBST> raiz_;

    void insertar_(std::unique_ptr<NodoBST>& nodo, Estudiante e) {
        if (!nodo) {
            nodo = std::make_unique<NodoBST>(std::move(e));
            return;
        }
        if (e.codigo < nodo->dato.codigo)
            insertar_(nodo->izquierdo, std::move(e));
        else if (e.codigo > nodo->dato.codigo)
            insertar_(nodo->derecho, std::move(e));
        else
            throw std::runtime_error("Codigo duplicado: " + std::to_string(e.codigo));
    }

    void inOrder_(const NodoBST* n, std::vector<Estudiante>& r) const {
        if (!n) return;
        inOrder_(n->izquierdo.get(), r);
        r.push_back(n->dato);
        inOrder_(n->derecho.get(), r);
    }

    void preOrder_(const NodoBST* n, std::vector<Estudiante>& r) const {
        if (!n) return;
        r.push_back(n->dato);
        preOrder_(n->izquierdo.get(), r);
        preOrder_(n->derecho.get(), r);
    }

    void postOrder_(const NodoBST* n, std::vector<Estudiante>& r) const {
        if (!n) return;
        postOrder_(n->izquierdo.get(), r);
        postOrder_(n->derecho.get(), r);
        r.push_back(n->dato);
    }

    int altura_(const NodoBST* n) const {
        if (!n) return -1;
        return 1 + std::max(altura_(n->izquierdo.get()),
                            altura_(n->derecho.get()));
    }

    const NodoBST* buscar_(const NodoBST* n, int cod) const {
        if (!n || n->dato.codigo == cod) return n;
        if (cod < n->dato.codigo) return buscar_(n->izquierdo.get(), cod);
        return buscar_(n->derecho.get(), cod);
    }

    std::unique_ptr<NodoBST> eliminar_(std::unique_ptr<NodoBST> nodo, int cod) {
        if (!nodo) return nullptr;
        if (cod < nodo->dato.codigo)
            nodo->izquierdo = eliminar_(std::move(nodo->izquierdo), cod);
        else if (cod > nodo->dato.codigo)
            nodo->derecho = eliminar_(std::move(nodo->derecho), cod);
        else {
            if (!nodo->izquierdo) return std::move(nodo->derecho);
            if (!nodo->derecho)   return std::move(nodo->izquierdo);
            NodoBST* suc   = minimo_(nodo->derecho.get());
            nodo->dato     = suc->dato;
            nodo->derecho  = eliminar_(std::move(nodo->derecho), suc->dato.codigo);
        }
        return nodo;
    }

    NodoBST* minimo_(NodoBST* n) const {
        while (n->izquierdo) n = n->izquierdo.get();
        return n;
    }

    void imprimir_(const NodoBST* n, std::string prefix, bool isLeft) const {
        if (!n) return;
        std::string conector = isLeft ? "+-- " : "`-- ";
        std::cout << prefix << conector
                  << n->dato.codigo << " [PPA:"
                  << std::fixed << std::setprecision(1)
                  << n->dato.ppa << "]\n";
        std::string ext = isLeft ? "|   " : "    ";
        imprimir_(n->izquierdo.get(), prefix + ext, true);
        imprimir_(n->derecho.get(),   prefix + ext, false);
    }
};

// ── ACTIVIDAD 12: main — datos de prueba ────────────────────

void actividad12() {
    std::cout << "=== ACTIVIDADES 8-12: BST Sistema Academico UNA-PUNO (C++17) ===\n\n";
    ArbolAcademico arbol;

    arbol.insertar({20210500,"Mamani Quispe, Juan",   "Ing. Sistemas", 15.8f,120,EstadoAcademico::ACTIVO,    "2021-I"});
    arbol.insertar({20210300,"Huanca Apaza, Maria",   "Ing. Civil",    14.2f,110,EstadoAcademico::ACTIVO,    "2021-I"});
    arbol.insertar({20210700,"Condori Flores, Pedro", "Medicina",      17.1f,130,EstadoAcademico::ACTIVO,    "2021-I"});
    arbol.insertar({20210100,"Ticona Lupaca, Rosa",   "Contabilidad",  12.0f, 90,EstadoAcademico::SUSPENDIDO,"2021-I"});
    arbol.insertar({20210400,"Larico Ccama, Carlos",  "Ing. Sistemas", 16.5f,115,EstadoAcademico::ACTIVO,    "2021-I"});
    arbol.insertar({20210600,"Cutipa Vargas, Elena",  "Agronomia",     13.7f,100,EstadoAcademico::ACTIVO,    "2021-I"});
    arbol.insertar({20210900,"Pari Choque, Luis",     "Ing. Sistemas", 18.3f,140,EstadoAcademico::EGRESADO,  "2021-I"});

    arbol.imprimirArbol();

    std::cout << "\n-- In-Order (ordenado por codigo) --\n";
    std::cout << std::left << std::setw(12) << "CODIGO"
              << std::setw(35) << "NOMBRE"
              << std::setw(18) << "ESCUELA" << "PPA\n";
    std::cout << std::string(70, '-') << '\n';
    for (const auto& e : arbol.inOrder()) e.print();

    std::cout << "\n-- Pre-Order --\n";
    for (const auto& e : arbol.preOrder()) std::cout << e.codigo << " ";
    std::cout << "\n\n-- Post-Order --\n";
    for (const auto& e : arbol.postOrder()) std::cout << e.codigo << " ";
    std::cout << "\n\n-- BFS (nivel por nivel) --\n";
    for (const auto& e : arbol.bfs()) std::cout << e.codigo << " ";
    std::cout << '\n';

    std::cout << "\n-- Busqueda --\n";
    auto res = arbol.buscar(20210700);
    if (res) { std::cout << "Encontrado: "; res->print(); }
    auto noExiste = arbol.buscar(99999999);
    std::cout << "Buscar 99999999: " << (noExiste ? "encontrado" : "no encontrado") << '\n';

    std::cout << "\n-- Estudiantes con PPA >= 15.0 --\n";
    for (const auto& e : arbol.porRangoPPA(15.0f)) e.print();

    std::cout << "\n-- Estadisticas --\n";
    arbol.estadisticas();

    std::cout << "\n-- Eliminando codigo 20210300 --\n";
    arbol.eliminar(20210300);
    std::cout << "Nodos restantes: " << arbol.inOrder().size() << '\n';
    std::cout << "In-Order: ";
    for (const auto& e : arbol.inOrder()) std::cout << e.codigo << " ";
    std::cout << '\n';

    std::cout << "\n-- Maximo --\n";
    auto mx = arbol.maximo();
    if (mx) std::cout << "Codigo maximo: " << mx->codigo << '\n';
}

// ── ACTIVIDAD 13: Benchmark C++ ─────────────────────────────

std::vector<Estudiante> generarDatos(int n) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int>   distCod(20000000, 29999999);
    std::uniform_real_distribution<float> distPPA(8.0f, 20.0f);

    std::vector<int> codigos;
    codigos.reserve(n * 2);
    while ((int)codigos.size() < n * 2)
        codigos.push_back(distCod(rng));
    std::sort(codigos.begin(), codigos.end());
    codigos.erase(std::unique(codigos.begin(), codigos.end()), codigos.end());
    codigos.resize(n);
    std::shuffle(codigos.begin(), codigos.end(), rng);

    std::vector<Estudiante> datos;
    datos.reserve(n);
    for (int i = 0; i < n; i++)
        datos.emplace_back(codigos[i], "Estudiante_" + std::to_string(i),
                           "Ingenieria", 10.0f + distPPA(rng) * 0.5f,
                           100, EstadoAcademico::ACTIVO, "2024-I");
    return datos;
}

void actividad13() {
    using Clock = std::chrono::high_resolution_clock;
    std::cout << "\n=== ACTIVIDAD 13: Benchmark C++17 ===\n";
    std::cout << std::setw(8) << "N"
              << " | " << std::setw(12) << "Ins (ms)"
              << " | " << std::setw(12) << "Bus (ms)"
              << " | Altura\n";
    std::cout << std::string(50, '-') << '\n';

    for (int n : {100, 1000, 10000, 100000}) {
        auto datos     = generarDatos(n);
        int buscarCod  = datos[n / 2].codigo;

        ArbolAcademico arbol;
        auto t0 = Clock::now();
        for (auto& e : datos) arbol.insertar(e);
        auto t1 = Clock::now();
        double ms_ins = std::chrono::duration<double, std::milli>(t1 - t0).count();

        auto t2 = Clock::now();
        arbol.buscar(buscarCod);
        auto t3 = Clock::now();
        double ms_bus = std::chrono::duration<double, std::milli>(t3 - t2).count();

        std::cout << std::setw(8) << n
                  << " | " << std::fixed << std::setprecision(4)
                  << std::setw(12) << ms_ins
                  << " | " << std::setw(12) << ms_bus
                  << " | " << arbol.altura() << '\n';
    }
}

} // namespace una_puno

int main() {
    una_puno::actividad12();
    una_puno::actividad13();
    return 0;
}
