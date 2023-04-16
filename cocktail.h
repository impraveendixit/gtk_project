#ifndef COCKTAIL_H_INCLUDED
#define COCKTAIL_H_INCLUDED

struct cocktail;

struct cocktail {
	char *id;
	char *name;
	char *alcoholic;
	char *category;
	char *ingredient1;
	char *ingredient2;
	char *ingredient3;
	unsigned int favorite : 1;
	unsigned int has_alcohol : 1;
	struct cocktail *next;
};

struct drinks_info {
	struct cocktail *head;
};

struct mem_chunk {
	char *memory;
	size_t size;
};

void update_favorite_list(struct drinks_info *di, const char *id,
			  unsigned int add_favorite);

int extract_drinks_info(const char *str, struct drinks_info *di);

void print_drinks_info(struct drinks_info *di);

void destroy_drinks_info(struct drinks_info *di);

//void get_api_curl(const char *url, const char *filename);

int get_api(const char *url, struct mem_chunk *mem);

#endif /* COCKTAIL_H_INCLUDED */
