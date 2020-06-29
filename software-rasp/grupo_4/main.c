#include <gtk/gtk.h>

typedef struct {
    GtkWidget *w_lbl_quantity;
    GtkWidget *w_sbtn_quantity;
} app_widgets;

int main(int argc, char *argv[])
{
    GtkBuilder    *builder; 
    GtkWidget     *window;
    app_widgets   *widgets = g_slice_new(app_widgets);

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
 
    // get pointers to widgets
    widgets->w_lbl_quantity = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_quantity"));
    widgets->w_sbtn_quantity = GTK_WIDGET(gtk_builder_get_object(builder, "sbtn_quantity"));
 
    gtk_builder_connect_signals(builder, widgets);

    g_object_unref(builder);

    gtk_widget_show(window); 
    gtk_main();
    g_slice_free(app_widgets, widgets);

    return 0;
}

void on_btn_update_clicked(GtkButton *button, app_widgets *app_wdgts)
{
    guint32 quantity = 0; // stores integer read from spin button widget
    gchar out_str[100] = {0}; // buffer for string
 
    quantity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_wdgts->w_sbtn_quantity));
    g_snprintf(out_str, sizeof(out_str), "%d", quantity);
    gtk_label_set_text(GTK_LABEL(app_wdgts->w_lbl_quantity), out_str);
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
