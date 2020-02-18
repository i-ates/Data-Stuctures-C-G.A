#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct dna {
    int dna;
    struct dna *next;
};
struct gen {
    int fitness;
    struct dna *dna_data;
    struct gen *next;
};

int bestChFitness;

void addDnaToGen(struct gen *head, int dna) {
    struct dna* current = (struct dna *)malloc(sizeof(struct dna));
    current->dna = dna;
    current->next = NULL;
    if (head->dna_data == NULL) {
        head->dna_data = current;
    } else {
        struct dna* node = head->dna_data;
        while (node->next != NULL) node = node->next;
        node->next = current;
    }
}

void addGen(struct gen **head, char genList[], int n) {
    if(*head == NULL) {
        *head = malloc(sizeof(struct gen));
        for (int i = 0; i < n; i++) {
            addDnaToGen(*head, atoi(&genList[2 * i]));
        }
        (*head)->next = NULL;
    } else {
        struct gen *temp = *head;
        while(temp->next) temp=temp->next;

        temp->next = malloc(sizeof(struct gen));
        for (int i = 0; i < n; i++) {
            addDnaToGen(temp->next,atoi(&genList[2 * i]));
        }
        temp->next->next = NULL;
    }
}

void Display(struct gen *head, int generation, char bestChromosome[]) {
    struct gen *current = head;
    printf("GENERATION: %d\n", generation);
    while(current != NULL) {
        struct dna *CurrentDna = current->dna_data;
        while(CurrentDna != NULL) {
            printf("%d",CurrentDna->dna);
            if (CurrentDna->next != NULL) printf(":");
            CurrentDna = CurrentDna->next;
        }
        printf(" -> %d\n", current->fitness);
        current=current->next;
    }
    printf("Best chromosome found so far: %s -> %d\n", bestChromosome, bestChFitness);
}

void sort(struct gen **head) {
    for(struct gen *temp1=*head;temp1!=NULL;temp1=temp1->next) {
        for(struct gen *temp2=temp1->next;temp2!=NULL;temp2=temp2->next) {
            if(temp2->fitness < temp1->fitness) {
                int a = temp1->fitness;
                struct dna *dnatemp = temp1->dna_data;
                temp1->fitness = temp2->fitness;
                temp1->dna_data = temp2->dna_data;
                temp2->fitness = a;
                temp2->dna_data = dnatemp;
            }
        }
    }
}

void calculateFitness(struct gen *head,int n, int generation, char bestChromosome[]){
    struct gen *current = head;
    while(current != NULL) {
        current->fitness = 0;
        struct dna *CurrentDna = current->dna_data;

        for (int i = n-1; i >= 0; i--) {
            if (CurrentDna->dna == 1) {
                current->fitness += pow(2, i);
            }
            CurrentDna = CurrentDna->next;
        }

        if (bestChFitness == 0 || current->fitness < bestChFitness) {
            char tempStr[2*n] ;
            int i = 0;
            for(struct dna *CurrentDna2 = current->dna_data;CurrentDna2!=NULL;CurrentDna2=CurrentDna2->next) {
                if (CurrentDna2->dna) tempStr[i++] = '1';
                else tempStr[i++] = '0';
                if (CurrentDna2->next != NULL) tempStr[i++] = ':';
            }
            strcpy(bestChromosome,tempStr);
            bestChFitness = current->fitness;
        }
        current=current->next;
    }
    sort(&head);
    Display(head, generation, bestChromosome);
}

void doMutate(struct gen *head, int mutateIndex){
    for(struct gen *current = head;current!=NULL;current=current->next) {
        int i = 1;
        for(struct dna *CurrentDna = current->dna_data;CurrentDna!=NULL;CurrentDna=CurrentDna->next){
            if (i++ == mutateIndex) {
                if (CurrentDna->dna) CurrentDna->dna = 0;
                else CurrentDna->dna = 1;
            }
        }
    }
}

void doXover(struct gen **head, int genIndex1, int genIndex2, int dnaIndex1, int dnaIndex2) {
    int curr1Index = 1;
    for(struct gen *temp1=*head;temp1!=NULL;temp1=temp1->next) {
        int curr2Index = 1;
        for(struct gen *temp2=*head;temp2!=NULL;temp2=temp2->next) {
            if (curr1Index == genIndex1 && curr2Index++ == genIndex2){
                int curr1DnaIndex = 1;
                for(struct dna *temp3=temp1->dna_data;temp3!=NULL;temp3=temp3->next){
                    int curr2DnaIndex = 1;
                    for(struct dna *temp4=temp2->dna_data;temp4!=NULL;temp4=temp4->next){
                        for (int i = dnaIndex1; i <= dnaIndex2; i++) {
                            if (curr1DnaIndex == i && curr2DnaIndex++ == i){
                                int temp = temp3->dna;
                                temp3->dna = temp4->dna;
                                temp4->dna = temp;
                            }
                        }
                    }
                    curr1DnaIndex++;
                }
            }
        }
        curr1Index++;
    }
}

void evolution(struct gen *head, char selectionLine[], char xoverLine[], int pop_size){
    char *xoverptr = strtok(xoverLine, ":");
    char *xoverptr2 = strtok(NULL, ":");

    char *array[pop_size];
    int i = 0;
    char *selectionptr = strtok(selectionLine, " ");

    while(selectionptr != NULL) {
        array[i++] = selectionptr;
        selectionptr = strtok (NULL, " ");
    }

    for (int j = 0; j < pop_size/2; j++) {
        selectionptr = array[j];
        char *sptr =  strtok(selectionptr, ":");
        char *sptr2 = strtok(NULL, ":");
        doXover(&head,atoi(sptr),atoi(sptr2),atoi(xoverptr),atoi(xoverptr2));
    }
}

void readFiles(struct gen **head, int dna_size, int pop_size, char bestChromosome[]){
    FILE *populationFile = fopen("population", "r");
    FILE *selectionFile = fopen("selection", "r");
    FILE *xoverFile = fopen("xover", "r");
    FILE *mutateFile = fopen("mutate", "r");
    char selectionLine[dna_size*2+1];
    char xoverLine[dna_size*2+1];
    char mutateLine[dna_size*2+1];
    char populationLine[dna_size*2+1];
    int generation = 1;

    if (selectionFile == NULL || xoverFile == NULL || mutateFile == NULL ||populationFile == NULL) printf("Some files can't open");
    else {
        while (fgets(populationLine,dna_size*2+1, populationFile) != NULL) {
            addGen(head,populationLine,dna_size);
        }
        calculateFitness(*head, dna_size, 0,bestChromosome);

        while (fgets(selectionLine, dna_size*2+1, selectionFile) != NULL) {
            fgets(xoverLine, dna_size*2+1, xoverFile);
            fgets(mutateLine, dna_size*2+1, mutateFile);
            evolution(*head,selectionLine,xoverLine,pop_size);
            doMutate(*head, atoi(mutateLine));
            calculateFitness(*head, dna_size, generation++, bestChromosome);
        }
    }

    fclose(selectionFile);
    fclose(xoverFile);
    fclose(mutateFile);
    fclose(populationFile);
}

void freeList(struct gen* head) {
    struct gen *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        struct dna *CurrentDna = temp->dna_data;
        struct dna *tempDna;
        while(CurrentDna != NULL){
            tempDna = CurrentDna;
            CurrentDna = CurrentDna->next;
            free(tempDna);
        }
        free(temp);
    }
}

int main(int argc,char* argv[]) {
    int dna_size = atoi(argv[1]);
    int pop_size = atoi(argv[2]);
    int generation_size = atoi(argv[3]);
    char bestChromosome[dna_size*2];

    struct gen* List = NULL;
    readFiles(&List,dna_size,pop_size,bestChromosome);
    freeList(List);
}