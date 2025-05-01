#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define BAZA_SIZE 10
#define MAX_SIZE 20

FILE *fp;

char tmp_ime[MAX_SIZE];
char tmp_prezime[MAX_SIZE];
char tmp_brIndexa[MAX_SIZE];
int tmp_brBodova;

int main () {
	bool flag=true;
	while(flag) {	
		int opcija=0;
		do {
			printf("\nIZABERITE OPCIJU: \n");
			printf("1. Unos novog studenta \n");
			printf("2. Izmena broja bodova studenta \n");
			printf("3. Brisanje studenta \n");
			printf("4. Citanje liste \n");
			printf("5. Izlaz \n ");	
			printf(" Izabrali ste opciju: ");	
			scanf("%d",&opcija);
			if(opcija<0 || opcija>5)
				printf("Ne postoji opcija pod tim brojem! \n");
		} while(opcija<0 || opcija>5);

		switch(opcija) {
			case 1: 
				printf("Unesite ime: ");
				scanf("%s",tmp_ime);
				printf("Unesite prezime: ");
				scanf("%s",tmp_prezime);
				printf("Unesite broj indeksa: ");
				scanf("%s",tmp_brIndexa);
				printf("Unesite broj bodova: ");
				scanf("%d",&tmp_brBodova);
				printf("\n");
		
				fp = fopen ("/dev/baza", "w");
				if(fp==NULL) {
					puts("Problem pri otvaranju /dev/baza \n");
					return -1;
				}
		
				fprintf(fp,"%s,%s,%s=%d\n", tmp_ime, tmp_prezime, tmp_brIndexa, tmp_brBodova);

				if(fclose(fp)) {
					puts("Problem pri zatvaranju /dev/baza\n");
					return -1;
				}
				printf("\nUspesan unos!\n");
			break;
				
			case 2: 
				printf("Unesite ime: ");
				scanf("%s",tmp_ime);
				printf("Unesite prezime: ");
				scanf("%s",tmp_prezime);
				printf("Unesite broj indeksa: ");
				scanf("%s",tmp_brIndexa);
				printf("Unesite novi broj bodova: ");
				scanf("%d",&tmp_brBodova);
				printf("\n");
		
				fp = fopen ("/dev/baza", "w");
				if(fp==NULL) {
					puts("Problem pri otvaranju /dev/baza \n");
					return -1;
				}
		
				fprintf(fp,"%s,%s,%s=%d\n", tmp_ime, tmp_prezime, tmp_brIndexa, tmp_brBodova);

				if(fclose(fp)) {
					puts("Problem pri zatvaranju /dev/baza\n");
					return -1;
				}
				printf("\nUspesna izmena!\n");
			break;

			case 3:
				printf("Unesite ime: ");
				scanf("%s",tmp_ime);
				printf("Unesite prezime: ");
				scanf("%s",tmp_prezime);
				printf("Unesite broj indeksa: ");
				scanf("%s",tmp_brIndexa);
				printf("\n");
		
				fp = fopen ("/dev/baza", "w");
				if(fp==NULL) {
					puts("Problem pri otvaranju /dev/baza \n");
					return -1;
				}
		
				fprintf(fp,"izbrisi=%s,%s,%s\n", tmp_ime, tmp_prezime, tmp_brIndexa);

				if(fclose(fp)) {
					puts("Problem pri zatvaranju /dev/baza\n");
					return -1;
				}
				printf("\nUspesno brisanje!\n");
			break;

			case 4:    		
				fp = fopen ("/dev/baza", "r");
				if(fp==NULL) {
					puts("Problem pri otvaranju /dev/baza \n");
					return -1;
				}
				
				while(fscanf(fp, "%s %s %s - %d\n", tmp_brIndexa, tmp_ime, tmp_prezime, &tmp_brBodova) != EOF)
					printf("%s %s %s - %d\n", tmp_brIndexa, tmp_ime, tmp_prezime, tmp_brBodova);

				if(fclose(fp)) {
					puts("Problem pri zatvaranju /dev/baza\n");
					return -1;
				}
				printf("\n\nUspesno citanje!\n");
			break;	
			
			case 5: flag=false;
			break;	

			default:
			break;
		}
	}
}

