import time
import streamlit as st
import pandas as pd
import requests
import plotly.express as px

# Função para obter os dados do endpoint
def get_data():
    url = 'http://localhost:8080/last-records'
    response = requests.get(url)
    data = response.json()
    return data

# Criar o dashboard com os gráficos
def create_dashboard(data):
    # Converter os dados para DataFrames do Pandas
    df_temperature = pd.DataFrame(data['temperature'])
    df_fall_detection = pd.DataFrame(data['fall_detection'])
    df_panic_alert = pd.DataFrame(data['panic_alert'])
    df_spo2 = pd.DataFrame(data['spo2'])
    df_bpm = pd.DataFrame(data['bpm'])

    # Gráfico de Linha para Temperatura
    st.subheader('Temperatura')
    fig_temperature = px.line(df_temperature, x='reported_at', y='reported_value', title='Temperatura')
    st.plotly_chart(fig_temperature)

    # Gráfico de Linha para SpO2
    st.subheader('SpO2')
    fig_spo2 = px.line(df_spo2, x='reported_at', y='reported_value', title='SpO2')
    st.plotly_chart(fig_spo2)

    # Gráfico de Linha para BPM
    st.subheader('BPM')
    fig_bpm = px.line(df_bpm, x='reported_at', y='reported_value', title='BPM')
    st.plotly_chart(fig_bpm)

    # Gráfico de Séries Temporais Discretas para Fall Detection
    st.subheader('Fall Detection')
    df_fall_detection['value'] = 1  # Cria uma coluna 'value' com valor 1 para cada ocorrência
    fig_fall_detection = px.scatter(df_fall_detection, x='reported_at', y='value', title='Fall Detection',
                                    color='value', labels={'value': 'Ocorrência'})
    fig_fall_detection.update_traces(mode='markers', marker=dict(size=12))  # Define o modo como 'markers'
    st.plotly_chart(fig_fall_detection)

    # Gráfico de Séries Temporais Discretas para Panic Alert
    st.subheader('Panic Alert')
    df_panic_alert['value'] = 1  # Cria uma coluna 'value' com valor 1 para cada ocorrência
    fig_panic_alert = px.scatter(df_panic_alert, x='reported_at', y='value', title='Panic Alert',
                                 color='value', labels={'value': 'Ocorrência'})
    fig_panic_alert.update_traces(mode='markers', marker=dict(size=12))  # Define o modo como 'markers'
    st.plotly_chart(fig_panic_alert)

# Exibir hora da última atualização
def show_last_update_time():
    st.write(f'Última atualização: {time.strftime("%H:%M:%S")}')

# Criar o dashboard inicial
data = get_data()
create_dashboard(data)
show_last_update_time()

# Autorecarregar os dados a cada 10 segundos
while True:
    time.sleep(10)
    st.experimental_rerun()
