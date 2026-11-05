// ============================================================================
// WattControl v1 - Gestor de Energía para Lavandería (El Niño 2026)
// Asignatura: Programación Orientada a Objetos
// Negocio: Lavandería (Dígito final 4)
// ============================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

using namespace std;

#define HORAS_DIA 24
#define MAX_CARGAS 3
#define SEMILLA 2026

// --- CONSTANTES DEL NEGOCIO: LAVANDERÍA ---
const float POTENCIA_CRITICA = 1.2;     // Bomba de agua y Recepción (kW)
const float POTENCIA_APLAZ1 = 7.0;      // Lavadora Industrial (kW)
const float POTENCIA_APLAZ2 = 5.0;      // Secadora Industrial (kW)
const float POTENCIA_CONTRATADA = 11.0; // Potencia máxima contratada (kW)

const float PRECIO_CARO = 850.0;        // COP/kWh (Umbral precio caro)
const float PRECIO_ESCASEZ = 1500.0;    // COP/kWh (Límite racionamiento)
const float EMBALSES_ALERTA = 40.0;     // % Alerta de embalses

const int PICO_INICIO = 18;             // Hora pico inicio (6 pm)
const int PICO_FIN = 20;                // Hora pico fin (8 pm)

const int MIN_HORAS_APLAZ1 = 6;         // Mínimo horas al día Lavadora
const int MIN_HORAS_APLAZ2 = 5;         // Mínimo horas al día Secadora

const int FALLA_CADA = 20;              // 1 de cada 20 lecturas falla

// ============================================================================
// 1. CLASE BASE ABSTRACTA: Carga
// ============================================================================
class Carga {
protected:
    string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;

public:
    Carga(string n, float p) {
        nombre = n;
        potenciaKW = p;
        encendida = false;
        horasUso = 0;
    }

    virtual ~Carga() {}

    // Polimorfismo: método virtual puro
    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    void encender() {
        encendida = true;
        horasUso++;
    }

    void apagar() {
        encendida = false;
    }

    float costoHora(float precio) {
        if (encendida) {
            return potenciaKW * precio;
        }
        return 0.0f;
    }

    void reiniciarHoras() { horasUso = 0; }

    string getNombre() { return nombre; }
    float getPotencia() { return potenciaKW; }
    bool estaEncendida() { return encendida; }
    int getHorasUso() { return horasUso; }
};

// ============================================================================
// 2. HERENCIA: CargaCritica
// ============================================================================
class CargaCritica : public Carga {
public:
    CargaCritica(string n, float p) : Carga(n, p) {}

    // Regla 1: Nunca se apaga
    bool debeEncender(int hora, float precio, float embalses) override {
        return true;
    }
};

// ============================================================================
// 3. HERENCIA: CargaAplazable
// ============================================================================
class CargaAplazable : public Carga {
private:
    int horasMinimas;

public:
    CargaAplazable(string n, float p, int hMin) : Carga(n, p) {
        horasMinimas = hMin;
    }

    // Reglas 2 a 5: Condición compuesta con operadores lógicos
    bool debeEncender(int hora, float precio, float embalses) override {
        bool horaPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool condicionAplazamiento = (embalses < EMBALSES_ALERTA) || horaPico || (precio > PRECIO_CARO);
        
        return !condicionAplazamiento;
    }

    int getHorasMinimas() { return horasMinimas; }
};

// ============================================================================
// 4. COMPOSICIÓN: Medidor
// ============================================================================
class Medidor {
private:
    float totalKWh;
    float costoTotal;

public:
    Medidor() : totalKWh(0.0f), costoTotal(0.0f) {}

    void registrarConsumo(float kw, float precioHora) {
        totalKWh += kw;
        costoTotal += (kw * precioHora);
    }

    void reiniciar() {
        totalKWh = 0.0f;
        costoTotal = 0.0f;
    }

    float getTotalKWh() const { return totalKWh; }
    float getCostoTotal() const { return costoTotal; }
};

// ============================================================================
// 5. ASOCIACIÓN: Mercado
// ============================================================================
class Mercado {
public:
    float precioHora(int hora) const {
        int minimo, maximo;
        if (hora <= 6)       { minimo = 398; maximo = 640; }
        else if (hora <= 14) { minimo = 456; maximo = 970; }
        else if (hora <= 17) { minimo = 596; maximo = 1040; }
        else if (hora <= 20) { minimo = 705; maximo = 1612; }
        else                 { minimo = 565; maximo = 970; }

        return static_cast<float>(minimo + rand() % (maximo - minimo + 1));
    }

    float nivelEmbalses(int hora) const {
        float nivel = 50.0f - (hora * 0.5f);
        return (nivel < 20.0f) ? 20.0f : nivel;
    }
};

// ============================================================================
// 6. AGREGACIÓN, COMPOSICIÓN Y ASOCIACIÓN: Negocio (Lavandería)
// ============================================================================
class Negocio {
private:
    string nombre;
    Medidor medidor;             // Composición: el medidor nace y muere con el negocio
    Carga* cargas[MAX_CARGAS];   // Agregación: arreglo de punteros a Carga creadas afuera
    int numCargas;
    Mercado* mercado;            // Asociación: conoce el mercado pero no lo crea

    float costoCon;
    float costoSin;
    int horasSimuladas;
    int fallas;

public:
    Negocio(string n, Mercado* m) 
        : nombre(n), mercado(m), numCargas(0), costoCon(0.0f), costoSin(0.0f), horasSimuladas(0), fallas(0) {}

    ~Negocio() {}

    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas++] = c;
        }
    }

    void mostrar() const {
        cout << "Negocio: " << nombre << endl;
        cout << "Cargas registradas (" << numCargas << "):" << endl;
        for (int i = 0; i < numCargas; i++) {
            cout << " - " << cargas[i]->getNombre() << " (" << cargas[i]->getPotencia() << " kW)" << endl;
        }
    }

    void evaluar(int hora, float precio, float embalses) {
        cout << "\nLectura manual -> Hora: " << hora << ":00 | Precio: $" << precio 
             << " COP | Embalses: " << embalses << "%" << endl;
        for (int i = 0; i < numCargas; i++) {
            bool dec = cargas[i]->debeEncender(hora, precio, embalses);
            cout << " - " << left << setw(18) << cargas[i]->getNombre() 
                 << ": " << (dec ? "ENCENDER" : "APAGAR / ESPERAR") << endl;
        }
    }

    void simularDia() {
        costoCon = 0.0f;
        costoSin = 0.0f;
        horasSimuladas = 0;
        fallas = 0;
        medidor.reiniciar();

        for (int i = 0; i < numCargas; i++) {
            cargas[i]->reiniciarHoras();
        }

        cout << fixed << setprecision(2);
        cout << "\n=======================================================================================" << endl;
        cout << "                       SIMULACIÓN DE 24 HORAS - WATTCONTROL V1                         " << endl;
        cout << "=======================================================================================" << endl;
        cout << left << setw(6)  << "Hora" 
             << setw(12) << "Precio(COP)" 
             << setw(12) << "Embalses(%)" 
             << setw(15) << cargas[0]->getNombre() 
             << setw(15) << cargas[1]->getNombre() 
             << setw(15) << cargas[2]->getNombre() 
             << setw(12) << "Costo Hora" << endl;
        cout << "---------------------------------------------------------------------------------------" << endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            float precio = mercado->precioHora(h);
            float embalses = mercado->nivelEmbalses(h);

            costoSin += (POTENCIA_CRITICA + POTENCIA_APLAZ1 + POTENCIA_APLAZ2) * precio;

            // Uso de break: Precio de escasez y alerta de embalses
            if (precio > PRECIO_ESCASEZ && embalses < EMBALSES_ALERTA) {
                cout << setw(6) << (to_string(h) + ":00") 
                     << setw(12) << precio 
                     << " [ALERTA] PRECIO DE ESCASEZ: Simulacion interrumpida" << endl;
                break;
            }

            horasSimuladas++;

            // Uso de continue: Falla aleatoria de lectura de sensor
            if (rand() % FALLA_CADA == 0) {
                for (int i = 0; i < numCargas; i++) cargas[i]->apagar();
                fallas++;
                cout << setw(6) << (to_string(h) + ":00") 
                     << setw(12) << "--" 
                     << setw(12) << embalses 
                     << " [FALLA SENSOR] Lectura perdida: Equipos apagados" << endl;
                continue;
            }

            int horasRestantesDia = HORAS_DIA - h;
            bool deseoEncender[MAX_CARGAS];

            deseoEncender[0] = cargas[0]->debeEncender(h, precio, embalses);

            for (int i = 1; i < numCargas; i++) {
                CargaAplazable* cap = dynamic_cast<CargaAplazable*>(cargas[i]);
                bool estado = cap->debeEncender(h, precio, embalses);

                int faltantes = cap->getHorasMinimas() - cap->getHorasUso();
                if (faltantes > 0 && horasRestantesDia <= faltantes) {
                    estado = true;
                }
                deseoEncender[i] = estado;
            }

            // Deslastre de Potencia Contratada (Máximo 11.0 kW)
            float potenciaTotalPrevista = 0.0f;
            for (int i = 0; i < numCargas; i++) {
                if (deseoEncender[i]) potenciaTotalPrevista += cargas[i]->getPotencia();
            }

            if (potenciaTotalPrevista > POTENCIA_CONTRATADA) {
                if (deseoEncender[2]) deseoEncender[2] = false; // Deslastre: apaga secadora
            }

            float kwHoraActual = 0.0f;
            for (int i = 0; i < numCargas; i++) {
                if (deseoEncender[i]) {
                    cargas[i]->encender();
                    kwHoraActual += cargas[i]->getPotencia();
                } else {
                    cargas[i]->apagar();
                }
            }

            float costoHoraActual = kwHoraActual * precio;
            medidor.registrarConsumo(kwHoraActual, precio);
            costoCon += costoHoraActual;

            cout << left << setw(6)  << (to_string(h) + ":00")
                 << setw(12) << precio
                 << setw(12) << setprecision(1) << embalses
                 << setw(15) << (cargas[0]->estaEncendida() ? "ON" : "off")
                 << setw(15) << (cargas[1]->estaEncendida() ? "ON" : "off")
                 << setw(15) << (cargas[2]->estaEncendida() ? "ON" : "off")
                 << setw(12) << setprecision(2) << costoHoraActual << endl;
        }

        cout << "---------------------------------------------------------------------------------------" << endl;
        cout << "Simulacion terminada." << endl;
        cout << "Costo total con control inteligente: $" << costoCon << " COP" << endl;
        cout << "Costo total sin control (lineal):    $" << costoSin << " COP" << endl;
        cout << "Ahorro obtenido:                     $" << (costoSin - costoCon) << " COP" << endl;
    }
};

// ============================================================================
// FUNCIÓN MAIN (Prueba de integración general)
// ============================================================================
int main() {
    srand(SEMILLA);
    Mercado mercado;
    Negocio lavanderia("Lavanderia Automatizada El Nido", &mercado);

    // Registrar cargas institucionales
    lavanderia.agregarCarga(new CargaCritica("Bomba/Recepcion", POTENCIA_CRITICA));
    lavanderia.agregarCarga(new CargaAplazable("Lavadora Ind.", POTENCIA_APLAZ1, MIN_HORAS_APLAZ1));
    lavanderia.agregarCarga(new CargaAplazable("Secadora Ind.", POTENCIA_APLAZ2, MIN_HORAS_APLAZ2));

    cout << "=== PRUEBA DE INTEGRACION WATTCONTROL V1 ===" << endl;
    lavanderia.mostrar();

    // Ejecutar simulación completa de 24 horas
    lavanderia.simularDia();

    return 0;
}

