#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "cocktail.h"


int main(int argc, char **argv)
{
	struct drinks_info drinks;
	int quit = 0;
	struct mem_chunk mem;

	mem.size = 0;
	mem.memory = malloc(1);
	if (mem.memory == NULL) {
		printf("Out of memory");
		return -1;
	}

//	get_api_curl("https://www.thecocktaildb.com/api/json/v1/1/search.php?s=margarita",
//		"testpage1.json");

	if (get_api("https://www.thecocktaildb.com/api/json/v1/1/search.php?s=margarita",
		&mem) != 0) {
		printf("Unable to fetch API\n");
		return -1;
	}

	extract_drinks_info(mem.memory, &drinks);
	free(mem.memory);
//	print_drinks_info(&drinks);

	ui_init(&argc, &argv, &drinks);

	while (!quit)
		ui_run(&quit);

	ui_exit();

	destroy_drinks_info(&drinks);

	return 0;
}
