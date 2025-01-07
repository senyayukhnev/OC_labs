#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define MAX_POINTS 1000
#define MAX_CLUSTERS 10

// Структура для хранения координат точек
typedef struct {
    double x, y;
} Point;

// Глобальные переменные
Point points[MAX_POINTS];
Point centroids[MAX_CLUSTERS];
int labels[MAX_POINTS];
int num_points = 0, num_clusters = 0;
int max_threads = 2;
pthread_mutex_t lock;

// Функция для вычисления евклидова расстояния
double distance(Point a, Point b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

// Потоковая функция для вычисления меток кластеров
void* assign_labels(void* arg) {
    int thread_id = *(int*)arg;

    for (int i = thread_id; i < num_points; i += max_threads) {
        double min_dist = distance(points[i], centroids[0]);
        int best_clumaster = 0;

        for (int j = 1; j < num_clusters; j++) {
            double dist = distance(points[i], centroids[j]);
            if (dist < min_dist) {
                min_dist = dist;
                best_cluster = j;
            }
        }

        pthread_mutex_lock(&lock);
        labels[i] = best_cluster;
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

// Функция для обновления центроидов
void update_centroids() {
    int counts[MAX_CLUSTERS] = {0};
    Point new_centroids[MAX_CLUSTERS] = {{0, 0}};

    for (int i = 0; i < num_points; i++) {
        int cluster = labels[i];
        new_centroids[cluster].x += points[i].x;
        new_centroids[cluster].y += points[i].y;
        counts[cluster]++;
    }

    for (int j = 0; j < num_clusters; j++) {
        if (counts[j] > 0) {
            centroids[j].x = new_centroids[j].x / counts[j];
            centroids[j].y = new_centroids[j].y / counts[j];
        }
    }
}

// Функция для чтения числа
int read_int() {
    char buffer[16];
    int value;
    read(STDIN_FILENO, buffer, sizeof(buffer));
    value = atoi(buffer);
    return value;
}

// Функция для чтения точки
void read_point(Point* point) {
    char buffer[32];
    read(STDIN_FILENO, buffer, sizeof(buffer));
    sscanf(buffer, "%lf %lf", &point->x, &point->y);
}

// Функция для вывода строки
void write_string(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

// Функция для вывода точки
void write_point(const Point* point) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "(%.2f, %.2f)", point->x, point->y);
    write_string(buffer);
}

// Основная функция кластеризации
void kmeans() {
    pthread_t threads[max_threads];
    int thread_ids[max_threads];

    for (int i = 0; i < max_threads; i++) {
        thread_ids[i] = i;
    }

    for (int iteration = 0; iteration < 100; iteration++) {
        // Распределение меток точек
        for (int i = 0; i < max_threads; i++) {
            pthread_create(&threads[i], NULL, assign_labels, &thread_ids[i]);
        }

        for (int i = 0; i < max_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        // Обновление центроидов
        update_centroids();
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        write_string("Usage: ./program <num_threads> <num_clusters>\n");
        return 1;
    }

    max_threads = atoi(argv[1]);
    num_clusters = atoi(argv[2]);

    if (num_clusters > MAX_CLUSTERS || max_threads < 1) {
        write_string("Error: Invalid number of clusters or threads.\n");
        return 1;
    }

    pthread_mutex_init(&lock, NULL);

    // Ввод точек
    write_string("Enter number of points: ");
    num_points = read_int();

    if (num_points > MAX_POINTS) {
        write_string("Error: Too many points.\n");
        return 1;
    }

    write_string("Enter points (x y):\n");
    for (int i = 0; i < num_points; i++) {
        read_point(&points[i]);
    }

    // Инициализация центроидов
    for (int i = 0; i < num_clusters; i++) {
        centroids[i] = points[i];
    }

    // Выполнение кластеризации
    kmeans();

    // Вывод результатов
    write_string("Cluster assignments:\n");
    for (int i = 0; i < num_points; i++) {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Point ");
        write_string(buffer);
        write_point(&points[i]);
        snprintf(buffer, sizeof(buffer), " -> Cluster %d\n", labels[i]);
        write_string(buffer);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}
