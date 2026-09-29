# Prueba y uso del driver TB6612FNG en Arduino

Control de dos motores DC con un **Arduino**, un **joystick analógico** y el driver de motores **TB6612FNG**.
Mueves el joystick y el carro avanza, retrocede o gira. Este documento está escrito para personas que **no tienen conocimientos previos de electrónica**.

---

## 📑 Contenido

1. [¿Qué hace este proyecto?](#1-qué-hace-este-proyecto)
2. [Materiales](#2-materiales)
3. [Conceptos básicos (sin miedo)](#3-conceptos-básicos-sin-miedo)
   - [¿Por qué no conectar el motor directo al Arduino?](#31-por-qué-no-conectar-el-motor-directo-al-arduino)
   - [¿Qué es un puente H?](#32-qué-es-un-puente-h)
   - [¿Qué es PWM?](#33-qué-es-pwm)
   - [El driver TB6612FNG](#34-el-driver-tb6612fng)
4. [Conexiones (diagrama)](#4-conexiones-diagrama)
5. [Cómo funciona el código](#5-cómo-funciona-el-código)
6. [Cómo usarlo](#6-cómo-usarlo)
7. [Problemas comunes](#7-problemas-comunes)
8. [Limitaciones y mejoras posibles](#8-limitaciones-y-mejoras-posibles)
9. [Glosario](#9-glosario)

---

## 1. ¿Qué hace este proyecto?

| Movimiento del joystick | Resultado en el carro |
|---|---|
| Al centro | Los motores se detienen |
| Hacia adelante | Ambos motores giran hacia adelante |
| Hacia atrás | Ambos motores giran hacia atrás |
| Hacia un lado | El carro gira (solo un motor se mueve) |

Mientras más lejos empujes el joystick, **más rápido** giran los motores.

---

## 2. Materiales

- 1 × Arduino (Uno, Nano o similar)
- 1 × Driver de motores **TB6612FNG** (en placa, tipo SparkFun)
- 2 × Motores DC (con reductor, tipo "motor amarillo")
- 1 × Módulo joystick analógico (5 pines: GND, +5V, VRx, VRy, SW)
- 1 × Batería o fuente para los motores (entre **4.5 V y 13.5 V**, por ejemplo 2 pilas de litio 18650 o 6 pilas AA)
- Cables jumper

> ⚠️ **Importante:** los motores **NO** se alimentan desde el Arduino. Necesitan su propia batería. Más abajo se explica por qué.

---

## 3. Conceptos básicos (sin miedo)

### 3.1 ¿Por qué no conectar el motor directo al Arduino?

Piensa en el Arduino como un **cerebro**: sabe tomar decisiones, pero es débil físicamente. Un motor es un **músculo**: necesita mucha más energía (corriente) de la que un pin del Arduino puede dar.

- Un pin del Arduino entrega como máximo ~**20 mA**.
- Un motor pequeño puede pedir **500 mA a 1000 mA** (¡25 a 50 veces más!).

Si conectas el motor directo, el pin se puede **quemar**. Por eso usamos un **driver**: un "intermediario" que recibe las órdenes suaves del Arduino y las convierte en energía fuerte tomada de la batería.

```
   CEREBRO                 MÚSCULO
  (Arduino)  ──órdenes──▶  (Driver)  ──energía──▶  (Motor)
  señales débiles         obedece y usa la batería
```

### 3.2 ¿Qué es un puente H?

Un motor DC gira en un sentido u otro dependiendo de **hacia dónde fluye la corriente**:

- Corriente de izquierda a derecha → gira hacia **adelante**.
- Corriente de derecha a izquierda → gira hacia **atrás**.

Para invertir la corriente se usa un circuito llamado **puente H**, que se llama así porque su dibujo parece una letra H. Son **4 interruptores** (S1, S2, S3, S4) alrededor del motor:

```
        + Batería (VM)
      ┌───────┴───────┐
      │               │
    ┌─┴─┐           ┌─┴─┐
    │S1 │           │S3 │
    └─┬─┘           └─┬─┘
      │               │
      ├───── (M) ─────┤        ← El motor está en el "centro" de la H
      │               │
    ┌─┴─┐           ┌─┴─┐
    │S2 │           │S4 │
    └─┬─┘           └─┬─┘
      │               │
      └───────┬───────┘
          - Tierra (GND)
```

**¿Cómo se usa?** Cerrando (encendiendo) solo ciertos interruptores:

```
  ADELANTE                         ATRÁS
  S1 y S4 cerrados                 S2 y S3 cerrados

   +                                +
   ┌──┴───────┐                     ┌───────┴──┐
  [S1]       S3                     S1       [S3]
   ├──(M) ──►─┤                     ├─◄─(M)────┤
   S2       [S4]                   [S2]       S4
   └───────┬──┘                     └──┴───────┘
           -                        -

  La corriente va de izquierda      La corriente va de derecha
  a derecha → motor gira ↻          a izquierda → motor gira ↺
```

> 🚫 Nunca se cierran **S1 y S2** al mismo tiempo (ni **S3 y S4**): sería un cortocircuito directo entre la batería y tierra. El driver **TB6612FNG ya trae esto protegido**, por eso es más seguro que armarlo tú mismo.

**¿Y qué tiene que ver con el código?** Los pines `IA1` e `IA2` (y `IB1` e `IB2`) son los que le dicen al driver **cuáles interruptores cerrar**, es decir, **en qué dirección** girar el motor.

### 3.3 ¿Qué es PWM?

**PWM** significa *Pulse Width Modulation* (Modulación por Ancho de Pulso). Suena complicado, pero la idea es simple.

El Arduino solo puede hacer dos cosas con un pin: **encendido (5 V)** o **apagado (0 V)**. No puede dar "2.5 V" directamente. Entonces, ¿cómo controlar la velocidad de un motor?

**Truco: encender y apagar muy rápido.**

Imagina que enciendes y apagas una luz 500 veces por segundo. Tus ojos no ven el parpadeo: solo ven una luz **más tenue**. Con el motor pasa igual: recibe pulsos tan rápidos que su inercia los "promedia".

Lo que cambia es **cuánto tiempo pasa encendido** dentro de cada ciclo (a eso se le llama *ciclo de trabajo*):

```
 0% (parado)          ________________________________  0 V
                      (siempre apagado)

 25% (lento)          ┌─┐     ┌─┐     ┌─┐     ┌─┐
                    ──┘ └─────┘ └─────┘ └─────┘ └────

 50% (medio)          ┌───┐   ┌───┐   ┌───┐   ┌───┐
                    ──┘   └───┘   └───┘   └───┘   └──

 75% (rápido)         ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
                    ──┘     └─┘     └─┘     └─┘     └─

 100% (máx.)          ┌──────────────────────────────  5 V
                    ──┘  (siempre encendido)
```

En Arduino esto se hace con `analogWrite(pin, valor)`:

| `valor` | Ciclo de trabajo | Efecto en el motor |
|---|---|---|
| 0 | 0 % | Detenido |
| 64 | ≈ 25 % | Lento |
| 127 | ≈ 50 % | Velocidad media |
| 191 | ≈ 75 % | Rápido |
| 255 | 100 % | Máxima velocidad |

> 📌 Solo algunos pines del Arduino Uno/Nano pueden hacer PWM: los marcados con **~** (3, 5, 6, 9, 10, 11). Por eso `PWMA` está en el pin 5 y `PWMB` en el 6.

### 3.4 El driver TB6612FNG

Es un chip que trae **dos puentes H** dentro, así que puede controlar **2 motores** de forma independiente (motor A y motor B). Cada motor necesita **3 señales** del Arduino:

| Señal | ¿Para qué sirve? |
|---|---|
| **PWMx** | Controla la **velocidad** (usa PWM) |
| **xIN1** | Junto con xIN2, decide la **dirección** |
| **xIN2** | Junto con xIN1, decide la **dirección** |

Además hay un pin general:

| Pin | ¿Para qué sirve? |
|---|---|
| **STBY** | "Interruptor general". En `HIGH` el driver trabaja; en `LOW` se duerme y los motores no se mueven |

#### Tabla de verdad (cómo se combinan las señales)

| IN1 | IN2 | PWM | Resultado |
|:---:|:---:|:---:|---|
| HIGH | LOW | Valor PWM | Gira en un sentido (**adelante**) a la velocidad del PWM |
| LOW | HIGH | Valor PWM | Gira en sentido contrario (**atrás**) |
| LOW | LOW | — | Motor libre (**se detiene**, sin frenar) |
| HIGH | HIGH | — | **Freno** (el motor se frena activamente) |

> Nota: que el carro avance con `HIGH/LOW` o con `LOW/HIGH` depende de **cómo conectaste los cables del motor** (A01/A02 y B01/B02). Si al mover el joystick hacia adelante un motor gira al revés, simplemente **invierte sus dos cables** en el driver.

#### Pines de la placa

```
                 TB6612FNG (vista superior)
        ┌─────────────────────────────────┐
  PWMA ─┤●                               ●├─ VM   (batería +)
  AIN2 ─┤●                               ●├─ VCC  (5 V lógica)
  AIN1 ─┤●                               ●├─ GND
  STBY ─┤●                               ●├─ AO1  ┐ Motor A
  BIN1 ─┤●                               ●├─ AO2  ┘
  BIN2 ─┤●                               ●├─ BO2  ┐ Motor B
  PWMB ─┤●                               ●├─ BO1  ┘
   GND ─┤●                               ●├─ GND
        └─────────────────────────────────┘
        ◄── Del Arduino ──►   ◄─ Energía y Motores ─►
```

- **Lado izquierdo:** las señales que vienen del Arduino (las "órdenes").
- **Lado derecho:** la energía (VM, VCC, GND) y las salidas hacia los motores.

---

## 4. Conexiones (diagrama)

### 4.1 Tabla de conexiones

**Arduino → Driver TB6612FNG**

| Arduino | Driver | Función |
|:---:|:---:|---|
| Pin 5 | PWMA | Velocidad motor A |
| Pin 8 | AIN1 | Dirección motor A |
| Pin 9 | AIN2 | Dirección motor A |
| Pin 6 | PWMB | Velocidad motor B |
| Pin 11 | BIN1 | Dirección motor B |
| Pin 10 | BIN2 | Dirección motor B |
| Pin 4 | STBY | Activar / desactivar driver |
| 5V | VCC | Alimentación del chip (lógica) |
| GND | GND | Tierra común |

**Alimentación y motores**

| Origen | Destino |
|---|---|
| Batería **(+)** | **VM** del driver |
| Batería **(–)** | **GND** del driver (y también al GND del Arduino) |
| Motor A (2 cables) | **AO1** y **AO2** |
| Motor B (2 cables) | **BO1** y **BO2** |

**Joystick → Arduino**

| Joystick | Arduino |
|:---:|:---:|
| GND | GND |
| +5V | 5V |
| VRx | A0 |
| VRy | A1 |

### 4.2 Diagrama de cableado

El driver está dibujado con la **misma forma que la placa real**: señales del Arduino a la izquierda, energía y motores a la derecha.

```
                              ┌───────────────────────────────┐
   ARDUINO                    │        TB6612FNG              │
                              │                               │
   Pin 5  ────────────────────┤ PWMA                       VM ├──────── Batería (+) / 7.4V
   Pin 9  ────────────────────┤ AIN2                      VCC ├──────── Arduino 5V
   Pin 8  ────────────────────┤ AIN1                      GND ├──────── Batería (–) / GND
   Pin 4  ────────────────────┤ STBY                      AO1 ├───┐
   Pin 11 ────────────────────┤ BIN1                      AO2 ├───┴──── [ MOTOR A ]
   Pin 10 ────────────────────┤ BIN2                      BO2 ├───┐
   Pin 6  ────────────────────┤ PWMB                      BO1 ├───┴──── [ MOTOR B ]
   GND    ────────────────────┤ GND                       GND ├──────── GND común
                              │                               │
                              └───────────────────────────────┘

   JOYSTICK
   ┌──────────┐
   │ GND ─────┼──── GND
   │ +5V ─────┼──── 5V
   │ VRx ─────┼──── A0
   │ VRy ─────┼──── A1
   └──────────┘
```

> ⚠️ **Regla de oro:** todos los **GND** (Arduino, driver, batería, joystick) deben estar **unidos**. Sin una tierra común, las señales no se entienden entre sí y el motor se comporta de forma errática o no funciona.

> ⚠️ **No conectes la batería a VCC.** La batería va a **VM**. VCC es solo para los 5 V del Arduino.

---

## 5. Cómo funciona el código

### 5.1 Definición de pines

```cpp
#define PWMA 5     // Velocidad motor A
#define IA1 8      // Dirección motor A
#define IA2 9

#define PWMB 6     // Velocidad motor B
#define IB1 11     // Dirección motor B
#define IB2 10

#define STBY 4     // Activa el driver

#define JSX A0     // Joystick eje horizontal
#define JSY A1     // Joystick eje vertical
```

`#define` le pone un nombre fácil de recordar a cada número de pin, para no tener que memorizar que "el 5 es la velocidad del motor A".

### 5.2 `setup()`: preparación

Se ejecuta **una sola vez** al encender:

1. Configura los pines del driver como **salidas** (el Arduino les envía órdenes).
2. Configura los pines del joystick como **entradas** (el Arduino los lee).
3. Abre la comunicación serie a `115200` para poder ver mensajes en el PC.
4. Pone `STBY` en `HIGH` para **despertar** al driver.

### 5.3 Lectura del joystick

El joystick es en realidad **dos potenciómetros** (perillas variables), uno por eje. El Arduino lee cada uno como un número de **0 a 1023**:

```
              Y = 1023 (arriba)
                  ▲
                  │
  X = 0 ◄─────────┼─────────► X = 1023
 (derecha)        │            (izquierda)
                  ▼
              Y = 0 (abajo)

  En reposo el joystick marca aprox. X ≈ 520, Y ≈ 507 (el "centro")
```

Para evitar lecturas "nerviosas", el código **lee 10 veces y saca el promedio**:

```cpp
for (size_t i = 0; i < 10 ; i++) {
  sumaX += analogRead(JSX);
  sumaY += analogRead(JSY);
}
int x = sumaX / 10;
int y = sumaY / 10;
```

### 5.4 Zona muerta (el "centro")

Un joystick nunca marca exactamente el mismo valor en reposo; oscila un poquito. Por eso el código define una **zona muerta**: un rango donde se considera "quieto".

```cpp
bool xCentro = x > 500 && x < 540;
bool yCentro = y > 487 && y < 527;
```

```
   0                500 ─── 540                1023
   ├─────────────────┼────────┼──────────────────┤
     Derecha           CENTRO         Izquierda
                    (zona muerta)
```

### 5.5 Decisión de movimiento

```
¿Está en el centro?  ──Sí──▶  stop()
        │
       No
        ▼
 X < 500 y Y centro   ──▶ girarDerecha()
 X > 540 y Y centro   ──▶ girarIzquierda()
 Y > 527 y X centro   ──▶ adelante()
 Y < 487 y X centro   ──▶ atras()
```

### 5.6 Funciones de movimiento

| Función | Motor A | Motor B | Resultado |
|---|---|---|---|
| `adelante()` | Adelante | Adelante | El carro avanza |
| `atras()` | Atrás | Atrás | El carro retrocede |
| `girarDerecha()` | Detenido | Adelante | El carro gira hacia la derecha |
| `girarIzquierda()` | Adelante | Detenido | El carro gira hacia la izquierda |
| `stop()` | Detenido | Detenido | El carro se detiene |

### 5.7 `cambiarVelocidadPWM()`

```cpp
void cambiarVelocidadPWM(int PWM, int value) {
  int v = map(value, 0, 1023, 0, 180);
  int V = constrain(v, 0, 180);
  analogWrite(PWM, V);
}
```

- `map(...)` **convierte** un número de un rango a otro. Aquí toma el valor del joystick (0–1023) y lo lleva a un rango de 0–180.
- `constrain(...)` se asegura de que el resultado **nunca se salga** del rango 0–180.
- `analogWrite(...)` envía el PWM al driver.

El límite de **180** (en vez de 255) sirve para **no darle el máximo de potencia** a los motores.

---

## 6. Cómo usarlo

1. **Arma** el circuito siguiendo el [diagrama](#42-diagrama-de-cableado).
2. Abre el código en el **Arduino IDE** y súbelo a la placa.
3. Abre el **Monitor Serie** a `115200` baudios: verás los valores `X` y `Y` del joystick y la dirección detectada.
4. **Conecta la batería** de los motores.
5. Mueve el joystick 🎮.

> 💡 **Recomendación para la primera prueba:** levanta el carro (que las ruedas queden en el aire) para ver hacia dónde gira cada motor antes de dejarlo correr por el suelo.

---

## 7. Problemas comunes

| Síntoma | Posible causa | Solución |
|---|---|---|
| Los motores no se mueven | `STBY` no está en HIGH o el cable está suelto | Revisa la conexión del pin 4 |
| Los motores no se mueven | Batería sin conectar a **VM** | Conecta el (+) de la batería a VM |
| Comportamiento errático | GND no compartido | Une todos los GND |
| Un motor gira al revés | Cables del motor invertidos | Intercambia sus dos cables en el driver |
| El carro se mueve solo con el joystick quieto | Joystick descalibrado | Mira los valores X/Y en el Monitor Serie y ajusta los rangos de la zona muerta |
| El Arduino se reinicia al arrancar los motores | Caída de voltaje | Usa una batería con más capacidad y verifica las conexiones |
| El chip se calienta mucho | Motores piden demasiada corriente | El TB6612FNG soporta ~1.2 A continuos por motor; usa motores más pequeños |

---

## 8. Limitaciones y mejoras posibles

Estas son cosas que conviene conocer del código actual:

1. **Solo un eje a la vez.** Si mueves el joystick en diagonal (por ejemplo adelante + derecha) ninguna condición se cumple y los motores **siguen con la última orden** que recibieron. Una mejora sería mezclar ambos ejes (*mezcla diferencial*) para poder girar mientras avanzas.
2. **Escalas de velocidad distintas.** `adelante()` y `atras()` pasan por `cambiarVelocidadPWM()` (máximo 180), mientras que `girarDerecha()` y `girarIzquierda()` usan `analogWrite()` directo con rango 0–255. Por eso al girar el carro puede ir más rápido que al avanzar.
3. **Arranque brusco al avanzar/retroceder.** Al pasar la zona muerta, el valor que llega ya es alto (≈ 527 → PWM ≈ 93 ), así que el motor arranca "de golpe" en lugar de suave. Se puede solucionar restando la zona muerta antes de calcular la velocidad, como ya se hace en los giros.
4. **Giro sobre una rueda.** Al girar, un motor se detiene y el otro avanza, por lo que el carro pivota alrededor de una rueda. Para girar sobre su propio eje, un motor debería ir adelante y el otro atrás.
5. **Botón del joystick sin usar.** El pin SW del joystick puede usarse para, por ejemplo, activar un modo "turbo" o una bocina.

---

## 9. Glosario

| Término | Significado sencillo |
|---|---|
| **Arduino** | Pequeña computadora que ejecuta tu programa y controla componentes |
| **Driver** | Circuito intermedio que maneja la energía de los motores por orden del Arduino |
| **Puente H** | Arreglo de 4 interruptores que permite girar un motor en ambos sentidos |
| **PWM** | Técnica de encender/apagar muy rápido para simular voltajes intermedios y controlar velocidad |
| **Ciclo de trabajo** | Porcentaje de tiempo que la señal PWM está encendida |
| **Corriente (A)** | Cantidad de electricidad que fluye; los motores necesitan mucha más que un pin del Arduino |
| **Voltaje (V)** | La "fuerza" con que la electricidad empuja |
| **GND / Tierra** | Referencia de 0 V; todos los componentes deben compartirla |
| **VM** | Alimentación de los **M**otores (la batería) |
| **VCC** | Alimentación de la lógica del chip (5 V) |
| **STBY** | *Standby*: modo de espera del driver |
| **Potenciómetro** | Resistencia variable; lo que hay dentro de cada eje del joystick |
| **Zona muerta** | Rango central del joystick que se ignora para evitar movimientos no deseados |

---

Hecho con 💡 para aprender electrónica desde cero.
