import tkinter as tk
import serial
import time
import threading
import csv
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Função para conectar à porta serial
def conectar_serial():
    try:
        global serial_port
        serial_port = serial.Serial('COM6', 115200, timeout=1)  # Ajustar a porta conforme necessário
        time.sleep(2)  # Dar tempo para inicializar a conexão
        tk.messagebox.showinfo("Conexão", "Conectado ao ESP32!")
        iniciar_leitura_serial()
    except Exception as e:
        tk.messagebox.showerror("Erro", f"Erro ao conectar à porta serial: {str(e)}")

# Variáveis globais
linhas_lidas = 0
distancias = []  # Lista para armazenar distâncias
tempos = []  # Lista para armazenar tempos
velocidades = []
picos = []  # Lista para armazenar picos detectados

# Função para calcular velocidade
def calcular_velocidade():
    velocidades = []
    for i in range(1, len(distancias)):
        delta_dist = distancias[i] - distancias[i - 1]
        delta_tempo = tempos[i] - tempos[i - 1]
        velocidade = delta_dist / delta_tempo if delta_tempo != 0 else 0
        velocidades.append(velocidade)
    return velocidades

# Função para ler dados da porta serial
def iniciar_leitura_serial():
    def ler_dados():
        global linhas_lidas, tempo_inicio
        tempo_inicio = time.time()
        while True:
            if serial_port.is_open:
                linha = serial_port.readline().decode('utf-8').strip()
                if linha:
                    linhas_lidas += 1
                    if linhas_lidas > 10:  # Ignorar as primeiras 10 linhas
                        atualizar_valores(linha)

    thread_leitura = threading.Thread(target=ler_dados)
    thread_leitura.daemon = True
    thread_leitura.start()

# Função para atualizar os gráficos
def atualizar_valores(dados):
    try:
        # Separar os valores de velocidade, distância e picos
        if "Velocidade:" in dados and "Distancia:" in dados and "Pico:" in dados:
            partes = dados.split(",")  # Divide a string pelo delimitador ','

            # Extrai a velocidade
            velocidade_str = partes[0].split(":")[1].strip().replace("Km/H", "")
            velocidade = float(velocidade_str)

            # Extrai a distância
            distancia_str = partes[1].split(":")[1].strip().replace("Cm", "")
            distancia = float(distancia_str)

            # Extrai o pico
            pico_str = partes[2].split(":")[1].strip().replace("V", "")
            pico = float(pico_str)

            # Atualiza os dados globais
            tempo_atual = time.time() - tempo_inicio  # Calcula o tempo decorrido
            tempos.append(tempo_atual)
            distancias.append(distancia)
            velocidades.append(velocidade)
            picos.append(pico)

            # Atualiza os gráficos
            ax1.clear()
            ax2.clear()
            ax3.clear()

            # Gráfico de Distância
            ax1.plot(tempos, distancias, label="Distância (cm)", color="blue")
            ax1.set_title("Distância em Tempo Real")
            ax1.set_xlabel("Tempo (s)")
            ax1.set_ylabel("Distância (cm)")
            ax1.legend(loc="upper left")

            # Gráfico de Velocidade
            ax2.plot(tempos, velocidades, label="Velocidade (Km/h)", color="red")
            ax2.set_title("Velocidade em Tempo Real")
            ax2.set_xlabel("Tempo (s)")
            ax2.set_ylabel("Velocidade (Km/h)")
            ax2.legend(loc="upper left")

            # Gráfico de Picos
            ax3.plot(tempos, picos, label="Picos (cm)", color="green")
            ax3.set_title("Picos em Tempo Real")
            ax3.set_xlabel("Tempo (s)")
            ax3.set_ylabel("Picos (cm)")
            ax3.legend(loc="upper left")

            # Atualiza o Canvas
            canvas.draw()

    except ValueError as e:
        tk.messagebox.showerror("Erro", f"Erro ao processar os dados recebidos: {dados}\nDetalhes: {str(e)}")

# Função para exportar os dados para CSV
def exportar_csv():
    ficheiro = tk.filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV files", "*.csv")])
    if ficheiro:
        try:
            with open(ficheiro, mode='w', newline='') as f:
                escritor_csv = csv.writer(f)
                escritor_csv.writerow(["Tempo (s)", "Distância (cm)", "Velocidade (Km/h)", "Pico (cm)"])
                for i in range(len(tempos)):
                    tempo = round(tempos[i], 2)
                    distancia = round(distancias[i], 2)
                    velocidade = round(velocidades[i - 1] if i > 0 else 0, 2)
                    pico = round(picos[i], 2)
                    escritor_csv.writerow([tempo, distancia, velocidade, pico])
            tk.messagebox.showinfo("Sucesso", "Dados exportados com sucesso para CSV!")
        except Exception as e:
            tk.messagebox.showerror("Erro", f"Erro ao exportar para CSV: {str(e)}")

# Função para reiniciar o gráfico
def reiniciar_grafico():
    global tempos, distancias, linhas_lidas, velocidades, picos
    tempos = []
    distancias = []
    velocidades = []
    picos = []
    linhas_lidas = 0

    ax1.clear()
    ax2.clear()
    ax3.clear()
    ax1.set_title("Distância em Tempo Real")
    ax1.set_xlabel("Tempo (s)")
    ax1.set_ylabel("Distância (cm)")

    ax2.set_title("Velocidade em Tempo Real")
    ax2.set_xlabel("Tempo (s)")
    ax2.set_ylabel("Velocidade (Km/h)")

    ax3.set_title("Picos em Tempo Real")
    ax3.set_xlabel("Tempo (s)")
    ax3.set_ylabel("Picos (cm)")

    canvas.draw()

# Configuração da janela principal
root = tk.Tk()
root.title("Monitor de Distância, Velocidade e Picos")

# Frame para organizar os botões
frame_botoes = tk.Frame(root)
frame_botoes.pack(pady=10)

# Botão para conectar à porta serial
btn_conectar = tk.Button(frame_botoes, text="Conectar ao ESP32", command=conectar_serial)
btn_conectar.pack(side="top", pady=5)

# Botão para reiniciar o gráfico
btn_reiniciar_grafico = tk.Button(frame_botoes, text="Reiniciar Gráfico", command=reiniciar_grafico)
btn_reiniciar_grafico.pack(side="top", pady=5)

# Botão para exportar os dados para CSV
btn_exportar_csv = tk.Button(frame_botoes, text="Exportar para CSV", command=exportar_csv)
btn_exportar_csv.pack(side="top", pady=5)


# Gráfico de distância, velocidade e picos
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(6, 12))
fig.subplots_adjust(hspace=0.5)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()
canvas.get_tk_widget().pack(pady=10)



root.mainloop()
