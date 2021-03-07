/**
 * PACMAN ANALYZER
 * 
 * This program analyzes a PACMAN logfile from Linux and writes 
 * a file with useful statistics per package and in general.
 * 
 * Hugo Isaac Valdez Ruvalcaba - A01631301
 * 
 * Advanced Programming FJ2021
 * Professor Obed N Munoz
 * March 7th, 2021
 * 
 * References:
 *  1.- HashTable C implementation: https://medium.com/@bennettbuchanan/an-introduction-to-hash-tables-in-c-b83cbf2b4cf6
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct List
{
    char *key;
    char *name;
    char *install_date;
    char *last_update_date;
    char *update_num;
    char *removal_date;
    struct List *next;
} List;

typedef struct HashTable
{
    unsigned int size;
    List **array;
} HashTable;

HashTable *ht;
int first_installation,
    installed_pkg_count,
    upgraded_pkg_count,
    removed_pkg_count,
    curr_installed_pkg_count,
    alpm_scripttlet_log_count,
    alpm_log_count,
    pacman_log_count;
char *oldest_pkg, *newest_pkg, *pkg_no_upgrades;

/* Main methods */
int analyze_log(char *logFile);
void process_line(char **split);
void get_pkg_no_upgrades(HashTable *ht);
void write_report(char *reportFile, HashTable *ht);

/* Helper methods */
unsigned int hash(const char *key, unsigned int size);
HashTable *ht_create(unsigned int size);
List *ht_get(HashTable *hashtable, const char *key);
int ht_put(HashTable *hashtable, const char *key, const char *prop, const char *value);
void node_handler(HashTable *hashtable, List *node, const char *prop);

/* MAIN PROGRAM */

int main(int argc, char **argv)
{
    // Validate flags and arguments
    if (argc != 5)
    {
        printf("[ERROR] Usage: ./pacman-analizer -input <file.txt> -report <file.txt> \n");
        return 1;
    }

    // Associate file with flag
    int input, output;
    if (!strcmp(argv[1], "-input") && !strcmp(argv[3], "-report"))
    {
        input = 2;
        output = 4;
    }
    else if (!strcmp(argv[1], "-report") && !strcmp(argv[3], "-input"))
    {
        input = 4;
        output = 2;
    }
    else
    {
        printf("[ERROR] Usage: ./pacman-analizer -input <file.txt> -report <file.txt> \n");
        return 1;
    }

    // Create hash table
    ht = ht_create(10);
    if (ht == NULL)
    {
        return 1;
    }

    // Process logs
    int analyzedFailed = analyze_log(argv[input]);

    if (!analyzedFailed)
    {
        // Get packages with no upgrades
        get_pkg_no_upgrades(ht);

        // Write report
        write_report(argv[output], ht);
    }

    return 0;
}

// Open log file and read lines
int analyze_log(char *logFile)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(logFile, "r");
    if (fp == NULL)
    {
        printf("[ERROR] Log file [%s] does not exist.\n", logFile);
        return 1;
    }
    else
    {
        printf("[INFO] Processing logs from [%s] input file\n", logFile);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        int i;
        char *token;
        char *split[1000];

        // Split line
        token = strtok(line, " ");
        for (i = 0; token != NULL; i++)
        {
            split[i] = token;
            token = strtok(NULL, " ");
        }

        // Process line if log is relevant to stats
        if (split[0] != NULL && split[1] != NULL && split[2] != NULL && split[3] != NULL && split[4] != NULL)
        {
            if (!(strcmp(split[2], "[PACMAN]") && strcmp(split[2], "[ALPM]") && strcmp(split[2], "[ALPM-SCRIPTLET]")))
            {
                process_line(split);
            }
            else if (!(strcmp(split[1], "[PACMAN]") && strcmp(split[1], "[ALPM]") && strcmp(split[1], "[ALPM-SCRIPTLET]")))
            {
                process_line(split);
            }
        }
    }

    fclose(fp);
    if (line)
    {
        free(line);
    }
    return 0;
}

// Method to extract relevant info from relevant lines
void process_line(char **split)
{
    char *type, *action, *pkg, *timestamp;

    // Date may come in two different formats, so validate both
    if (!(strcmp(split[2], "[PACMAN]") && strcmp(split[2], "[ALPM]") && strcmp(split[2], "[ALPM-SCRIPTLET]")))
    {
        type = split[2];
        action = split[3];
        pkg = split[4];

        char *date = ++split[0];
        split[1][strlen(split[1]) - 1] = 0;
        char *hour = split[1];
        timestamp = (char *)malloc(2 + strlen(date) + strlen(hour));
        strcpy(timestamp, date);
        strcat(timestamp, " ");
        strcat(timestamp, hour);
    }
    else
    {
        type = split[1];
        action = split[2];
        pkg = split[3];

        split[0]++;
        split[0][strlen(split[0]) - 1] = 0;
        timestamp = split[0];
    }

    // Count log type
    alpm_log_count += !strcmp(type, "[ALPM]");
    pacman_log_count += !strcmp(type, "[PACMAN]");
    alpm_scripttlet_log_count += !strcmp(type, "[ALPM-SCRIPTLET]");

    // Perform logic per action type
    if (!strcmp(action, "installed") || !strcmp(action, "reinstalled"))
    {
        // Update count
        installed_pkg_count++;
        curr_installed_pkg_count++;

        // Save reference to oldest package installed
        if (!first_installation)
        {
            oldest_pkg = strdup(pkg);
            first_installation = 1;
        }

        // Update info in hash table
        ht_put(ht, pkg, "name", pkg);
        ht_put(ht, pkg, "install_date", timestamp);
        ht_put(ht, pkg, "last_update_date", "-");
        ht_put(ht, pkg, "update_num", "0");
        ht_put(ht, pkg, "removal_date", "-");

        // Update variables
        newest_pkg = strdup(pkg);
    }
    else if (!strcmp(action, "upgraded"))
    {
        // Update count
        upgraded_pkg_count++;

        // Update info in hash table
        int update_num;
        if (ht_get(ht, pkg)->update_num != NULL)
        {
            update_num = atoi(ht_get(ht, pkg)->update_num);
            update_num++;
        }
        else
        {
            update_num = 1;
        }
        char *str1 = malloc(sizeof(char) * 10);
        sprintf(str1, "%d", update_num);

        ht_put(ht, pkg, "update_num", str1);
        ht_put(ht, pkg, "last_update_date", timestamp);

        // Update variables
        newest_pkg = strdup(pkg);
    }
    else if (!strcmp(action, "removed"))
    {
        // Update count
        removed_pkg_count++;
        curr_installed_pkg_count--;

        // Update info in hash table
        ht_put(ht, pkg, "removal_date", timestamp);
    }
}

// Method to iterate through hashtable to get all packages that have 0 upgrades
void get_pkg_no_upgrades(HashTable *ht)
{
    List *listptr;
    for (int i = 0; i < ht->size; ++i)
    {
        listptr = ht->array[i];
        while (listptr != NULL)
        {
            if (!strcmp(listptr->update_num, "0"))
            {
                if (pkg_no_upgrades != NULL)
                {
                    char *temp = strdup(pkg_no_upgrades);
                    pkg_no_upgrades = (char *)malloc(3 + strlen(temp) + strlen(listptr->name));
                    strcpy(pkg_no_upgrades, temp);
                    strcat(pkg_no_upgrades, ", ");
                    strcat(pkg_no_upgrades, listptr->name);
                }
                else
                {
                    pkg_no_upgrades = strdup(listptr->name);
                }
            }
            listptr = listptr->next;
        }
    }
}

// Method that outputs stats into a report file
void write_report(char *reportFile, HashTable *ht)
{
    FILE *fptr;
    List *listptr;

    fptr = fopen(reportFile, "w");
    if (fptr == NULL)
    {
        printf("Error!");
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Writing report into [%s] output file\n", reportFile);

    // Write general info
    fprintf(fptr, "Pacman Packages Report\n----------------------\n");
    fprintf(fptr, "- Installed packages : %d\n", installed_pkg_count);
    fprintf(fptr, "- Removed packages   : %d\n", removed_pkg_count);
    fprintf(fptr, "- Upgraded packages  : %d\n", upgraded_pkg_count);
    fprintf(fptr, "- Current installed  : %d\n", curr_installed_pkg_count);
    fprintf(fptr, "-------------\nGeneral Stats\n-------------\n");
    fprintf(fptr, "- Oldest package               : %s\n", oldest_pkg);
    fprintf(fptr, "- Newest package               : %s\n", newest_pkg);
    fprintf(fptr, "- Package with no upgrades     : %s\n", pkg_no_upgrades);
    fprintf(fptr, "- [ALPM-SCRIPTTLET] log count  : %d\n", alpm_scripttlet_log_count);
    fprintf(fptr, "- [ALPM] log count             : %d\n", alpm_log_count);
    fprintf(fptr, "- [PACMAN] log count           : %d\n", pacman_log_count);
    fprintf(fptr, "----------------\nList of packages\n----------------\n");

    // Write package info
    for (int i = 0; i < ht->size; ++i)
    {
        listptr = ht->array[i];
        while (listptr != NULL)
        {
            fprintf(fptr, "- Package Name           : %s\n", listptr->name);
            fprintf(fptr, "\t- Install date       : %s\n", listptr->install_date);
            fprintf(fptr, "\t- Last upgrade date  : %s\n", listptr->last_update_date);
            fprintf(fptr, "\t- Number of upgrades : %s\n", listptr->update_num);
            fprintf(fptr, "\t- Removal date       : %s\n\n", listptr->removal_date);
            listptr = listptr->next;
        }
    }
    fclose(fptr);
}

/* HELPER METHODS */

// Hash function
unsigned int hash(const char *key, unsigned int size)
{
    unsigned int hash;
    unsigned int i;

    hash = 0;
    i = 0;
    while (key && key[i])
    {
        hash = (hash + key[i]) % size;
        ++i;
    }
    return (hash);
}

// Hashtable initializer
HashTable *ht_create(unsigned int size)
{
    HashTable *ht;

    if (size < 1)
    {
        return NULL;
    }

    ht = malloc(sizeof(HashTable));
    if (ht == NULL)
    {
        return (NULL);
    }

    ht->array = (List **)malloc(size * sizeof(List));
    if (ht->array == NULL)
    {
        return (NULL);
    }

    memset(ht->array, 0, size * sizeof(List));

    ht->size = size;

    return ht;
}

// Hashtable get
List *ht_get(HashTable *hashtable, const char *key)
{
    char *key_cp;
    unsigned int i;
    List *tmp;

    if (hashtable == NULL)
    {
        return NULL;
    }
    key_cp = strdup(key);
    i = hash(key, hashtable->size);
    tmp = hashtable->array[i];

    while (tmp != NULL)
    {
        if (strcmp(tmp->key, key_cp) == 0)
        {
            break;
        }
        tmp = tmp->next;
    }
    free(key_cp);

    if (tmp == NULL)
    {
        return NULL;
    }

    return tmp;
}

// Hashtable put (collision avoidance trhough linked-lists)
int ht_put(HashTable *hashtable, const char *key, const char *prop, const char *value)
{
    List *node;

    if (hashtable == NULL)
    {
        return 1;
    }

    node = malloc(sizeof(List));
    if (node == NULL)
    {
        return (1);
    }

    node->key = strdup(key);

    if (!strcmp(prop, "name"))
    {
        node->name = strdup(value);
    }
    else if (!strcmp(prop, "install_date"))
    {
        node->install_date = strdup(value);
    }
    else if (!strcmp(prop, "last_update_date"))
    {
        node->last_update_date = strdup(value);
    }
    else if (!strcmp(prop, "update_num"))
    {
        node->update_num = strdup(value);
    }
    else if (!strcmp(prop, "removal_date"))
    {
        node->removal_date = strdup(value);
    }

    node_handler(hashtable, node, prop);

    return 0;
}

// Method to traverse all nodes in linked list
void node_handler(HashTable *hashtable, List *node, const char *prop)
{
    unsigned int i = hash(node->key, hashtable->size);
    List *tmp = hashtable->array[i];

    if (hashtable->array[i] != NULL)
    {
        tmp = hashtable->array[i];
        while (tmp != NULL)
        {
            if (strcmp(tmp->key, node->key) == 0)
            {
                break;
            }
            tmp = tmp->next;
        }
        if (tmp == NULL)
        {
            node->next = hashtable->array[i];
            hashtable->array[i] = node;
        }
        else
        {
            if (!strcmp(prop, "name"))
            {
                tmp->name = strdup(node->name);
            }
            else if (!strcmp(prop, "install_date"))
            {
                tmp->install_date = strdup(node->install_date);
            }
            else if (!strcmp(prop, "last_update_date"))
            {
                tmp->last_update_date = strdup(node->last_update_date);
            }
            else if (!strcmp(prop, "update_num"))
            {
                tmp->update_num = strdup(node->update_num);
            }
            else if (!strcmp(prop, "removal_date"))
            {
                tmp->removal_date = strdup(node->removal_date);
            }
        }
    }
    else
    {
        node->next = NULL;
        hashtable->array[i] = node;
    }
}