#include <pebble.h>
	
Window *window;
TextLayer *text_layer, *bt_layer, *brand_layer, *batt_layer, *date_layer;
char buffer[] = "00:00", batt_pc[] = "000%", date_buffer[] = "00";
//char charge_status;
GBitmap *face_bitmap;
BitmapLayer *face_layer;

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	strftime(date_buffer, sizeof("00"), "%d", tick_time);
	text_layer_set_text(text_layer, buffer);
	text_layer_set_text(date_layer, date_buffer);
}

void bt_handler(){
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(bt_layer, "");
	}
	else{
		text_layer_set_text(bt_layer, "No Bluetooth Connection");
	}
}

void batt_handler(BatteryChargeState charge_state){
	//charge_status = "%";
	snprintf(batt_pc, sizeof("000"), "%d", charge_state.charge_percent);
	text_layer_set_text(batt_layer, batt_pc);
}

void window_load(Window *window){
	
	//load bitmap for background/watchface
	face_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WATCHFACE);
	//add bitmap layer
	face_layer = bitmap_layer_create(GRect(0,0,144,168));
	bitmap_layer_set_bitmap(face_layer, face_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(face_layer));
	
	//add text layer for digital time
	text_layer = text_layer_create(GRect(0, 0, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//create text layer for date
	date_layer = text_layer_create(GRect(116, 76, 25, 50));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
	
	//time struct to display time on load
	struct tm *t;
	time_t temp;
	temp = time(NULL);
	t = localtime(&temp);
	
	//manually invoke tick handler to refresh time on load
	tick_handler(t, MINUTE_UNIT);
	
	//create text layer for bluetooth status
	bt_layer = text_layer_create(GRect(0, 130, 144, 38));
	text_layer_set_background_color(bt_layer, GColorClear);
	text_layer_set_text_color(bt_layer, GColorWhite);
	text_layer_set_text_alignment(bt_layer, GTextAlignmentCenter);
	text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), (Layer*) bt_layer);
	
	//manually invoke bluetooth handler to refresh status on load
	bt_handler();
	
	//create text layer for 'pebble' brand at 6oclock
	brand_layer = text_layer_create(GRect(0, 145, 144, 23));
	text_layer_set_background_color(brand_layer, GColorClear);
	text_layer_set_text_color(brand_layer, GColorWhite);
	text_layer_set_text_alignment(brand_layer, GTextAlignmentCenter);
	text_layer_set_font(brand_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text(brand_layer, "pebble");
	layer_add_child(window_get_root_layer(window), (Layer*) brand_layer);
	
	//create text layer for pebble battery status
	batt_layer = text_layer_create(GRect(5, 76, 20, 50));
	text_layer_set_background_color(batt_layer, GColorClear);
	text_layer_set_text_color(batt_layer, GColorWhite);
	text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
	text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) batt_layer);
	
	//manually invoke battery status handler to refresh status on load
	batt_handler(battery_state_service_peek());
}
 
void window_unload(Window *window){
    text_layer_destroy(text_layer);
	text_layer_destroy(bt_layer);
	text_layer_destroy(brand_layer);
	text_layer_destroy(batt_layer);
	text_layer_destroy(date_layer);
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
	battery_state_service_subscribe((BatteryStateHandler) batt_handler);
}

void handle_deinit(void){
	window_destroy(window);
	tick_timer_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	battery_state_service_unsubscribe();
}

int main(void){
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}