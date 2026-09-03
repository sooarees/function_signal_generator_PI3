<img width="188" height="150" alt="diagrama-blocos-simplificado" src="https://github.com/user-attachments/assets/cc674f3c-7684-4ba3-8a4a-133d460f986e" /># Etapa 1

**(MÍNIMO DE 600 E MÁXIMO DE 1000 PALAVRAS no total do arquivo md.)**

A Etapa 1 compreende o planejamento e a definição da arquitetura do gerador de funções microcontrolado. Nesta fase, será elaborado o diagrama de blocos do sistema, escolhido o microcontrolador e definidos os componentes da interface, como display e botões. Também serão determinadas a topologia do circuito analógico e as características ajustáveis do equipamento, incluindo formas de onda, amplitude, frequência, duty cycle e offset.


## Desenvolvimento

### DAC x DDS

O primeiro estudo comparou um sintetizador digital direto (DDS) com a síntese pelo microcontrolador e saída por DAC. O DDS oferece boa resolução e troca rápida de frequência com menor carga de processamento [1], mas concentra a geração em um componente pronto. Com o DAC, o firmware produz e transfere as amostras, exigindo maior cuidado com temporização e filtragem, porém dando à equipe controle sobre a formação das ondas. Após consulta aos professores, escolheu-se essa solução pelo caráter mais autoral e didático.

### DAC interno x DAC externo

Um DAC interno reduziria componentes e conexões, mas sua resolução, velocidade e saída dependeriam do microcontrolador. O DAC externo exige uma interface adicional, porém pode ser escolhido, testado e substituído separadamente. Essa flexibilidade e a separação entre processamento digital e condicionamento analógico motivaram a escolha do conversor externo.

### Escolha do DAC: AD5689 x DAC0808

O AD5689, já disponível para a equipe, foi uma das opções consideradas. Ele possui dois canais, 16 bits, saída em tensão e comunicação SPI [2], favorecendo a precisão. Entretanto, sua atualização serial e seu tempo de acomodação limitariam a frequência de uma senoide representada por pelo menos 50 pontos.

O DAC0808 foi escolhido por priorizar velocidade: possui 8 bits, entrada paralela, saída em corrente e tempo típico de acomodação de 150 ns [3]. Uma senoide de 100 kHz com 50 pontos exige 5 MS/s, meta teoricamente compatível com o componente e que ainda será validada no sistema completo. Como limitações, ele ocupa oito GPIOs, oferece 256 níveis e requer conversão corrente-tensão.

### Definição dos parâmetros ajustáveis

Foram definidos como parâmetros preliminares: ondas senoidal, triangular, quadrada e dente de serra; frequência de 1 Hz a 100 kHz; amplitude máxima de 10 Vpp; offset de -5 V a +5 V; e impedância de saída de 50 Ω. Os limites de amplitude e offset, inclusive suas combinações, ainda serão validados. O equipamento será alimentado pela rede elétrica, com isolamento, retificação, filtragem e regulação para os circuitos digitais e analógicos.

### Escolha do microcontrolador

A escolha considerou a geração contínua para o DAC0808, temporização, DMA, GPIOs e interface touchscreen. Na condição mais exigente, 100 kHz com 50 pontos requer 5 MS/s, ou uma atualização a cada 200 ns. Por isso, a transferência não deve depender de uma interrupção da CPU para cada amostra.

| Microcontrolador | Recursos relevantes | Avaliação |
| --- | --- | --- |
| MSP430FR5994 | 16 MHz, DMA e plataforma conhecida [4] | Apenas 3,2 ciclos de clock por amostra a 5 MS/s; pouca margem para geração e interface |
| RP2040 | Dois Cortex-M0+ a 133 MHz, DMA e oito máquinas PIO [5] | Muito competente para controlar o DAC; interface gráfica exigiria mais recursos externos |
| ESP32-S3 | Dois núcleos a 240 MHz, GDMA e interface LCD paralela [6] | Forte para geração e interface; Wi-Fi e Bluetooth não são necessários atualmente |
| STM32G474 | Cortex-M4 a 170 MHz, FPU, DSP, DMA e timer de alta resolução [7] | Atende à geração e é a melhor alternativa para simplificar o projeto |
| **STM32H743ZI** | **Cortex-M7 a 480 MHz, DMA, ampla memória, LTDC e DMA2D [8]** | **Escolhido pela maior margem para geração, interface e futuras funções** |

O STM32G474 provavelmente atenderia ao requisito básico, assim como o RP2040 é uma opção forte para a geração pura por seu PIO. A escolha do STM32H743ZI não indica incapacidade dessas alternativas: sua vantagem é a margem para manter simultaneamente o fluxo de amostras, a interface gráfica, o ajuste de parâmetros e futuras funcionalidades.

A implementação prevista parte de um temporizador acionando transferências das amostras por DMA para o GPIO ligado ao DAC0808. Os oito bits deverão, preferencialmente, ocupar uma única porta GPIO. Ainda será necessário verificar a rota entre temporizador, controlador DMA, barramento e porta escolhida, definir os pinos e validar experimentalmente os 5 MS/s. O fluxo pretendido é: **tabela de amostras → DMA → GPIO → DAC0808 → condicionamento analógico → saída**.

![Uploading dia<svg viewBox="0 0 700 560" xmlns="http://www.w3.org/2000/svg" role="img" aria-labelledby="titulo descricao" xmlns:c2pa="http://c2pa.org/manifest"><metadata><c2pa:manifest>AAAWgmp1bWIAAAAeanVtZGMycGEAEQAQgAAAqgA4m3EDYzJwYQAAABZcanVtYgAAAEdqdW1kYzJtYQARABCAAACqADibcQN1cm46YzJwYTpkMGFiMGJiZC04OGQzLTRlNzAtOGYzYS00YzdlNDczY2JkZDEAAAADl2p1bWIAAAApanVtZGMyYXMAEQAQgAAAqgA4m3EDYzJwYS5hc3NlcnRpb25zAAAAALxqdW1iAAAARGp1bWRjYm9yABEAEIAAAKoAOJtxE2MycGEuaW5ncmVkaWVudC52MwAAAAAYYzJzaNjz7U67xgtEIZ9M9tLW8FEAAABwY2JvcqNpZGM6Zm9ybWF0bWltYWdlL3N2Zyt4bWxqaW5zdGFuY2VJRHgseG1wOmlpZDoxMTNlODg5YS1lMGJhLTQxOGQtYjJlZS0xMmFhMGMyNjQ1ZDhscmVsYXRpb25zaGlwaHBhcmVudE9mAAAB4mp1bWIAAABBanVtZGNib3IAEQAQgAAAqgA4m3ETYzJwYS5hY3Rpb25zLnYyAAAAABhjMnNop+7tj07HacDMk4mvCnFakAAAAZljYm9yomdhY3Rpb25zgqJmYWN0aW9ua2MycGEub3BlbmVkanBhcmFtZXRlcnOha2luZ3JlZGllbnRzgaJjdXJseC1zZWxmI2p1bWJmPWMycGEuYXNzZXJ0aW9ucy9jMnBhLmluZ3JlZGllbnQudjNkaGFzaFggj7XhFzOX6XY1jHlghWTt5erXTAzywiIXW6JL1AnifMakZmFjdGlvbngdY29tLmFudGhyb3BpYy5jbGF1ZGUucHJvdmlkZWRqcGFyYW1ldGVyc6F4H2NvbS5hbnRocm9waWMub3JpZ2luLWNvbmZpZGVuY2VndW5rbm93bmtkZXNjcmlwdGlvbnhmQ2xhdWRlIHByb3ZpZGVkIHRoaXMgZmlsZSBhdCB0aGUgcmVxdWVzdCBvZiBhIHVzZXIgYW5kIG1heSBoYXZlIGNyZWF0ZWQgb3IgbW9kaWZpZWQgdGhlIGZpbGUgY29udGVudHMubXNvZnR3YXJlQWdlbnShZG5hbWVmQ2xhdWRlcmFsbEFjdGlvbnNJbmNsdWRlZPUAAADIanVtYgAAAEBqdW1kY2JvcgARABCAAACqADibcRNjMnBhLmhhc2guZGF0YQAAAAAYYzJzaJacRDmTXXrQiiY3FB2mcfEAAACAY2JvcqVjYWxnZnNoYTI1NmNwYWRNAAAAAAAAAAAAAAAAAGRoYXNoWCA7KchMffeXYzo39JDCCndw23pz/Y+j9aAEZRwn0bnHP2RuYW1lbmp1bWJmIG1hbmlmZXN0amV4Y2x1c2lvbnOBomVzdGFydBirZmxlbmd0aBkeBAAAAj5qdW1iAAAAJ2p1bWRjMmNsABEAEIAAAKoAOJtxA2MycGEuY2xhaW0udjIAAAACD2Nib3KlY2FsZ2ZzaGEyNTZpc2lnbmF0dXJleE1zZWxmI2p1bWJmPS9jMnBhL3VybjpjMnBhOmQwYWIwYmJkLTg4ZDMtNGU3MC04ZjNhLTRjN2U0NzNjYmRkMS9jMnBhLnNpZ25hdHVyZWppbnN0YW5jZUlEeCx4bXA6aWlkOmFlOWEwNDc4LTUyOGEtNDM3OC1iNDE2LTQ5MzY1YTEwMWJiM3JjcmVhdGVkX2Fzc2VydGlvbnODomN1cmx4LXNlbGYjanVtYmY9YzJwYS5hc3NlcnRpb25zL2MycGEuaW5ncmVkaWVudC52M2RoYXNoWCCPteEXM5fpdjWMeWCFZO3l6tdMDPLCIhdbokvUCeJ8xqJjdXJseCpzZWxmI2p1bWJmPWMycGEuYXNzZXJ0aW9ucy9jMnBhLmFjdGlvbnMudjJkaGFzaFgglE0EKipltBew06LqDeFckrT/GaTYY/hCuKQtHxjmlUyiY3VybHgpc2VsZiNqdW1iZj1jMnBhLmFzc2VydGlvbnMvYzJwYS5oYXNoLmRhdGFkaGFzaFggC5VUXxqgU+ligwZrfCUUtiq7HytEqhsYCglBYJMorDp0Y2xhaW1fZ2VuZXJhdG9yX2luZm+jZG5hbWVvQW50aHJvcGljIEZpbGVzZ3ZlcnNpb25lMS4wLjBrc3BlY1ZlcnNpb25lMi40LjAAABA4anVtYgAAAChqdW1kYzJjcwARABCAAACqADibcQNjMnBhLnNpZ25hdHVyZQAAABAIY2JvctKEWQISogEmGCFZAgowggIGMIIBjaADAgECAhRA5aAK7sI50L64g/oGQgU9Z1UTADAKBggqhkjOPQQDAzBJMRcwFQYDVQQKEw5BbnRocm9waWMsIFBCQzEuMCwGA1UEAxMlQW50aHJvcGljIENvbnRlbnQgQ3JlZGVudGlhbHMgUm9vdCBDQTAeFw0yNjA4MDcxODQzNTZaFw0yODA4MDYxOTQzNTZaMEQxFzAVBgNVBAoTDkFudGhyb3BpYywgUEJDMSkwJwYDVQQDEyBBbnRocm9waWMgQ2xhdWRlIENvbnRlbnQgU2lnbmluZzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABJh6CmvLUBgFFNU0vUKlOVtE6djd17L5SuwX0LemFisBM3dkd/3cyjxFA3Qo5S46fX0/ihY0VZ7mfb9KF703t5OjWDBWMA4GA1UdDwEB/wQEAwIHgDAVBgNVHSUEDjAMBgorBgEEAYPoXgIBMAwGA1UdEwEB/wQCMAAwHwYDVR0jBBgwFoAUzlHiBIFOZFsj+OPEz5o+nMHXXMIwCgYIKoZIzj0EAwMDZwAwZAIwMXMdFJ4BetLLVY7ORuE9noqbbAZOZn/aArXyTwFAZfKrPzxF2vPoJNf1+UCdg1XGAjBwX1zd9WGqYkqmL5SFqw1QySjr1zJfpJM9+1rdDwSPLMOPOjKuiXjoU/pUUeG9RwmhY3BhZFkNngAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAPZYQOWsAcHdNHuBy8yGmu/1vKFwc2p7IT7wTs/j+ZeHKEn4spJCrrFD6TxLAmfUBixVsGTKiu7rs0xGQJ5sTnrv9NQ=</c2pa:manifest></metadata>
<title id="titulo">Diagrama de blocos simplificado do gerador de funções</title>
<desc id="descricao">A alimentação parte da rede elétrica e passa por uma fonte que atende todo o sistema. O display se comunica com o sintetizador. A cadeia de sinal vai da base de tempo ao sintetizador, divide-se entre o conversor digital-analógico com filtro de reconstrução e o gerador de onda retangular, e as duas se reúnem na chave seletora antes do ganho, do offset e do estágio de saída.</desc>

<defs>
<marker id="seta" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
<path d="M2 1L8 5L2 9" fill="none" stroke="#4A5C59" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"/>
</marker>
</defs>

<rect width="700" height="560" fill="#FFFFFF"/>

<g font-family="Helvetica, Arial, sans-serif">

<!-- ===== ALIMENTAÇÃO ===== -->
<text x="24" y="36" font-size="12.5" fill="#2E6B41">Alimentação</text>
<rect x="24" y="46" width="192" height="146" rx="10" fill="none" stroke="#C3CDCB" stroke-width="1" stroke-dasharray="5 4"/>

<rect x="40" y="62" width="160" height="42" rx="6" fill="#FFFFFF" stroke="#8B2D57" stroke-width="1.4"/>
<text x="120" y="88" font-size="13" fill="#0F1E1C" text-anchor="middle">Rede elétrica</text>

<rect x="40" y="126" width="160" height="50" rx="6" fill="#FFFFFF" stroke="#2E6B41" stroke-width="1.4"/>
<text x="120" y="147" font-size="13" fill="#0F1E1C" text-anchor="middle">Fonte de</text>
<text x="120" y="164" font-size="13" fill="#0F1E1C" text-anchor="middle">alimentação</text>

<line x1="120" y1="104" x2="120" y2="122" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<text x="120" y="212" font-size="11.5" fill="#4A5C59" text-anchor="middle">Alimenta todos os</text>
<text x="120" y="228" font-size="11.5" fill="#4A5C59" text-anchor="middle">blocos do sistema</text>

<!-- ===== INTERFACE ===== -->
<text x="24" y="286" font-size="12.5" fill="#A05A00">Interface</text>
<rect x="24" y="296" width="192" height="86" rx="10" fill="none" stroke="#C3CDCB" stroke-width="1" stroke-dasharray="5 4"/>
<rect x="40" y="312" width="160" height="54" rx="6" fill="#FFFFFF" stroke="#A05A00" stroke-width="1.4"/>
<text x="120" y="345" font-size="13" fill="#0F1E1C" text-anchor="middle">Display</text>

<path d="M200 339 L220 339 L220 132 L236 132" fill="none" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

<!-- ===== CADEIA DE SINAL ===== -->
<text x="240" y="36" font-size="12.5" fill="#17505C">Cadeia de sinal</text>

<rect x="240" y="46" width="430" height="40" rx="6" fill="#FFFFFF" stroke="#17505C" stroke-width="1.4"/>
<text x="455" y="71" font-size="13" fill="#0F1E1C" text-anchor="middle">Base de tempo</text>

<rect x="240" y="106" width="430" height="50" rx="6" fill="#FFFFFF" stroke="#17505C" stroke-width="1.6"/>
<text x="455" y="137" font-size="13.5" font-weight="600" fill="#0F1E1C" text-anchor="middle">Sintetizador e unidade de controle</text>

<line x1="455" y1="86" x2="455" y2="102" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

<rect x="240" y="182" width="204" height="44" rx="6" fill="#FFFFFF" stroke="#A05A00" stroke-width="1.4"/>
<text x="342" y="209" font-size="13" fill="#0F1E1C" text-anchor="middle">Conversor D/A</text>

<rect x="466" y="182" width="204" height="44" rx="6" fill="#FFFFFF" stroke="#8B2D57" stroke-width="1.4"/>
<text x="568" y="202" font-size="13" fill="#0F1E1C" text-anchor="middle">Gerador de onda</text>
<text x="568" y="218" font-size="13" fill="#0F1E1C" text-anchor="middle">retangular</text>

<line x1="342" y1="156" x2="342" y2="178" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<line x1="568" y1="156" x2="568" y2="178" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

<rect x="240" y="250" width="204" height="44" rx="6" fill="#FFFFFF" stroke="#A05A00" stroke-width="1.4"/>
<text x="342" y="277" font-size="13" fill="#0F1E1C" text-anchor="middle">Filtro de reconstrução</text>

<line x1="342" y1="226" x2="342" y2="246" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

<rect x="240" y="318" width="430" height="44" rx="6" fill="#FFFFFF" stroke="#0F1E1C" stroke-width="1.6"/>
<text x="455" y="345" font-size="13.5" font-weight="600" fill="#0F1E1C" text-anchor="middle">Chave seletora</text>

<line x1="342" y1="294" x2="342" y2="314" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<path d="M568 226 L568 314" fill="none" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

<rect x="240" y="386" width="270" height="44" rx="6" fill="#FFFFFF" stroke="#A05A00" stroke-width="1.4"/>
<text x="375" y="413" font-size="13" fill="#0F1E1C" text-anchor="middle">Ganho e offset</text>

<rect x="240" y="454" width="270" height="44" rx="6" fill="#FFFFFF" stroke="#0F1E1C" stroke-width="1.4"/>
<text x="375" y="481" font-size="13" fill="#0F1E1C" text-anchor="middle">Estágio de saída</text>

<line x1="375" y1="362" x2="375" y2="382" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<line x1="375" y1="430" x2="375" y2="450" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<line x1="375" y1="498" x2="375" y2="522" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>
<text x="375" y="544" font-size="14" font-weight="600" fill="#0F1E1C" text-anchor="middle">Saída</text>

<rect x="534" y="386" width="136" height="44" rx="6" fill="#FFFFFF" stroke="#A05A00" stroke-width="1.4"/>
<text x="602" y="406" font-size="12.5" fill="#0F1E1C" text-anchor="middle">Controle de</text>
<text x="602" y="422" font-size="12.5" fill="#0F1E1C" text-anchor="middle">amplitude e offset</text>

<path d="M670 132 L686 132 L686 408 L674 408" fill="none" stroke="#4A5C59" stroke-width="1.3" stroke-dasharray="4 4" marker-end="url(#seta)"/>
<line x1="530" y1="408" x2="514" y2="408" stroke="#4A5C59" stroke-width="1.4" marker-end="url(#seta)"/>

</g>
</svg>
grama-blocos-simplificado.svg…]()


## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)

- [1] [Analog Devices — All About Direct Digital Synthesis](https://www.analog.com/en/resources/analog-dialogue/articles/all-about-direct-digital-synthesis.html)
- [2] [Analog Devices — Datasheet do AD5689/AD5687](https://www.analog.com/media/en/technical-documentation/data-sheets/AD5689_5687.pdf)
- [3] [Texas Instruments — Datasheet do DAC0808](https://www.ti.com/lit/ds/symlink/dac0808.pdf)
- [4] [Texas Instruments — Datasheet do MSP430FR5994](https://www.ti.com/lit/ds/symlink/msp430fr5994.pdf)
- [5] [Raspberry Pi — Datasheet do RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [6] [Espressif Systems — Datasheet do ESP32-S3](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [7] [STMicroelectronics — Datasheet do STM32G474](https://www.st.com/resource/en/datasheet/stm32g474ce.pdf)
- [8] [STMicroelectronics — Datasheet do STM32H743ZI](https://www.st.com/resource/en/datasheet/stm32h743zi.pdf)


