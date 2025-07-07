

## 🌱 Projeto Agrinho 2025
Sistema Automatizado de Separação de Alimentos por Cor com Robótica Educacional
Tema: Festejando a Conexão entre o Campo e a Cidade
Turma: 3º Serie B Integral-Colégio Reni

## 🧠 Sobre o Projeto
Este projeto simula um sistema inteligente de triagem de frutas e vegetais, inspirado em tecnologias reais usadas na agricultura para separar alimentos antes do envio à cidade. Com uso de sensores, motores, um braço robótico e programação em Arduino, o sistema é capaz de:

✅ Posicionar objetos (alimentos) na esteira
✅ Detectar a presença do objeto usando sensor ultrassônico
✅ Identificar a cor do objeto
✅ Separar o objeto automaticamente, fechando uma porta se ele for vermelho
✅ Operar em modo automático ou manual, via botão

## ⚙️ Componentes Utilizados.
| Componente | Função |
|--|--|
| Arduino Mega 2560 |Unidade de controle  |
| Sensor Ultrassônico HC-SR04| Detecção de presença do objeto |
| Sensor de Cor TCS3200 | Leitura RGB para identificar a cor |
| 4 Servomotores SG90 | Movimentação do braço robótico |
| Motor DC com L298N | Acionamento da esteira transportadora |
| Display OLED 128x64	 | Interface visual com modo e cor detectada |
| Potenciômetros (4x) | Controle dos servos no modo manual |
| Botões | Alternar entre modo automático/manual |
| Fonte externa |Alimentação dos motores  |


	


	
	
	

💻 Estrutura do Código
O código está dividido em blocos funcionais:

- setup(): inicialização de componentes, display e posições do braço.

- loop(): controle do fluxo entre modo automático e manual

- executarModoAutomatico(): rotina automatizada com sensores e esteira

- executarModoManual(): controle do braço via potenciômetros

- detectarCor(): leitura RGB e decisão baseada na cor

- moverParaPosicao(): transições suaves entre posições do braço

- ligarEsteira(), desligarEsteira(): controle PWM da esteira

## 📊 Interface OLED
A tela mostra informações como:
```c
Agrinho
Modo: Automatico
Cor: VERMELHO
V:2 O:1
```
Modo: Manual ou Automático
Cor: Última cor detectada
V/O: Quantidade de objetos vermelhos (V) e outros (O)

## 🧪 Testes e Demonstrações.
Modo automático: simula a linha de separação na agricultura

Modo manual: permite controlar cada eixo do braço robótico

Testado com tampinhas de garrafa e blocos coloridos.

📎 Código-Fonte
O código completo está disponível no repositório:
📁 https://github.com/professorrichardson/EsteiraSeletoraAgrinho2025/blob/main/Agrinho2025.ino 
