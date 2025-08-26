#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STATES 256
#define MAX_ALPHABET 128
#define MAX_NAME 64

static char states[MAX_STATES][MAX_NAME];
static int num_states = 0;
static char alphabet[MAX_ALPHABET];
static int num_alpha = 0;
static char start_state[MAX_NAME] = "";
static char accept_states[MAX_STATES][MAX_NAME];
static int num_accept = 0;
static int trans[MAX_STATES][MAX_ALPHABET]; // índices de estado, -1 si no definido

static int state_index(const char *name) {
    for (int i = 0; i < num_states; ++i) {
        if (strcmp(states[i], name) == 0) return i;
    }
    return -1;
}

static int alpha_index(char sym) {
    for (int i = 0; i < num_alpha; ++i) {
        if (alphabet[i] == sym) return i;
    }
    return -1;
}

static int is_accept(const char *name) {
    for (int i = 0; i < num_accept; ++i) {
        if (strcmp(accept_states[i], name) == 0) return 1;
    }
    return 0;
}

static void trim(char *s) {
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

static void parse_list(char *value, char arr[][MAX_NAME], int *count) {
    char *tok = strtok(value, ",");
    while (tok) {
        trim(tok);
        if (*tok) {
            strncpy(arr[*count], tok, MAX_NAME - 1);
            arr[*count][MAX_NAME - 1] = '\0';
            (*count)++;
        }
        tok = strtok(NULL, ",");
    }
}

static void parse_alphabet(char *value) {
    char *tok = strtok(value, ",");
    while (tok) {
        trim(tok);
        if (strlen(tok) != 1) {
            fprintf(stderr, "Símbolo no es de 1 carácter: %s\n", tok);
            exit(1);
        }
        alphabet[num_alpha++] = tok[0];
        tok = strtok(NULL, ",");
    }
}

static void init_transitions() {
    for (int i = 0; i < MAX_STATES; ++i)
        for (int j = 0; j < MAX_ALPHABET; ++j)
            trans[i][j] = -1;
}

static void load_config(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("Conf.txt"); exit(1);}    
    init_transitions();
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (!*line || line[0] == '#') continue;
        char *colon = strchr(line, ':');
        if (!colon) { fprintf(stderr, "Línea inválida: %s\n", line); exit(1);}        
        *colon = '\0';
        char *key = line; trim(key);
        char *value = colon + 1; trim(value);
        if (strcmp(key, "states") == 0) {
            parse_list(value, states, &num_states);
        } else if (strcmp(key, "alphabet") == 0) {
            parse_alphabet(value);
        } else if (strcmp(key, "start") == 0) {
            strncpy(start_state, value, MAX_NAME - 1);
            start_state[MAX_NAME - 1] = '\0';
        } else if (strcmp(key, "accept") == 0) {
            parse_list(value, accept_states, &num_accept);
        } else if (strcmp(key, "transition") == 0) {
            char left[256], dest[256];
            char *arrow = strstr(value, "->");
            if (!arrow) { fprintf(stderr, "Transición inválida: %s\n", value); exit(1);}            
            *arrow = '\0';
            strncpy(dest, arrow + 2, sizeof(dest) - 1); dest[sizeof(dest) - 1] = '\0';
            strncpy(left, value, sizeof(left) - 1); left[sizeof(left) - 1] = '\0';
            trim(dest); trim(left);
            char origin[256]; char symstr[256];
            char *comma = strchr(left, ',');
            if (!comma) { fprintf(stderr, "Transición inválida: %s\n", value); exit(1);}            
            *comma = '\0';
            strncpy(origin, left, sizeof(origin) - 1); origin[sizeof(origin) - 1] = '\0'; trim(origin);
            strncpy(symstr, comma + 1, sizeof(symstr) - 1); symstr[sizeof(symstr) - 1] = '\0'; trim(symstr);
            if (strlen(symstr) != 1) { fprintf(stderr, "Símbolo no es de 1 carácter: %s\n", symstr); exit(1);}            
            int si = state_index(origin);
            if (si == -1) { fprintf(stderr, "Estado origen desconocido: %s\n", origin); exit(1);}            
            int ai = alpha_index(symstr[0]);
            if (ai == -1) { fprintf(stderr, "Símbolo fuera del alfabeto: %s\n", symstr); exit(1);}            
            int di = state_index(dest);
            if (di == -1) { fprintf(stderr, "Estado destino desconocido: %s\n", dest); exit(1);}            
            trans[si][ai] = di;
        } else {
            fprintf(stderr, "Clave desconocida: %s\n", key); exit(1);
        }
    }
    fclose(f);
    if (num_states == 0 || num_alpha == 0 || !*start_state || num_accept == 0) {
        fprintf(stderr, "Configuración incompleta (states, alphabet, start, accept)\n");
        exit(1);
    }
    if (state_index(start_state) == -1) {
        fprintf(stderr, "Estado inicial no está en states\n");
        exit(1);
    }
}

static int run_dfa(const char *input) {
    int current = state_index(start_state);
    for (const char *p = input; *p; ++p) {
        int ai = alpha_index(*p);
        if (ai == -1) return 0;
        int nxt = trans[current][ai];
        if (nxt == -1) return 0;
        current = nxt;
    }
    return is_accept(states[current]);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s Conf.txt Cadenas.txt\n", argv[0]);
        return 1;
    }
    load_config(argv[1]);
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror("Cadenas.txt"); return 1; }
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (!*line || line[0] == '#') continue;
        int ok = run_dfa(line);
        printf("%s: %s\\n", line, ok ? "ACCEPTED" : "REJECTED");
    }
    fclose(f);
    return 0;