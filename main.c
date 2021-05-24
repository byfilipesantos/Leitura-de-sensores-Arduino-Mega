#include <avr/io.h>
#include <util/delay.h>

/* Declara vetor de conversão de valores. */
const unsigned char conv[10] = {
	0x3F,	/* 0 */
	0x06,	/* 1 */
	0x5B,	/* 2 */
	0x4F,	/* 3 */
	0x66,	/* 4 */
	0x6D,	/* 5 */
	0x7D,	/* 6 */
	0x07,	/* 7 */
	0x7F,	/* 8 */
	0x6F	/* 9 */
};

int luminosidade()
{
	unsigned short leitura = 0;
	// Seleciona o canal 8 (MUX5:0 = 100000).
	ADMUX = ADMUX & 0xE0;
	ADCSRB |= 1 << 3;
	ADCSRA |= (1 << 6); // Inicia uma nova conversão.

	// Aguarda a conversão terminar (ADIF == 0).
	while ((ADCSRA & (1 << 4)) == 0);
	leitura = ADCL;
	leitura |= ADCH << 8;
	leitura = leitura * 0.005;
	return leitura;
}

int temperatura()
{
	unsigned short leitura = 0;
	// Seleciona o canal 9 (MUX5:0 = 100001).
	ADMUX = (ADMUX & 0xE0) | 0x01;
	ADCSRB |= 1 << 3;
	ADCSRA |= (1 << 6); // Inicia uma nova conversão.

	// Aguarda a conversão terminar (ADIF == 0).
	while ((ADCSRA & (1 << 4)) == 0);
	leitura = ADCL;
	leitura |= ADCH << 8;
	leitura = leitura * ((5.0 / 1023) * 100);
	return leitura;
}

int main(void)
{
	int modo = 0;	// Variável de controle da tela
	char disp = 0;
	int value = 0;
	int tempoB1 = 0;
	int tempoB2 = 0;
	int lm35 = 0;	
	int ldr = 0;	
	int b1 = 0;		// Alterna entre as telas dos sensores no modo operação 
	int b2 = 0;		// Troca de modo (operação e configuração)
	int b3 = 0;		// Incrementa
	int b4 = 0;		// Decrementa
	int auxBotao1 = 0;	// Variável auxiliar botão 1
	int auxBotao2 = 0;	// Variável auxiliar botão 2
	int auxBotao3 = 0;	// Variável auxiliar botão 3
	int auxBotao4 = 0;	// Variável auxiliar botão 4
	int configuracao = 0;
	int operacao = 0;
	int amostragemLdr = 0;
	int amostragemLm35 = 0;
	int alarmeGlobal = 0;
	int alarmeLdr = 0;
	int alarmeLm35 = 0;
	int minLdr = 0;
	int maxLdr = 0;
	int minLm35 = 0;
	int maxLm35 = 0;
	int histereseLdr = 0;
	int histereseLm35 = 0;

	ADCSRA |= (1 << 7); // Liga o conversor.
	ADMUX |= (1 << 6); // Configura a referência de tensão.

	DDRF = 0xFF;
	DDRG = 0x03;
	DDRH = 0x07;
	DDRJ = 0x00;

	while (1) {
		b1 = PINJ & 0x01; // Leitura botão 1
		b2 = PINJ & 0x02; // Leitura botão 2
		b3 = PINJ & 0x04; // Leitura botão 3
		b4 = PINJ & 0x08; // Leitura botão 4
		lm35 = temperatura();	// Leitura do sensor de temperatura
		ldr = luminosidade();	// Leitura do sensor de luminosidade

		if (b2)
		{
			tempoB2++;
			if (tempoB2 > 200)
			{
				if (modo == 0)
				{
					PORTH &= ~0x00;
					modo = 1; // Configuração
					configuracao = 0;
					PORTH |= 0x04;
				}
				else
				{
					PORTH &= ~0x04;
					modo = 0; // Operação
					operacao = 0;
					PORTH |= 0x00;
				}
				tempoB2 = 0;
			}
		}
		else
		{
			tempoB2 = 0;
		}
		
		if (b2 == 0 && auxBotao2 > 0)
		{
			if (operacao == 0 || configuracao == 0)
			{
				operacao++;
				configuracao++;
			}
			else if (operacao == 1 || configuracao == 1)
			{
				operacao++;
				configuracao++;
			}
			else // tela
			{
				operacao = 0;
				configuracao = 0;
			}
		}
		auxBotao2 = b2;


		if (b1)
		{
			tempoB1++;
			if (tempoB1 > 10 && modo == 0)
			{
				if (operacao == 0)
				{
					operacao = 1;
				}
				else
				{
					operacao = 0;
				}
				tempoB1 = 0;
			} else if (tempoB1 > 10 && modo == 1){
				if (operacao == 0)
				{
					configuracao = 1;
				}
				else
				{
					configuracao = 0;
				}
				tempoB1 = 0;
			}
		}
		else
		{
			tempoB1 = 0;
		}

		if (modo == 0)	//Modo de monitoramento
		{
			if (operacao == 0)
			{
				PORTH &= ~0x02;
				value = lm35;
				PORTH |= 0x01;
			}
			else if (operacao == 1)
			{
				PORTH &= ~0x01;
				value = ldr;
				PORTH |= 0x02;
			}
		}
		else	// Modo de configuração
		{
			if (configuracao == 1) // Configuração do intervalo de amostragem do sensor de luminosidade LDR.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					amostragemLdr++;
				}
				if (b4 == 0 && auxBotao4 > 0 && amostragemLdr > 1)
				{
					amostragemLdr--;
				}
				value = amostragemLdr;
			}
			else if (configuracao == 2) // Configuração do intervalo de amostragem do sensor de temperatura LM35.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					amostragemLm35++;
				}
				if (b4 == 0 && auxBotao4 > 0 && amostragemLm35 > 1)
				{
					amostragemLm35--;
				}
				value = amostragemLm35;
			}
			else if (configuracao == 3) // Habilita/desabilita alarmes globais
			{
				if (b3 == 0 && auxBotao3 > 0 && alarmeGlobal == 0)
				{
					alarmeGlobal++;
				}
				if (b4 == 0 && auxBotao4 > 0 && alarmeGlobal == 1)
				{
					alarmeGlobal--;
				}
				value = alarmeGlobal;
			}
			else if (configuracao == 4) // Habilita/desabilita alarme de luminosidade do sensor LDR.
			{
				if (b3 == 0 && auxBotao3 > 0 && alarmeLdr == 0)
				{
					alarmeLdr++;
				}
				if (b4 == 0 && auxBotao4 > 0 && alarmeLdr == 1)
				{
					alarmeLdr--;
				}
				value = alarmeLdr;
			}
			else if (configuracao == 5) // Habilita/desabilita alarme de temperatura do sensor LM35.
			{
				if (b3 == 0 && auxBotao3 > 0 && alarmeLm35 == 0)
				{
					alarmeLm35++;
				}
				if (b4 == 0 && auxBotao4 > 0 && alarmeLm35 == 1)
				{
					alarmeLm35--;
				}
				value = alarmeLm35;
			}
			else if (configuracao == 6) // Valor mínimo da faixa de trabalho do sensor de luminosidade LDR.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					minLdr++;
				}
				if (b4 == 0 && auxBotao4 > 0 && minLdr > 1)
				{
					minLdr--;
				}
				value = minLdr;
			}
			else if (configuracao == 7) // Valor máximo da faixa de trabalho do sensor de luminosidade LDR.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					maxLdr++;
				}
				if (b4 == 0 && auxBotao4 > 0 && maxLdr > 1)
				{
					maxLdr--;
				}
				value = maxLdr;
			}
			else if (configuracao == 8) // Valor mínimo da faixa de trabalho do sensor de temperatura LM35.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					minLm35++;
				}
				if (b4 == 0 && auxBotao4 > 0 && minLm35 > 1)
				{
					minLm35--;
				}
				value = minLm35;
			}
			else if (configuracao == 9) // Valor máximo da faixa de trabalho do sensor de temperatura LM35.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					maxLm35++;
				}
				if (b4 == 0 && auxBotao4 > 0 && maxLm35 > 1)
				{
					maxLm35--;
				}
				value = maxLm35;
			}
			else if (configuracao == 10) // Histerese na leitura do sensor de luminosidade LDR.
			{
				if (b3 == 0 && histereseLdr > 0)
				{
					histereseLdr++;
				}
				if (b4 == 0 && auxBotao4 > 0 && histereseLdr > 1)
				{
					histereseLdr--;
				}
				value = histereseLdr;
			}
			else if (configuracao == 11) // Histerese na leitura do sensor de temperatura LM35.
			{
				if (b3 == 0 && auxBotao3 > 0)
				{
					histereseLm35++;
				}
				if (b4 == 0 && auxBotao4 > 0 && histereseLm35 > 1)
				{
					histereseLm35--;
				}
				value = histereseLm35;
			}
			auxBotao3 = b3;
			auxBotao4 = b4;		
		}

		/* Lógica de escrita nos mostradores. */
		if (disp == 0) {
			value = conv[value / 10];
		} else {
			value = conv[value % 10];
		}

		/* Seleciona o mostrador e exibe o valor. */
		PORTG = ~(1 << disp);
		PORTF = value;

		/* Reinicia seletor do mostrador. */
		if (++disp == 2)
			disp = 0;

		_delay_ms(10);
	}
	return 0;
}
