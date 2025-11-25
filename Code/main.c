#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct user {
    char username[50];
    unsigned long passwordHash;
};

struct budgetRecord{
    char date[30]; // DD-MM-YYYY e.g: 27-11-2025
    float amount; 
    char type[20]; // is it income or expense
};

void homeMenu(struct user users[], int *usercount);
void dashboardMenu(char *username);

// home menu options
void registration(struct user users[], int *usercount);
void login(struct user users[], int usercount);

// dashboard menu options
void viewRecord(char *username);
void addRecord(char *username);
void deleteRecord(char *username);
void editRecord(char *username);
void sentAlert(char *username);
void sentMonthlyReport(char *username);

// utility functions
void loadCredentials(struct user users[], int *usercount);
void saveCredentialToFile(struct user u);
void getUserRecordFile(char *username, char *filepath);

void readRecords(FILE *fptr,int index);
void readExpenseRecords(FILE *fptr, float *totalExpense);
void readMonthlyBudget(FILE *fptr, char *monthNum, float *totalExpense, float *totalIncome);
char* returnMonth(char* monthNum);

// security algorithms
unsigned long hashString(const char *str);
float encryptAmount(float amount);
float decryptAmount(float encrypted);

void exitMenu();
void invalidChoice();

int main() {

    struct user users[100];
    int usercount = 0;

    loadCredentials(users, &usercount);

    homeMenu(users,&usercount);

    return 0;
}

void homeMenu(struct user users[], int *usercount) {
    int choice;

    do {
        printf("\n\tHOME MENU\t\n");
        printf("0. Exit\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 0:exitMenu();break;
            case 1:login(users, *usercount);break;
            case 2:registration(users, usercount);break;
            default:invalidChoice(); break;
        }
    } while(choice != 0);
    
}

void loadCredentials(struct user users[], int *usercount) {

    FILE *fptr = fopen("credentials/cred.txt", "r");
    if (fptr == NULL) {
        fptr = fopen("credentials/cred.txt", "w");
        fclose(fptr);
        return;
    }

    while (fscanf(fptr, "%s %lu", 
                  users[*usercount].username, 
                  &users[*usercount].passwordHash) != EOF) 
    {
        (*usercount)++;
    }

    fclose(fptr);
}

void saveCredentialToFile(struct user u) {

    FILE *fp = fopen("credentials/cred.txt", "a");
    if (fp == NULL) {
        printf("Error opening credentials file!\n");
        return;
    }

    fprintf(fp, "%s %lu\n", u.username, u.passwordHash);

    fclose(fp);
}

void registration(struct user users[], int *usercount) {

    printf("\n\tREGISTRATION\t\n");

    int duplicate;
    do {
        duplicate = 0;
        printf("Enter username: ");
        scanf("%s", users[*usercount].username);

        // Check username duplicate
        for(int i = 0; i < *usercount; i++) {
            if(strcmp(users[i].username, users[*usercount].username) == 0) {
                printf("Username already exists! Try another.\n");
                duplicate = 1;
                break;
            }
        }
    } while(duplicate == 1);

    // Password input + validation
    char password[50];
    int length;
    do {
        printf("Enter password (min 4 alpha numeric characters): ");
        scanf("%s", password);
        length = strlen(password);

        if (length < 4) {
            printf("Password too short! Try again.\n");
        }
    } while(length < 4);

    unsigned long hashed = hashString(password);
    users[*usercount].passwordHash = hashed;

    saveCredentialToFile(users[*usercount]);

    printf("Registration successful!\n");
    (*usercount)++;
}

void login(struct user users[], int usercount) {
    int attemptsMade = 0;
    int totalAttempts = 3;
    int found = 0;
    int wantToRegister =0;
    do{
        printf("\n\tLOGIN\t\n");

        char uname[50], pword[50];

        printf("Enter username: ");
        scanf("%s", uname);

        printf("Enter password: ");
        scanf("%s", pword);

        unsigned long hashed = hashString(pword);

        for (int i = 0; i < usercount; i++) {

            if(strcmp(users[i].username, uname) == 0 &&
                users[i].passwordHash == hashed) 
            {
                printf("Login successful!\n");
                found = 1;
                dashboardMenu(uname); 
                break;
            }
        }

        if (found==0) {
            attemptsMade++;

            printf("Invalid username or password.\n\n");
            printf("You have %d attempts out of %d \n\n\n", totalAttempts-attemptsMade, totalAttempts);

            if (attemptsMade <  totalAttempts) {
                printf("Do you want to register? (1 - Yes, 0 - No): ");
                scanf("%d", &wantToRegister);
                if (wantToRegister==1) {
                    registration(users, &usercount);
                }
            } else {
                printf("You fields are locked. Wait for 10 seconds\n");
                sleep(10); 

            }
        }
        
    } while (found==0 && attemptsMade < totalAttempts);
}

unsigned long hashString(const char *str) {
    unsigned long hash = 5381;   
    int c;
    while (*str != '\0') {
        c= *str;
        hash = ((hash << 5) + hash) + c;  // (hash * 33) + 112
        str++;                     
    }
    return hash;
}

void dashboardMenu(char *username) {
    int choice;

    do {
        printf("\n\tDASHBOARD (%s)\t\n", username);
        printf("1. View Records\n");
        printf("2. Add Record\n");
        printf("3. Edit Record\n");
        printf("4. Delete Record\n");
        printf("5. Send Monthly Report\n");
        printf("6. Send Alert\n");
        printf("0. Exit\n");

        printf("Enter choice(0-6): ");

        scanf("%d", &choice);


        switch(choice) {
            case 0: exitMenu();break;
            case 1: viewRecord(username); break;
            case 2: addRecord(username); break;
            case 3: editRecord(username); break;
            case 4: deleteRecord(username); break;
            case 5: sentMonthlyReport(username); break;
            case 6: sentAlert(username); break;
            default: invalidChoice();break;
          
        }

    } while(choice != 0);
}


void exitMenu() {
    printf("\n\tExited\n");
    exit(1);
    
}

void invalidChoice() {
    printf("Invalid choice! Try again.\n");
}

void getUserRecordFile(char *username, char *filepath) {
    sprintf(filepath, "records/%s.txt", username);
}

void addRecord(char *username) {
    struct budgetRecord p;
    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "a");
    if (!fp) {
        printf("Error opening records file!\n");
        return;
    }


    printf("Enter date (dd-mm-yyyy): ");
    scanf("%s", p.date);

    printf("Enter amount: ");
    scanf("%f", &p.amount);

    printf("Enter type (income or expense): ");
    getchar();  

    fgets(p.type, sizeof(p.type), stdin);
    p.type[strcspn(p.type, "\n")] = 0;

    float encryptedAmount = encryptAmount(p.amount);
    fprintf(fp, "%s %.2f %s\n", p.date, encryptedAmount, p.type);


    //fprintf(fp, "%s %.2f %s\n", p.date, p.amount, p.type);

    printf("Record added!\n");
    fclose(fp);
}

void viewRecord(char *username) {
    int i=1;
    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("\nNo records found!\n");
        return;
    }

    printf("\n\tYour Records \t\n");

    readRecords(fp, i);

    fclose(fp);
}


void editRecord(char *username) {
    struct budgetRecord arr[500];
    int count = 0, recordSno, found = 0;

    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records to edit!\n");
        return;
    }

    while (fscanf(fp, "%s %f %[^\n]",
                  arr[count].date,
                  &arr[count].amount,
                  arr[count].type) != EOF) 
    {
        arr[count].amount = decryptAmount(arr[count].amount);
        count++;
    }
    fclose(fp);

    viewRecord(username);
    printf("Enter S.no to edit that specific record or 0 to go back: ");
    scanf("%d", &recordSno);

    if (recordSno == 0) {
        return;
    } else {
        if (recordSno < 1 || recordSno > count) {
            printf("Invalid S.no selected\n");
            return;
        }

        int i = recordSno - 1;

        printf("Enter new date: ");
        scanf("%s", arr[i].date);

        printf("Enter new amount: ");
        scanf("%f", &arr[i].amount);

        printf("Enter new type (income or expense): ");
        getchar();
        fgets(arr[i].type, sizeof(arr[i].type), stdin);
        arr[i].type[strcspn(arr[i].type, "\n")] = 0;


        fp = fopen(filepath, "w");
        for (int j = 0; j < count; j++) {

            float encrypted = encryptAmount(arr[j].amount);

            fprintf(fp, "%s %.2f %s\n",
                    arr[j].date, encrypted, arr[j].type);
        }

        fclose(fp);
        printf("Record updated!\n");
    } 
}

void deleteRecord(char *username) {
    struct budgetRecord arr[500];
    int count = 0, recordSno, found = 0;

    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records to delete!\n");
        return;
    }

    while (fscanf(fp, "%s %f %[^\n]",
                  arr[count].date,
                  &arr[count].amount,
                  arr[count].type) != EOF) 
    {
        arr[count].amount = decryptAmount(arr[count].amount);
        count++;
    }
    fclose(fp);

    viewRecord(username);

    printf("Enter S.no to delete that record or 0 to go back: ");
    scanf("%d", &recordSno);

    if (recordSno==0) {
        return;
    } else {
        
        if (recordSno < 1 || recordSno > count) {
            printf("Invalid S.no selected\n");
            return;
        }

        fp = fopen(filepath, "w");

        for (int i = 0; i < count; i++) {

            if (i == recordSno - 1) {
                found = 1;
                continue;  
            }

            float encrypted = encryptAmount(arr[i].amount);

            fprintf(fp, "%s %.2f %s\n",
                    arr[i].date, encrypted, arr[i].type);
        }

        fclose(fp);

        if (found)
            printf("Record deleted!\n");
        else
            printf("Record not found!\n");
    }

}


void sentMonthlyReport(char *username) {
    char filepath[100];
    sprintf(filepath, "records/%s.txt", username);

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("No records found!\n");
        return;
    }

    char monthNum[3];
    printf("Enter month (e.g., 08 for August): ");
    scanf("%s", monthNum);

    char month[30] = "";

    strcat(month, returnMonth(monthNum));

    if (strcmp(month, "Invalid") == 0) {
        printf("Invalid month selected!\n");
        fclose(fp);
        return;
    } else {
        if (strcmp(month, "Back")==0){
            fclose(fp);
            return;
        }
        struct budgetRecord p;
        float totalExpense = 0, totalIncome = 0;

        readMonthlyBudget(fp, monthNum, &totalExpense, &totalIncome);

        fclose(fp);

        printf("\n\t Monthly Report for %s \t\n", month);
        printf("Total Income  : %.2f\n", totalIncome);
        printf("Total Expense : %.2f\n", totalExpense);
        printf("Net Balance   : %.2f\n", totalIncome - totalExpense);
    }

   

}


void sentAlert(char *username) {

    char filepath[100];
    sprintf(filepath, "records/%s.txt", username);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records found!\n");
        return;
    }

    float limit;
    printf("Enter expense limit for alert: ");
    scanf("%f", &limit);

    float totalExpense = 0;

    readExpenseRecords(fp, &totalExpense);

    fclose(fp);

    if (totalExpense > limit)
        printf("\nALERT: Your total expense of %.2f exceeded the limit of %.2f\n", totalExpense, limit);
    else
        printf("Total expense of %.2f is within the limit of %.2f\n", totalExpense, limit);
}

float encryptAmount(float amount) {
    return (amount *4) + 147.59;   
}

float decryptAmount(float encrypted) {
    return (encrypted-147.59) / 4;  
}

void readRecords(FILE *fptr,int index) {

    struct budgetRecord p;

    if (fscanf(fptr, "%s %f %[^\n]", &p.date, &p.amount, p.type) == EOF){
        return;
    }

    float amount = decryptAmount(p.amount);


    printf("S.no: %d | Date: %s | Amount: %.2f | type: %s \n",
            index, p.date, amount, p.type); 

    readRecords(fptr, index+1);
}

void readExpenseRecords(FILE *fptr, float *totalExpense) {
    struct budgetRecord budget;
    if (fscanf(fptr, "%s %f %s", budget.date, &budget.amount, budget.type) == EOF){
        return;
    }
    if (strcmp(budget.type, "expense") == 0) {
        budget.amount = decryptAmount(budget.amount);
        *totalExpense += budget.amount;
    }

    readExpenseRecords(fptr, totalExpense);


}

void readMonthlyBudget(FILE *fptr, char *monthNum, float *totalExpense, float *totalIncome) {
    struct budgetRecord budget;

    if (fscanf(fptr, "%s %f %s", budget.date, &budget.amount, budget.type) == EOF) {
        return;
    } 
    budget.amount = decryptAmount(budget.amount);
    if (strncmp(budget.date+3, monthNum,2)==0){
        if (strcmp(budget.type, "expense")==0) {
            *totalExpense += budget.amount;
        } else {
            *totalIncome += budget.amount;
        }
    }

    readMonthlyBudget(fptr, monthNum, totalExpense, totalIncome);
}

char* returnMonth(char* monthNum) {
    if (strcmp(monthNum, "00") == 0) return "Back";
    else if (strcmp(monthNum, "01") == 0) return "January";
    else if (strcmp(monthNum, "02") == 0) return "February";
    else if (strcmp(monthNum, "03") == 0) return "March";
    else if (strcmp(monthNum, "04") == 0) return "April";
    else if (strcmp(monthNum, "05") == 0) return "May";
    else if (strcmp(monthNum, "06") == 0) return "June";
    else if (strcmp(monthNum, "07") == 0) return "July";
    else if (strcmp(monthNum, "08") == 0) return "August";
    else if (strcmp(monthNum, "10") == 0) return "October";
    else if (strcmp(monthNum, "11") == 0) return "November";
    else if (strcmp(monthNum, "12") == 0) return "December";
    else return "Invalid";

}