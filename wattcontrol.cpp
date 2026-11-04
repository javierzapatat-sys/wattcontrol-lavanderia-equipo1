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

const int FALLA_CADA = 20;              // 1 de cada 20 lecturas falla// ============================================================================
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