# Projeto Cofre Eletrônico

![Logo](https://github.com/DouglasOliveiraC/CofrePIC/blob/main/telaNPI7.png)

Bem-vindo ao Projeto Cofre Eletrônico! Este projeto é uma proposta de solução para um trabalho escolar, desenvolvido com o objetivo de aprender e aplicar conceitos de eletrônica e programação embarcada.

## Índice

- [Sobre](#sobre)
- [Recursos](#recursos)
- [Instalação](#instalação)
- [Uso](#uso)
- [Contribuindo](#contribuindo)
- [Licença](#licença)
- [Contato](#contato)

## Sobre

O Projeto Cofre Eletrônico foi desenvolvido como parte de um trabalho escolar para fornecer uma solução de controle de acesso utilizando um teclado matricial e senha. Este projeto é puramente educacional e não é indicado para aplicações de segurança real.

## Recursos

- **Teclado Matricial**: Captura e validação de entrada de senha.
- **Display de 7 Segmentos**: Feedback visual durante a entrada da senha.
- **Motor de Passo**: Controle de abertura e fechamento do cofre.
- **Segurança de Senha**: Validação e armazenamento seguro da senha.
- **Feedback de Erro/Sucesso**: Indicação visual de sucesso ou erro na entrada da senha.
- **Timeout na Inserção do teclado**: Um timeout de 1 minuto para inserção de senha no teclado foi estabelecido.

## Instalação

### Pré-requisitos

- **Compilador XC8**
- **MPLAB X IDE**
- **Microcontrolador PIC16F886**
- **SimulIDE**

### Passos para Instalação

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/projeto-cofre-eletronico.git
   ```
2. Navegue até o diretório do projeto: 
   ```bash
   cd projeto-cofre-eletronico
   ```
3. Abra o projeto no MPLAB X IDE.

4. Abra o arquivo de simulação.

5. Compile o código usando o compilador XC8.

6. Carregue o firmware no microcontrolador.

## Uso

Operação Básica

- Ligue o sistema: Aperte "*" por 2 seg aproximadamente.
- Digite a senha: Utilize o teclado matricial para inserir a senha de 4 dígitos após mensagem "0000", logo após para abrir apenas pressione "*" e digite a senha, pressione "#" para validar as operações.
- A senha correta acionará o motor de passo para abrir o cofre.
- A senha incorreta exibirá um erro no display de 7 segmentos.

## Anomalias Observadas

Durante o desenvolvimento e testes, as seguintes anomalias foram observadas:

- Há resquícios das Mensagens na tela no caso de Introdução de Senha Válida e também na Abertura do cofre. Juntamente a esses problemas há travamento da execução, que podem estar relacionados a anomalias no Simulador.


## Contribuindo

Contribuições são bem-vindas! Se você deseja contribuir, por favor, siga os passos abaixo:

1. Faça um fork do projeto.
2. Crie uma branch para sua feature ou correção.
3. Realize as alterações necessárias.
4. Faça o commit das suas alterações.
5. Faça o push para o seu fork.
6. Abra um pull request.

## Licença

Este projeto está licenciado sob a [MIT License](LICENSE).

## Contato

Para mais informações, entre em contato pelo email: douoliver@gmail.com

