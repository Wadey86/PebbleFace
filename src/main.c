#include <pebble.h>
	
Window *window;
TextLayer *text_layer, *bt_layer;
char buffer[] = "00:00";
GBitmap *face_bitmap;
BitmapLayer *face_layer;

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	text_layer_set_text(text_layer, buffer);
}

void bt_handler(){
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(bt_layer, "");
	}
	else{
		text_layer_set_text(bt_layer, "No Bluetooth Connection");
	}
}

void window_load(Window *window){
	//load bitmap
	face_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WATCHFACE);
	//add bitmap layer
	face_layer = bitmap_layer_create(GRect(0,0,144,168));
	bitmap_layer_set_bitmap(face_layer, face_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(face_layer));
	
	text_layer = text_layer_create(GRect(0, 0, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	//time struct to display time on load
	struct tm *t;
	time_t temp;
	temp = time(NULL);
	t = localtime(&temp);
	//manually invoke tick handler to refresh time on load
	tick_handler(t, MINUTE_UNIT);
	
	bt_layer = text_layer_create(GRect(0, 125, 144, 43));
	text_layer_set_background_color(bt_layer, GColorClear);
	text_layer_set_text_color(bt_layer, GColorWhite);
	text_layer_set_text_alignment(bt_layer, GTextAlignmentCenter);
	text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), (Layer*) bt_layer);
}
 
void window_unload(Window *window){
    text_layer_destroy(text_layer);
	text_layer_destroy(bt_layer);
	gbitmap_destroy(face_bitmap);
	bitmap_layer_destroy(face_layer);
}

void handle_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
    	.unload = window_unload,
    });
	window_stack_push(window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	bluetooth_connection_service_subscribe((BluetoothConnectionHandler) bt_handler);
}

void handle_deinit(void){
	window_destroy(window);
	tick_timer_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
}

int main(void){
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}