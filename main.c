#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

char method[3][25] = {
        "A thread per matrix",
        "A thread per row",
        "A thread per element"
};

char per_method[3][25] = {
        "_per_matrix.txt",
        "_per_row.txt",
        "_per_element.txt"
};

char *files[5];


struct timeval start;

struct timeval end;

struct parameters {
    int i;
    int j;
};

int meth_num = 0;

int file_num = 2;

int Mat1_row;

int Mat2_col;

int Mat2_row;

int Mat1_col;

int **mat1;

int **mat2;

int **matRes;

int counter;

void Method1();

void *Method2();

void *Method3();

void Thread_Method1();

void Thread_Method2();

void Thread_Method3();

void saveRes();

void clearMat();


void readMat();

void doMethod() {
    //checking if multiplication is possible
    if (Mat1_col != Mat2_row) {
        printf("Matrices can't be multiplied !!");
        exit(-1);
    }
    //calling the three methods
    Thread_Method1();
    Thread_Method2();
    Thread_Method3();
}

void Thread_Method1() {
    //calculate time and call the desired method then save result
    gettimeofday(&start, NULL);
    Method1();
    gettimeofday(&end, NULL);
    saveRes();
}

void Method1() {
    //matrix multiplication
    counter = 1;
    for (int i = 0; i < Mat1_row; i++) {
        for (int j = 0; j < Mat2_col; j++) {
            int temp = 0;
            for (int k = 0; k < Mat1_col; k++) {
                temp += mat1[i][k] * mat2[k][j];
            }
            matRes[i][j] = temp;
        }
    }
}


void Thread_Method2() {
    gettimeofday(&start, NULL);
    //thread per row
    pthread_t t[Mat1_row];
    for (long i = 0; i < Mat1_row; i++) {
        int rc = pthread_create(&t[i], NULL, &Method2, (void *) i);
        //checks the creation of thread if success
        if (rc) {
            printf("failed to create thread\n");
            exit(-1);
        }
        counter++;
    }
    //joining the threads
    for (int i = 0; i < Mat1_row; i++) {
        pthread_join(t[i], NULL);
    }
    gettimeofday(&end, NULL);
    saveRes();
}

void *Method2(void *msg) {
    long i;
    i = (long) msg;
    for (int j = 0; j < Mat2_col; j++) {
        int temp = 0;
        for (int k = 0; k < Mat1_col; k++) {
            temp += mat1[i][k] * mat2[k][j];
        }
        matRes[i][j] = temp;

    }
    //kill thread
    pthread_exit(NULL);
}

void Thread_Method3() {
    gettimeofday(&start, NULL);
    //thread per element method
    pthread_t t3[Mat1_row][Mat2_col];
    for (int i = 0; i < Mat1_row; ++i) {
        for (int j = 0; j < Mat2_col; ++j) {
            //struct to save and send method args
            struct parameters *args = malloc(sizeof(struct parameters));
            args->i = i;
            args->j = j;
            int rc = pthread_create(&t3[i][j], NULL, &Method3, (void *) args);
            if (rc) {
                printf("failed to create thread\n");
                exit(-1);
            }
            counter++;
        }

    }

    for (int i = 0; i < Mat1_row; i++) {
        for (int j = 0; j < Mat2_col; j++) {
            pthread_join(t3[i][j], NULL);
        }

    }
    gettimeofday(&end, NULL);
    saveRes();
}

void *Method3(void *msg) {
    struct parameters *args = (struct parameters *) msg;
    int i = args->i;
    int j = args->j;
    int temp = 0;
    //specific row multiplication by specific col
    for (int k = 0; k < Mat1_col; k++) {
        temp += mat1[i][k] * mat2[k][j];
    }
    matRes[i][j] = temp;
    //free memory allocation of the struct
    free(args);
    //kill thread
    pthread_exit(NULL);
}


void saveRes() {
    FILE *op = fopen(files[file_num++], "w");
    fprintf(op, "Method: %s\n\n", method[meth_num++]);
    for (int i = 0; i < Mat1_row; i++) {
        for (int j = 0; j < Mat2_col; j++) {
            fprintf(op, "%d ", matRes[i][j]);
        }
        fprintf(op, "\n");
    }

    printf("Method: %s\n\n", method[meth_num - 1]);
    printf("Time In micro secs: %lu\n", end.tv_usec - start.tv_usec);
    printf("Number of Threads: %d\n\n\n", counter);
    fclose(op);
    counter = 0;
    clearMat();
}

void clearMat() {
    for (int i = 0; i < Mat1_row; i++) {
        for (int j = 0; j < Mat2_col; j++) {
            matRes[i][j] = 0;
        }
    }
}

void alloc_mat1() {
    //opens the file where matrix1 and allocate memory space for it then read it
    FILE *fp1;
    fp1 = fopen(files[0], "r");
    //checking if the file not exist
    if (fp1 == NULL) {
        printf("File not found %s\n", files[0]);
        exit(-1);
    }

    fscanf(fp1, "row=%d col=%d", &Mat1_row, &Mat1_col);

    mat1 = (int **) malloc(Mat1_row * sizeof(int *));
    for (int i = 0; i < Mat1_row; i++)
        mat1[i] = (int *) malloc(Mat1_col * sizeof(int *));
    readMat(fp1, mat1, Mat1_row, Mat1_col);
    fclose(fp1);

}

void alloc_mat2() {
    //opens the file where matrix2 and allocate memory space for it then read it
    FILE *fp2;

    fp2 = fopen(files[1], "r");
    if (fp2 == NULL) {
        printf("File not found %s\n", files[1]);
        exit(-1);
    }
    //get the value of rows and cols
    fscanf(fp2, "row=%d col=%d", &Mat2_row, &Mat2_col);
    mat2 = (int **) malloc(Mat2_row * sizeof(int *));
    for (int i = 0; i < Mat2_row; i++)
        mat2[i] = (int *) malloc(Mat2_col * sizeof(int *));

    readMat(fp2, mat2, Mat2_row, Mat2_col);

    fclose(fp2);

}

void readMat(FILE *fp, int **arr, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            int number;
            fscanf(fp, "%d", &number);
            arr[i][j] = number;
        }
    }
}

void allocMat() {
    alloc_mat1();
    alloc_mat2();
    //allocate memory space for result mat
    matRes = (int **) malloc(Mat1_row * sizeof(int *));
    for (int i = 0; i < Mat1_row; i++)
        matRes[i] = (int *) malloc(Mat2_col * sizeof(int *));
}

void customOrDefault(int argc, char *argv[]) {
    //checking if the user has entered inputs or not
    if (argc >= 3) {
        files[0] = argv[1];
        files[1] = argv[2];
        char *a = malloc(sizeof(char)*100);
        strcpy(a, argv[3]) ;

        files[2] = strcat(strdup(a),per_method[0]);
        files[3] = strcat(strdup(a),per_method[1]);
        files[4] = strcat(strdup(a),per_method[2]);
        free (a);

    } else {
        //default files
        files[0] = "a";
        files[1] = "b";
        files[2] = "c_per_matrix.txt";
        files[3] = "c_per_row.txt";
        files[4] = "c_per_element.txt";
    }
}

int main(int argc, char *argv[]) {
    customOrDefault(argc, argv);
    allocMat();
    doMethod();
}
