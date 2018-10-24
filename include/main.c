#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*aaaa*/
void leitura_struct(FILE *entrada, FILE *saida){
	int stack = 0;
	int old_stack = 0;
	char caractere;
	printf("%d %d %d\n",!feof(entrada), old_stack == stack, stack!=0);
	while(!feof(entrada)){
		fscanf(entrada,"%c",&caractere);
		fprintf(saida,"%c",caractere);
		switch(caractere){
			case '{':
				old_stack = stack;
				stack++;
				break;
			case '}':
				old_stack = stack;
				stack--;
				if(old_stack != stack && stack==0)
					return;
				break;
		}
	}
}

void leitura_funcoes(FILE *entrada, FILE *saida){
	int stack = 0, i=0;
	char palavra[7001];
	char caractere;
	while(!feof(entrada)){
		fscanf(entrada,"%c",&caractere);
		switch(caractere){
			case '{':
				if(stack==0){
					fprintf(saida,";");
				}
				stack++;
				break;
			case '}':
				stack--;
				break;
			default:
				i++;
				palavra[i] = caractere;
				if(caractere=='#' || caractere==' '){
					palavra[i] = '\0';
					if(strcmp(palavra,"struct") == 0 ||
					strcmp(palavra,"enum") == 0 ||
					strcmp(palavra,"union")== 0){
						fseek(entrada,-1,SEEK_CUR);
						leitura_struct(entrada,saida);
					}
					i=-1;
				}
				if(stack==0 && caractere!='#'){
					fprintf(saida,"%c",caractere);
				}else{
					fgets(palavra,7000,entrada);
					if(caractere=='#') fprintf(saida,"%c%s",caractere,palavra);
				}
		}
	}
}

int main(int argc, char **argv){
	FILE *entrada = fopen(argv[1],"r");
	FILE *saida = fopen(argv[2],"w");

	if (entrada == NULL || saida == NULL){
		printf("Algum erro foi detectado!\n");
		abort();
	}

	leitura_funcoes(entrada, saida);

	fclose(entrada);
	fclose(saida);
	return 0;
}
