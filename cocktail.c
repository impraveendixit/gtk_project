#include <json-c/json.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cocktail.h"


static struct cocktail *cocktail_object_create(void)
{
	struct cocktail *ct = calloc(1, sizeof(struct cocktail));
	if (ct == NULL) {
		printf("Memory allocation failure\n");
		return NULL;
	}
	return ct;
}

static void cocktail_object_destroy(struct cocktail *ct)
{
	if (ct) {
		if (ct->id)
			free(ct->id);
		if (ct->name)
			free(ct->name);
		if (ct->alcoholic)
			free(ct->alcoholic);
		if (ct->category)
			free(ct->category);
		if (ct->ingredient1)
			free(ct->ingredient1);
		if (ct->ingredient2)
			free(ct->ingredient2);
		if (ct->ingredient3)
			free(ct->ingredient3);
		free(ct);
	}
}

static void add_cocktail(struct cocktail **head_ptr, struct cocktail *new)
{
	struct cocktail **pp = head_ptr;

	while (*pp && *pp != NULL)
		pp = &((*pp)->next);

	new->next = *pp;
	*pp = new;
}

static void destroy_cocktails(struct cocktail *head)
{
	struct cocktail *p = head;

	while (p) {
		struct cocktail *tmp = p;
		p = tmp->next;
		cocktail_object_destroy(tmp);
	}
}

static void extract_cocktail_field(json_object *obj, struct cocktail *ct)
{
	const char *strp = NULL;
	json_object *tmp = NULL;

	if (json_object_object_get_ex(obj, "idDrink", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->id = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strDrink", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->name = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strAlcoholic", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->alcoholic = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strCategory", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->category = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strIngredient1", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->ingredient1 = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strIngredient2", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->ingredient2 = strdup(strp);
	}
	if (json_object_object_get_ex(obj, "strIngredient3", &tmp)) {
		strp = json_object_get_string(tmp);
		if (strp)
			ct->ingredient3 = strdup(strp);
	}
}

static void __check_for_alcohol(struct cocktail *ct)
{
	if (!strcmp(ct->alcoholic, "Alcoholic"))
		ct->has_alcohol = 1;
}

static int extract_cocktails(json_object *drinks, struct drinks_info *di)
{
	int len = json_object_array_length(drinks);

	for (register int i = 0; i < len; i++) {
		json_object *obj = json_object_array_get_idx(drinks, i);
		struct cocktail *ct = cocktail_object_create();
		if (ct == NULL)
			continue;
		extract_cocktail_field(obj, ct);
		__check_for_alcohol(ct);
		add_cocktail(&di->head, ct);
	}
}

void update_favorite_list(struct drinks_info *di, const char *id,
			  unsigned int add_favorite)
{
	struct cocktail *ct = di->head;

	while (ct) {
		if (!strcmp(id, ct->id)) {
			if (add_favorite)
				ct->favorite = 1;
			else
				ct->favorite = 0;
			break;
		}
		ct = ct->next;
	}
}

static void extract_favorite_cocktails(json_object *drinks,
				       struct drinks_info *di)
{
	int len = json_object_array_length(drinks);

	for (register int i = 0; i < len; i++) {
		json_object *obj = json_object_array_get_idx(drinks, i);
		const char *id = json_object_get_string(obj);

		struct cocktail *p = di->head;
		while (p) {
			if (!strcmp(p->id, id)) {
				p->favorite = 1;
				break;
			}
			p = p->next;
		}
	}
}

static int extract_favorite_drinks_info(const char *json_file,
					struct drinks_info *di)
{
	json_object *root = json_object_from_file(json_file);
	json_object *tmp = NULL;
	if (!root) {
		printf("json parsing failed. %s\n", json_file);
		return -1;
	}

	if (json_object_object_get_ex(root, "idDrinks", &tmp)) {
		extract_favorite_cocktails(tmp, di);
	}
	return 0;
}

int extract_drinks_info(const char *str, struct drinks_info *di)
{
//	json_object *root = json_object_from_file(json_file);
	json_object *root = json_tokener_parse(str);
	json_object *tmp = NULL;
	if (!root) {
		printf("json parsing failed.\n");
		return -1;
	}

	/* Reset head pointer */
	di->head = NULL;
	if (json_object_object_get_ex(root, "drinks", &tmp)) {
		extract_cocktails(tmp, di);
		extract_favorite_drinks_info("favorite.json", di);
	}

	json_object_put(root);
	return 0;
}

void write_favorite_drinks_info(const char *json_file,
				struct drinks_info *di)
{
	json_object *root = json_object_from_file(json_file);
	json_object *tmp;
	struct cocktail *p = di->head;

	if (!root) {
		printf("json parsing failed.\n");
		return;
	}
	if (json_object_object_get_ex(root, "idDrinks", &tmp)) {
		/* Delete all elements of array */
		json_object_array_del_idx(tmp, 0,
					  json_object_array_length(tmp));

		while (p) {
			if (p->favorite)
				json_object_array_add(tmp,
					    json_object_new_string(p->id));
			p = p->next;
		}

		json_object_to_file_ext(json_file, root,
					(JSON_C_TO_STRING_PRETTY |
					 JSON_C_TO_STRING_SPACED |
					 JSON_C_TO_STRING_NOSLASHESCAPE));
	}
	json_object_put(root);
}

static void print_favorite_drinks_info(struct drinks_info *di)
{
	struct cocktail *p = di->head;

	while (p) {
		if (p->favorite) {
			printf("ID: %s\n", p->id);
			printf("Name: %s\n", p->name);
			printf("Alcoholic: %s\n", p->alcoholic);
			printf("Category: %s\n", p->category);
			printf("ingredient1: %s\n", p->ingredient1);
			printf("ingredient2: %s\n", p->ingredient2);
			printf("ingredient3: %s\n", p->ingredient3);
		}
		p = p->next;
	}
}

void print_drinks_info(struct drinks_info *di)
{
	struct cocktail *p = di->head;

	while (p) {
		printf("ID: %s\n", p->id);
		printf("Name: %s\n", p->name);
		printf("Alcoholic: %s\n", p->alcoholic);
		printf("Category: %s\n", p->category);
		printf("ingredient1: %s\n", p->ingredient1);
		printf("ingredient2: %s\n", p->ingredient2);
		printf("ingredient3: %s\n", p->ingredient3);
		p = p->next;
	}
	printf("\n--------Favorite Drinks:------\n");
	print_favorite_drinks_info(di);
}

void destroy_drinks_info(struct drinks_info *di)
{
	write_favorite_drinks_info("favorite.json", di);
	destroy_cocktails(di->head);
}

/*void get_api_curl(const char *url, const char *filename)
{
	char cmd[256] = "";

	snprintf(cmd, 256, "curl %s -H Accept: application/json >%s",
		 url, filename);
	system(cmd);
}
*/

static size_t write_callback(void *contents, size_t size,
			     size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct mem_chunk *mem = (struct mem_chunk *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(!ptr) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int get_api(const char *url, struct mem_chunk *mem)
{
	CURL *curl_handle;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)mem);

	/* some servers do not like requests that are made without a user-agent
	   field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* get it! */
	res = curl_easy_perform(curl_handle);

	/* check for errors */
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		return -1;
	} else {
		printf("%lu bytes retrieved\n", (unsigned long)mem->size);
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	/* we are done with libcurl, so clean it up */
	curl_global_cleanup();

	return 0;
}
