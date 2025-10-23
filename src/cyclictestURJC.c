#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h> 
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <fcntl.h>

#define TIME_SLEEP 1000
#define SEC_TO_NANOSEC 1000000000LL
#define MILI_TO_NANOSEC 1000LL
#define DURATION 60
#define NUMERO_DE_CILCOS 60000
#define PRIORITY 99

typedef struct{
    int id;
    int cpu;
} thread_info;

typedef struct{
    long long latencia_media;
    long long latencia_max;
    long long *array_de_latencias;
    int length;
} lat_info;


void *thread_job(void *arg);
long long long_abs(long long n);
long long time_diff_ns(struct timespec start, struct timespec end);
int array_to_csv(int file, long long *array, int length, int cpu);
void salida_controlada(pthread_t *hilos, thread_info *hilos_cpu, lat_info *latencias, int latency_target_fd, int file);


int main(int argc, char* argv[]) {
    int i, err,latency_target_fd, file;
    lat_info *latencia_cpu;
    int N = (int) sysconf(_SC_NPROCESSORS_ONLN); // Calculamos el numero de cores que hay
    long long latencia_media_total, latencia_max_total;
    struct timespec start, now;
    pthread_t *hilos = malloc(N * sizeof(pthread_t));
    thread_info *hilos_cpu = malloc(N * sizeof(thread_info));
    lat_info *latencias = malloc(N * sizeof(lat_info));
    char *filename = "datos_latencia.csv";
    
    

    latencia_media_total = 0;
    latencia_max_total = 0;
    
    // Para obtener la mínima latencia 
    static int32_t latency_target_value = 0;
    latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    write(latency_target_fd, &latency_target_value, 4);

        for(i = 0; i != N; i++) {
            hilos_cpu[i].id = i;
            hilos_cpu[i].cpu = i;
            err = pthread_create(&hilos[i], NULL, thread_job, &hilos_cpu[i]);
            if(err != 0) {
                fprintf(stderr, "Error creando el hilo");
                exit(EXIT_FAILURE);
            }
        }

        for(i = 0; i != N; i++) {
            pthread_join(hilos[i], (void**)&latencia_cpu);
            if(latencia_cpu != NULL){
                latencias[i] = *latencia_cpu;
                free(latencia_cpu);
            }
            else{
                free(latencias);
                free(hilos);
                free(hilos_cpu);
                free(latencia_cpu);
                close(latency_target_fd);
                exit(EXIT_FAILURE);
            }    
        }
        file = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
        char *header = "CPU, NUMERO_ITERACION, LATENCIA\n";
        if (write(file, header, strlen(header)) < 0) {
                fprintf(stderr,"Error al escribir en el archivo");
                salida_controlada(hilos,hilos_cpu, latencias, latency_target_fd, file);
                exit(EXIT_FAILURE);
            }
        for(i = 0; i != N; i++) {
            latencia_media_total = latencias[i].latencia_media;
                if(latencias[i].latencia_media > latencia_max_total) {
                    latencia_max_total = latencias[i].latencia_media;
                }
                err = array_to_csv(file,latencias[i].array_de_latencias, latencias[i].length, i);
                if(err != 0) {
                    salida_controlada(hilos,hilos_cpu, latencias, latency_target_fd, file);
                    exit(EXIT_FAILURE);
                }
                printf("[%d]    latencia media = %lld ns.  |  max = %lld ns\n",i,latencias[i].latencia_media, latencias[i].latencia_max);
        }
        printf("Total latencia media = %lld ns.  |  max = %lld ns\n", latencia_media_total, latencia_max_total);   

    salida_controlada(hilos,hilos_cpu, latencias, latency_target_fd, file);
    exit(EXIT_SUCCESS);
}   

void *thread_job(void *arg) {
    struct timespec start_lat, end_lat, start, now;
    long long latencia, latencia_media, latencia_max;
    lat_info *latencia_cpu = malloc(sizeof(lat_info));
    long long *array_de_latencias = malloc(sizeof(long long) * NUMERO_DE_CILCOS);
    cpu_set_t set;
    int n, err;

    // Asignamos la cpu correspondiente
    thread_info *info = (thread_info *)arg;
    CPU_ZERO(&set);
    CPU_SET(info->cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);

    // Ponemos la prioridad a 99
    struct sched_param priority_thread;
    priority_thread.sched_priority = PRIORITY;
    err = pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_thread);
    if (err != 0){
        fprintf(stderr, "Error cambiando la prioridad del thread\n");
        return(NULL);
    }
    n = 0;
    latencia_media = 0;
    latencia_max = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &now);

    while(time_diff_ns(start,now) < 60 * SEC_TO_NANOSEC){
    
    clock_gettime(CLOCK_MONOTONIC, &start_lat);
    usleep(TIME_SLEEP); // usleep realiza pausas en microsegundos por eso ponemos 1000 = 1 milisegundo
    clock_gettime(CLOCK_MONOTONIC, &end_lat);

    latencia = time_diff_ns(start_lat, end_lat) - (TIME_SLEEP * MILI_TO_NANOSEC);
    latencia_media += latencia;
    array_de_latencias[n] = latencia;
    if(latencia > latencia_max) {
        latencia_max = latencia;
    }
    n += 1;
    clock_gettime(CLOCK_MONOTONIC, &now);
    }   
    if(n > 0) {
        latencia_cpu->latencia_media = latencia_media / n;
        latencia_cpu->latencia_max = latencia_max;
        latencia_cpu->array_de_latencias = array_de_latencias;
        latencia_cpu->length = n;
    }
    else {
        latencia_cpu = NULL;
    }
    return(latencia_cpu);
}

long long time_diff_ns(struct timespec start, struct timespec end) {
    return((end.tv_sec - start.tv_sec) * SEC_TO_NANOSEC + (end.tv_nsec - start.tv_nsec));
}

int array_to_csv(int file, long long *array, int length,int cpu) {
    int i,len;
    char buffer[128];
    
    if(array != NULL) {

        if (file < 0) {
            fprintf(stderr,"Error al abrir o crear el archivo");
            return(-1);
        }
        // Escribimos la cabecera
        
        for (i = 0; i<length; i++) {
            if (i < length - 1) {
                len = snprintf(buffer, sizeof(buffer), "%d,%d,%lld,\n", cpu,i,array[i]);
            }
            else {
                len = snprintf(buffer, sizeof(buffer), "%d,%d,%lld\n", cpu,i,array[i]);
            }
            if (write(file, buffer, len) < 0) {
                fprintf(stderr,"Error al escribir en el archivo");
                return(-1);
            }
        }
        return(0);
    }
    else {
        return(-1);
    }


}
void salida_controlada(pthread_t *hilos, thread_info *hilos_cpu, lat_info *latencias, int latency_target_fd, int file) {
    free(latencias);
    free(hilos);
    free(hilos_cpu);
    close(latency_target_fd);
    close(file);

}