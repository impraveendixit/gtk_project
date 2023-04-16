#include <gtk/gtk.h>

#include "ui.h"
#include "cocktail.h"

struct ui_struct {
	void *userdata;
	GtkListStore *list_store;
	gchar *combo_text;
	unsigned int fav_btn : 1;
	unsigned int quit : 1;
	unsigned int under_21 : 1;
};

static struct ui_struct ui;

static void __list_set_row_value(GtkListStore *list, GtkTreeIter *iter,
				 struct cocktail *ct, unsigned int fav_btn)
{
	gtk_list_store_append(list, iter);
	gtk_list_store_set(list, iter, 0, ct->id, -1);
	gtk_list_store_set(list, iter, 1, ct->name, -1);
	gtk_list_store_set(list, iter, 2, ct->category, -1);
	gtk_list_store_set(list, iter, 3, ct->ingredient1, -1);
	gtk_list_store_set(list, iter, 4, ct->ingredient2, -1);
	gtk_list_store_set(list, iter, 5, ct->ingredient3, -1);
	if (fav_btn)
		gtk_list_store_set(list, iter, 6, ct->favorite, -1);
}

static void get_list_by_name(struct ui_struct *ui, const char *search_text)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if ((!ui->under_21 | !ct->has_alcohol) &&
		    (strstr(ct->name, search_text)))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);

		ct = ct->next;
	}
}

static void get_list_by_category(struct ui_struct *ui, const char *search_text)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if ((!ui->under_21 | !ct->has_alcohol) &&
		    (strstr(ct->category, search_text)))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);
		ct = ct->next;
	}
}

static void get_list_by_ingredient1(struct ui_struct *ui,
				    const char *search_text)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if ((!ui->under_21 | !ct->has_alcohol) &&
		    (strstr(ct->ingredient1, search_text)))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);
		ct = ct->next;
	}
}

static void get_list_by_ingredient2(struct ui_struct *ui,
				    const char *search_text)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if ((!ui->under_21 | !ct->has_alcohol) &&
		    (strstr(ct->ingredient2, search_text)))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);
		ct = ct->next;
	}
}

static void get_list_by_ingredient3(struct ui_struct *ui,
				    const char *search_text)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if ((!ui->under_21 | !ct->has_alcohol) &&
		    (strstr(ct->ingredient3, search_text)))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);
		ct = ct->next;
	}
}

static void get_list_by_favorites(struct ui_struct *ui)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkTreeIter iter;

	while (ct) {
		if (ct->favorite && (!ui->under_21 | !ct->has_alcohol))
			__list_set_row_value(ui->list_store, &iter,
					     ct, ui->fav_btn);
		ct = ct->next;
	}
}

static void on_combo_box_changed(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	ui->combo_text =
		gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

	if (ui->combo_text && !strcmp(ui->combo_text, "Favorites")) {
		gtk_list_store_clear(ui->list_store);
		get_list_by_favorites(ui);
	}
}

static void on_search_entry_changed(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));

	gtk_list_store_clear(ui->list_store);

	if (!ui->combo_text || !strcmp(ui->combo_text, "Name")) {
		get_list_by_name(ui, text);
	} else if (!strcmp(ui->combo_text, "Category")) {
		get_list_by_category(ui, text);
	} else if (!strcmp(ui->combo_text, "Ingredient1")) {
		get_list_by_ingredient1(ui, text);
	} else if (!strcmp(ui->combo_text, "Ingredient2")) {
		get_list_by_ingredient2(ui, text);
	} else if (!strcmp(ui->combo_text, "Ingredient3")) {
		get_list_by_ingredient3(ui, text);
	}
}

static void on_favorite_toggled(GtkCellRendererToggle *cell,
				gchar *path, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	GtkTreeIter iter;
	gboolean active;
	gchar *id;

	active = gtk_cell_renderer_toggle_get_active(cell);

	/* Fetch row iter */
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ui->list_store),
					    &iter, path);

	/* Fetch first column to compare */
	gtk_tree_model_get(GTK_TREE_MODEL(ui->list_store),
			   &iter, 0, &id, -1);

	if (active) {
		gtk_list_store_set(GTK_LIST_STORE(ui->list_store),
				   &iter, 6, FALSE, -1);
		update_favorite_list((struct drinks_info *)ui->userdata, id, 0);
	} else {
		gtk_list_store_set(GTK_LIST_STORE(ui->list_store),
				   &iter, 6, TRUE, -1);
		update_favorite_list((struct drinks_info *)ui->userdata, id, 1);
	}
}

static void favorite_screen_create(struct ui_struct *ui)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkBuilder *builder;
	GError *error = NULL;
	GObject *win;
	GObject *combo_box;
	GObject *entry;
	GtkTreeIter iter;

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "appui4.glade", &error) == 0) {
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return;
	}

	win = gtk_builder_get_object(builder, "fav_window");

	ui->list_store = GTK_LIST_STORE(gtk_builder_get_object(builder, "fav_list"));
	while (ct) {
		if (ct->favorite && (!ui->under_21 | !ct->has_alcohol))
			__list_set_row_value(ui->list_store,
					     &iter, ct, ui->fav_btn);
		ct = ct->next;
	}

	entry = gtk_builder_get_object(builder, "search_entry");
	g_signal_connect(entry, "activate",
			 G_CALLBACK(on_search_entry_changed), ui);

	combo_box = gtk_builder_get_object(builder, "combo_filter");
	g_signal_connect(combo_box, "changed",
			 G_CALLBACK(on_combo_box_changed), ui);

	gtk_widget_show_all(GTK_WIDGET(win));
	g_object_unref(builder);
}

static void listall_screen_create(struct ui_struct *ui)
{
	struct cocktail *ct = ((struct drinks_info *)(ui->userdata))->head;
	GtkBuilder *builder;
	GError *error = NULL;
	GObject *win;
	GObject *combo_box;
	GObject *entry;
	GtkTreeIter iter;
	GObject *cr_fav;
	GObject *col_fav;

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "appui3.glade", &error) == 0) {
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return;
	}
	win = gtk_builder_get_object(builder, "listall_window");

	entry = gtk_builder_get_object(builder, "search_entry");
	g_signal_connect(entry, "activate",
			 G_CALLBACK(on_search_entry_changed), ui);

	col_fav = gtk_builder_get_object(builder, "col_fav");
	cr_fav = gtk_builder_get_object(builder, "cr_fav");
	/* Add column attribute */
	gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(col_fav),
				 GTK_CELL_RENDERER(cr_fav), "active", 6);
	g_signal_connect(cr_fav, "toggled",
			 G_CALLBACK(on_favorite_toggled), ui);

	/* Combo filter box */
	combo_box = gtk_builder_get_object(builder, "combo_filter");
	g_signal_connect(combo_box, "changed",
			 G_CALLBACK(on_combo_box_changed), ui);


	ui->list_store = GTK_LIST_STORE(gtk_builder_get_object(builder, "cocktail_list"));
	while (ct) {
		if (!ui->under_21 | !ct->has_alcohol)
			__list_set_row_value(ui->list_store, &iter, ct, ui->fav_btn);
		ct = ct->next;
	}

	gtk_widget_show_all(GTK_WIDGET(win));
	g_object_unref(builder);
}

static void on_fav_button_clicked(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	ui->fav_btn = 0;
	favorite_screen_create(ui);
}

static void on_list_button_clicked(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	ui->fav_btn = 1;
	listall_screen_create(ui);
}

static void on_exit_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *win = (GtkWidget *)data;
	gtk_widget_destroy(win);
}

static void option_screen_create(struct ui_struct *ui)
{
	GtkBuilder *builder;
	GError *error = NULL;
	GObject *win;
	GObject *btn;

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "appui2.glade", &error) == 0) {
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return;
	}

	win = gtk_builder_get_object(builder, "option_window");

	btn = gtk_builder_get_object(builder, "exit_button");
	g_signal_connect(btn, "clicked",
			 G_CALLBACK(on_exit_button_clicked), win);

	btn = gtk_builder_get_object(builder, "list_button");
	g_signal_connect(btn, "clicked",
			 G_CALLBACK(on_list_button_clicked), ui);

	btn = gtk_builder_get_object(builder, "fav_button");
	g_signal_connect(btn, "clicked",
			 G_CALLBACK(on_fav_button_clicked), ui);

	gtk_widget_show_all(GTK_WIDGET(win));
	g_object_unref(builder);
}

static void on_app_window_destroy(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	gtk_main_quit();
	ui->quit = 1;
}

static void on_yes_button_clicked(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	ui->under_21 = 0;
	option_screen_create(ui);
}

static void on_no_button_clicked(GtkWidget *widget, gpointer data)
{
	struct ui_struct *ui = (struct ui_struct *)data;
	ui->under_21 = 1;
	option_screen_create(ui);
}

static void main_screen_create(struct ui_struct *ui)
{
	GError *error = NULL;
	GtkBuilder *builder;
	GObject *btn;
	GObject *win;

	/* Construct a GtkBuilder instance and load our UI description */
	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "appui1.glade", &error) == 0) {
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return;
	}

	win = gtk_builder_get_object(builder, "app_window");
	g_signal_connect(win, "destroy", G_CALLBACK(on_app_window_destroy), ui);

	btn = gtk_builder_get_object(builder, "yes_button");
	g_signal_connect(btn, "clicked", G_CALLBACK(on_yes_button_clicked), ui);

	btn = gtk_builder_get_object(builder, "no_button");
	g_signal_connect(btn, "clicked", G_CALLBACK(on_no_button_clicked), ui);

	gtk_widget_show_all(GTK_WIDGET(win));

	g_object_unref(builder);
}

int ui_init(int *argc, char ***argv, void *userdata)
{
	gtk_init(argc, argv);

	main_screen_create(&ui);

	ui.userdata = userdata;
	ui.quit = 0;
	return 0;
}

void ui_run(int *quit)
{
	gtk_main();
	*quit = ui.quit;
}

void ui_exit(void)
{
}
