# Projeto de criptografia aplicada (TP1)

## Introdução

Neste projeto desenvovolvemos um serviço de Message Relay(em Python), que permite aos utilizadores trocarem mensagens entre si com garantias de autenticidade e impossibilidade de interceção de mensagens.
O sistema permite que os clientes enviem mensagens para o servidor de forma segura, consultem as mensagens por ler.

## Objetivo

Este projeto tem como objetivo a implementação de um sistema de comunicação seguro entre membros de uma organização, utilizando a criptografia para assegurar a autenticidade e integridade de todas as mensagens trocadas entre utilizadores. O projeto prevê ainda que seja impossível que utilizadores fora da rede consigam quer intercetar as mensagens, quer sequer conectar-se com o servidor.


## Arquitetura do Sistema

Sistema composto por dois programas: 

- __msg_server.py__ 

Servidor responsável por responder às solicitações dos utilizadores e conservar o estado da aplicação.

- __msg_client.py__ 

Cliente (executado por cada utilizador) que permite o acesso à funcionalidade oferecida pelo serviço em si.

- __aux.py__ 

Contém funções utilitárias para manipulação de certificados(validação, parser e serialização), criação de pares de dados. Essas funções foram projetadas para promover a reutilização de código em diferentes partes do projeto.

## Segurança na comunicação

De modo a assegurar uma comunição protegida entre o servidor e clientes contra acessos ilegítimos e/ou manipulção, a mensagens entre são encriptadas por encriptação AES-GCM (os parâmetros p e g estão fixos no código mas reconhecemos como grupo que não é uma prática ideal epresentativa do mundo real, esta simplificação foi feita para facilitar a implementação e focar nos aspectos principais do projeto) sendo que utilizam o procolo Diffie_Hellman para acordarem numa chave. Para além disso, para assegurar a autenticidade e integridade do programa, é utilizado chaves do algoritmo de assinatura RSA.
De forma a garantir que um cliente leia apenas mensagens que são direcionadas para si, impedindo que o server tente enviar as mensagens para destinatários difentes, o cliente que manda a mensagem efetua uma assinatura num pair, conteúdo da mensagem e uid para quem pretende enviar, o cliente que tenta ler a mensagem executa uma verificação para ver se o uid do par corresponde com o seu.  

## Melhorias Futuras

Uma funcionabilidade que gostariamos de ter implementado é a confidencialidade das mensagens perante um servidor "curioso". O objetivo era implementar a criptografia end-to-end (E2EE) para assegurar que apenas os remetentes e destinatários das mensagens pudessem acessar seu conteúdo. No entanto, devido à complexidade de alterar a estrutura existente do sistema, não foi possível realizar essa implementação.

## Conclusão

Através da implementação deste sistema, pudemos aprofundar,como grupo, o nosso entendimento sobre a importância da criptografia na segurança e autenticidade das comunicações.O projeto conseguiu alcançar os objetivos básicos propostos, demonstrando a viabilidade da aplicação de técnicas criptográficas para proteger a integridade das mensagens trocadas entre os usuários e garantir a sua autenticidade. Estamos cientes da necessidade de fortalecer a segurança e a confiabilidade das comunicações no sistema.