import pandas as pd
import matplotlib.pyplot as plt

# Archivos a graficar
archivos = [
    ("Pi_noRT_1.csv", "Pi_RT_1.csv"),
    ("Pi_noRT_2.csv", "Pi_RT_2.csv"),
    ("Pi_noRT_3.csv", "Pi_RT_3.csv")
]

# --- 1. Calcular rango global de latencias ---
min_lat = float('inf')
max_lat = float('-inf')

for no_rt, rt in archivos:
    df_noRT = pd.read_csv(no_rt)
    df_RT = pd.read_csv(rt)

    df_noRT.columns = [c.strip().upper() for c in df_noRT.columns]
    df_RT.columns = [c.strip().upper() for c in df_RT.columns]

    if "LATENCIA" not in df_noRT.columns or "LATENCIA" not in df_RT.columns:
        continue

    # Convertir a microsegundos
    df_noRT["LATENCIA_US"] = df_noRT["LATENCIA"] / 1000.0
    df_RT["LATENCIA_US"] = df_RT["LATENCIA"] / 1000.0

    # Actualizar mínimos y máximos globales
    min_lat = min(min_lat, df_noRT["LATENCIA_US"].min(), df_RT["LATENCIA_US"].min())
    max_lat = max(max_lat, df_noRT["LATENCIA_US"].max(), df_RT["LATENCIA_US"].max())

# --- 2. Graficar con rango común ---
for i, (no_rt, rt) in enumerate(archivos, start=1):
    df_noRT = pd.read_csv(no_rt)
    df_RT = pd.read_csv(rt)

    df_noRT.columns = [c.strip().upper() for c in df_noRT.columns]
    df_RT.columns = [c.strip().upper() for c in df_RT.columns]

    if "LATENCIA" not in df_noRT.columns or "LATENCIA" not in df_RT.columns:
        print(f"Columnas LATENCIA no encontradas en grupo {i}")
        continue

    # Convertir a microsegundos
    df_noRT["LATENCIA_US"] = df_noRT["LATENCIA"] / 1000.0
    df_RT["LATENCIA_US"] = df_RT["LATENCIA"] / 1000.0

    # Calcular desviaciones estándar
    std_noRT = df_noRT["LATENCIA_US"].std()
    std_RT = df_RT["LATENCIA_US"].std()

    # Crear histograma
    plt.figure(figsize=(7, 5))
    plt.hist(df_noRT["LATENCIA_US"], bins=70, color='steelblue', alpha=0.7,
             label=f'Pi_noRT (σ = {std_noRT:.2f} µs)', range=(min_lat, max_lat))
    plt.hist(df_RT["LATENCIA_US"], bins=70, color='sandybrown', alpha=0.7,
             label=f'Pi_RT (σ = {std_RT:.2f} µs)', range=(min_lat, max_lat))

    plt.title(f"Latency Distribution - Grupo {i}", fontsize=13)
    plt.xlabel("Latency (microsec.)", fontsize=11)
    plt.ylabel("Frequency", fontsize=11)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig(f"histograma_grupo_{i}.png", dpi=150)

plt.show()
